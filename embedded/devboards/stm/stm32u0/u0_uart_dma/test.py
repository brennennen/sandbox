#!/usr/bin/env python3
##
# Sends a test message over uart and waits for a response.
#
import serial
import time
import sys

SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
PREAMBLE1 = 0x55
PREAMBLE2 = 0xAA
MSG_ID_TEST = 0x01

# Polynomial: 0x07 (SMBus/ATM)
def crc8(data):
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = (crc << 1) ^ 0x07
            else:
                crc <<= 1
    return crc & 0xFF

def transmit_message(ser, payload_str):
    payload_bytes = payload_str.encode('utf-8')
    length = len(payload_bytes)
    if length > 64:
        raise ValueError("Payload too long for STM32 buffer!")
    packet = bytearray([PREAMBLE1, PREAMBLE2, MSG_ID_TEST, length])
    packet.extend(payload_bytes)
    crc = crc8(packet[2:])
    packet.append(crc)
    print(f"tx [{len(packet)} bytes]: {packet.hex(' ')}")
    ser.write(bytes(packet))

def receive_message(ser):
    print("Listening for response...")
    while True:
        byte1 = ser.read(1)
        if not byte1:
            continue
        if byte1[0] == PREAMBLE1:
            byte2 = ser.read(1)
            if byte2 and byte2[0] == PREAMBLE2:
                header = ser.read(2)
                if len(header) < 2:
                    continue
                msg_id = header[0]
                length = header[1]
                bytes_to_read = length + 1 # payload + crc
                print(f"bytes_to_read: {bytes_to_read}")
                variable_body = ser.read(bytes_to_read)
                print(f"len(variable_body): {len(variable_body)}")
                if len(variable_body) < bytes_to_read:
                    print("Error: Incomplete packet received")
                    continue
                payload = variable_body[:-1]
                received_crc = variable_body[-1]
                data = bytearray([byte1[0], byte2[0], msg_id, length]) + variable_body
                calculated_crc = crc8(data[2:-1])
                if received_crc == calculated_crc:
                    print(f"crc match! ({received_crc} == {calculated_crc})")
                else:
                    print(f"ERROR! CRC MISMATCH! ({received_crc} != {calculated_crc})")
                print(f"msg crc: {received_crc}, calc crc: {calculated_crc}")
                print(f"rx: [{len(data)} bytes]: {data.hex(' ')}")
                print(f"Payload: {payload.decode('utf-8')}")
                return

def main():
    if len(sys.argv) < 4:
        print("missing argument. usage: test.py COM3 115200 \"message to send\"")
        exit(-1)
    port = sys.argv[1]
    baud_rate = sys.argv[2]
    msg = sys.argv[3]

    try:
        ser = serial.Serial(port, baud_rate, timeout=1.0)
        ser.reset_input_buffer()
        print(f"Connected to {port} at {baud_rate}")
    except serial.SerialException as e:
        print(f"Could not open port {port}: {e}")
        return

    try:
        transmit_message(ser, msg)
        receive_message(ser)
    except KeyboardInterrupt:
        print("\nStopping...")
    except TypeError:
        print("\nTimeout or bad data received.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()
