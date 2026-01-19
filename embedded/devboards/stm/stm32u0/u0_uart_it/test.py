#!/usr/bin/env python3
##
# Test script for u0_art_it. Sends a message to the stm32u0 and waits for
# a response.
# usage: `./test.py "message to echo back here"`
# dependencies: pyserial (python -m pip install pyserial)
#

import serial
import sys

SERIAL_PORT = 'COM4'
BAUD_RATE = 115200
TIMEOUT = 2.0

def main():
    if len(sys.argv) < 2:
        print(f"Usage: python {sys.argv[0]} \"message to echo back here\"")
        sys.exit(1)
    message = sys.argv[1] + "\n"
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT) as ser:
            ser.reset_input_buffer()
            print(f"Sending: {repr(message)}")
            ser.write(message.encode('utf-8'))
            response = ser.readline().decode('utf-8', errors='ignore')
            if response:
                print(f"Received: {repr(response)}")
            else:
                print("Timed out: No response received.")
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)

if __name__ == "__main__":
    main()
