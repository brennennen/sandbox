/**
 * Small example program used to test communicating with the BMP180 from
 * userspace. The goal being to test the board/datasheet comm before writing
 * a kernel module/driver.
 *
 * BMP180 is a pressure and temperature sensor made for mobile devices.
 * Datasheet: https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
 * Read process described on page 15
 *
 * Compile: gcc ./bmp180log.c
 * Packages: i2c-tools libi2c-dev
 * References:
 * * i2c-tools repo: https://github.com/omapconf/omapconf/blob/master/i2c-tools
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/types.h>
#include "i2c-dev.h" // i2c-tools

// Uncomment to use test data instead of reading from the device on the i2c bus.
//#define TEST_BMP180 1
#ifdef TEST_BMP180
typedef enum {
    TEST_READ_TEMP,
    TEST_READ_PRESSURE,
} test_read_mode_t;
static test_read_mode_t test_read_mode = TEST_READ_TEMP;
int test_main();
uint8_t test_read_i2c_byte(uint8_t address);
void test_write_i2c_byte(uint8_t address, uint8_t value);
#define d_printf(format, ...) printf(format, ##__VA_ARGS__)
#else
#define d_printf(format, ...)
#endif

// Calibration data (only needs to be read once, hard coded at factory)
static int16_t ac1 = 0;
static int16_t ac2 = 0;
static int16_t ac3 = 0;
static uint16_t ac4 = 0;
static uint16_t ac5 = 0;
static uint16_t ac6 = 0;
static int16_t b1 = 0;
static int16_t b2 = 0;
static int16_t mb = 0;
static int16_t mc = 0;
static int16_t md = 0;

static int i2c_file = -1;

uint8_t read_i2c_byte(uint8_t address) {
#ifdef TEST_BMP180
    return test_read_i2c_byte(address);
#else
    int32_t result = i2c_smbus_read_byte_data(i2c_file, address);
    return (uint8_t) result;
#endif
}

int32_t write_i2c_byte(uint8_t address, uint8_t value) {
#ifdef TEST_BMP180
    test_write_i2c_byte(address, value);
#else
    return i2c_smbus_write_byte_data(i2c_file, address, value);
#endif
}

void read_calibration_data() {
    ac1 = (int16_t)(read_i2c_byte(0xAA) << 8) + read_i2c_byte(0xAB);
    ac2 = (int16_t)(read_i2c_byte(0xAC) << 8) + read_i2c_byte(0xAD);
    ac3 = (int16_t)(read_i2c_byte(0xAE) << 8) + read_i2c_byte(0xAF);
    ac4 = (uint16_t)(read_i2c_byte(0xB0) << 8) + read_i2c_byte(0xB1);
    ac5 = (uint16_t)(read_i2c_byte(0xB2) << 8) + read_i2c_byte(0xB3);
    ac6 = (uint16_t)(read_i2c_byte(0xB4) << 8) + read_i2c_byte(0xB5);
    b1 = (int16_t)(read_i2c_byte(0xB6) << 8) + read_i2c_byte(0xB7);
    b2 = (int16_t)(read_i2c_byte(0xB8) << 8) + read_i2c_byte(0xB9);
    mb = (int16_t)(read_i2c_byte(0xBA) << 8) + read_i2c_byte(0xBB);
    mc = (int16_t)(read_i2c_byte(0xBC) << 8) + read_i2c_byte(0xBD);
    md = (int16_t)(read_i2c_byte(0xBE) << 8) + read_i2c_byte(0xBF);
    d_printf("ac1: %d, ac2: %d, ac3: %d, ac4: %d, ac5: %d, ac6: %d\n"
            "b1: %d, b2: %d, mb: %d, mc: %d, md: %d\n",
            ac1, ac2, ac3, ac4, ac5, ac6, b1, b2, mb, mc, md);
}

int32_t read_uncompensated_temperature_value() {
    write_i2c_byte(0xF4, 0x2E);
    usleep(5000);
    int32_t ut = (int32_t)(read_i2c_byte(0xF6) << 8) + read_i2c_byte(0xF7);
    d_printf("ut: %d\n", ut);
    return ut;
}

int32_t read_uncompensated_pressure_value(uint8_t oss) {
    uint8_t write_val = 0x34 + (oss << 6);
    write_i2c_byte(0xF4, write_val);
    usleep(5000);
    int32_t up = (int32_t)(read_i2c_byte(0xF6) << 16) + (read_i2c_byte(0xF7) << 8) + read_i2c_byte(0xF8);
    up = up >> (8 - oss);
    d_printf("up: %d\n", up);
    return up;
}

/**
 * Calculates temperature from values read from the BMP180, algorithm
 * taken from BMP180 data sheet. BOSCH claims the meaning behind each variable
 * is "proprietary", and has refused to disclose any more about the algorithm
 * on their forums. Naming matches the datasheet.
 *
 * @param raw_temp Uncompensated temperature as read from the device.
 * @param raw_pressure Uncompensated pressure as read from the device.
 * @return Temperature in celsius.
 */
int32_t calculate_true_temperature(int32_t raw_temp, int32_t up) {
    int32_t x1 = ((raw_temp - (int32_t)ac6) * (int32_t)ac5) >> 15;
    int32_t x2 = ((int32_t)mc << 11) / (x1 + (int32_t)md);
    int32_t b5 = x1 + x2;
    int32_t t = (b5 + 8) >> 4;
    d_printf("x1: %d, x2: %d, b5: %d, t: %d\n",
        x1, x2, b5, t);
    return t;
}

/**
 * Calculates pressure from values read from the BMP180, algorithm taken
 * from BMP180 data sheet. BOSCH claims the meaning behind each variable
 * is "proprietary", and has refused to disclose any more about the algorithm
 * on their forums. Naming matches the datasheet.
 *
 * @param raw_temp Uncompensated temperature as read from the device.
 * @param raw_pressure Uncompensated pressure as read from the device.
 * @param oss Oversampling mode used to read the uncompensated pressure.
 * @return Pressure in "Pa" (pascals).
 */
int32_t calculate_true_pressure(int32_t raw_temp, uint32_t raw_pressure, uint8_t oss) {
    int32_t b5_x1 = ((raw_temp - (int32_t)ac6) * (int32_t)ac5) >> 15;
    int32_t b5_x2 = ((int32_t)mc << 11) / (b5_x1 + (int32_t)md);
    int32_t b5 = b5_x1 + b5_x2;
    int32_t b6 = b5 - 4000;
    int32_t b3_x1 = (b2 * ((b6 * b6) > 12)) > 11;
    int32_t b3_x2 = (ac2 * b6) >> 11;
    int32_t b3_x3 = b3_x1 + b3_x2;
    int32_t b3 = (((ac1 * 4 + b3_x3) << oss) + 2) / 4;

    int32_t b4_x1 = (int32_t)ac3 * b6 >> 13;
    int32_t b4_x2 = ((int32_t)b1 * ((b6 * b6) >> 12)) >> 16;
    int32_t b4_x3 = ((b4_x1 + b4_x2) + 2) >> 2;
    uint32_t b4 = (uint32_t)ac4 * ((uint32_t)(b4_x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)raw_pressure - b3) * (50000 >> oss);
    int32_t p1 = 0;
    if (b7 < 0x80000000) {
        p1 = (int32_t)(b7 << 1) / b4;
    } else {
        p1 = (int32_t)(b7 / b4) << 1;
    }
    int32_t p_x1 = (p1 >> 8) * (p1 >> 8);
    p_x1 = (p_x1 * 3038) >> 16;
    int32_t p_x2 = (-7357 * p1) >> 16;
    int32_t p2 = p1 + ((int32_t)(p_x1 + p_x2 + 3791) >> 4);

    d_printf("b6: %d, b3_x1: %d, b3_x2: %d, b3_x3: %d, b3: %d\n",
        b6, b3_x1, b3_x2, b3_x3, b3);
    d_printf("b4_x1: %d, b4_x2: %d, b4_x3: %d, b4: %d\n",
        b4_x1, b4_x2, b4_x3, b4);
    d_printf("b7: %d, p1: %d, p_x1: %d, p_x2: %d, p2: %d\n",
            b7, p1, p_x1, p_x2, p2);
    return p2;
}

int main_app() {
    printf("start bmp180_log:main_app\n");

    // settings
    const unsigned char bmp180_addr = 0x77; // I2C address of the BMP180
    const char filename[] = "/dev/i2c-1";

    i2c_file = open(filename, O_RDWR);
    if (i2c_file == -1) {
        perror("failed to open i2c file");
    }
    int open_i2c_result = ioctl(i2c_file, I2C_SLAVE, bmp180_addr);
    if (open_i2c_result < 0) {
        perror("failed to set i2c slave");
    }

    read_calibration_data();
    int32_t ut = read_uncompensated_temperature_value();
    uint8_t pressure_oversampling = 0;
    int32_t up = read_uncompensated_pressure_value(pressure_oversampling);

    int32_t temp = calculate_true_temperature(ut, up);
    float temp_c = temp / 10;
    float temp_f = temp_c * (9.0/5) + 32;
    printf("temp: c: %f, f: %f\n", temp_c, temp_f);

    int32_t pressure = calculate_true_pressure(ut, up, pressure_oversampling);
    printf("pressure: %d pa\n", pressure);
    printf("exit bmp180_log:main_app\n");
}

int main() {
#ifdef TEST_BMP180
    test_main();
#else
    main_app();
#endif
}

// MARK: TEST

#ifdef TEST_BMP180
/**
 * Reads data pulled from the datasheet instead of the i2c bus to sanity check
 * the algorithms match.
 */
uint8_t test_read_i2c_byte_datasheet_example(uint8_t address) {
    if (0xF6 <= address && address <= 0xF8) {
        // temperature or pressure data depending on what was written to 0xF4
        if (test_read_mode == TEST_READ_TEMP) {
            switch(address) {
                case 0xF6: return 0x6C; // 27898
                case 0xF7: return 0xFA;
                case 0xF8: return 0x00; // (not used)
            }
        } else { // TEST_READ_PRESSURE
            switch(address) {
                case 0xF6: return 0x5D; // 23843 << 8
                case 0xF7: return 0x23;
                case 0xF8: return 0x00;
            }
        }
    } else {
        // calibration data
        switch(address) {
            case 0xAA: return 0x01; // 408
            case 0xAB: return 0x98;
            case 0xAC: return 0xFF; // -72
            case 0xAD: return 0xB8;
            case 0xAE: return 0xC7; // -14383
            case 0xAF: return 0xD1;
            case 0xB0: return 0x7F; // 32741
            case 0xB1: return 0xE5;
            case 0xB2: return 0x7F; // 32757
            case 0xB3: return 0xF5;
            case 0xB4: return 0x5A; // 23153
            case 0xB5: return 0x71;
            case 0xB6: return 0x18; // 6190
            case 0xB7: return 0x2E;
            case 0xB8: return 0x00; // 4
            case 0xB9: return 0x04;
            case 0xBA: return 0x80; // -32768
            case 0xBB: return 0x00;
            case 0xBC: return 0xDD; // -8711
            case 0xBD: return 0xF9;
            case 0xBE: return 0x0B; // 2868
            case 0xBF: return 0x34;
            default: return 0x00;
        }
    }
}

/**
 * Reads data pulled from calling 'i2cget' on each address with a real device
 * to sanity check the algorithms with real world data.
 */
uint8_t test_read_i2c_byte_manual(uint8_t address) {
    if (0xF6 <= address && address <= 0xF8) {
        // temperature or pressure data depending on what was written to 0xF4
        if (test_read_mode == TEST_READ_TEMP) {
            // i2cset -y 1 0x77 0xF4 0x2E
            switch(address) {
                case 0xF6: return 0x56;
                case 0xF7: return 0x3E;
                case 0xF8: return 0x00; // (not used)
            }
        } else { // TEST_READ_PRESSURE
            // i2cset -y 1 0x77 0xF4 0x34
            switch(address) {
                case 0xF6: return 0xA0;
                case 0xF7: return 0x60;
                case 0xF8: return 0x00;
            }
        }
    }
    else {
        switch(address) {
            case 0xAA: return 0x1C; // i2cget -y 1 0x77 0xAA
            case 0xAB: return 0xD1; // i2cget -y 1 0x77 0xAB
            case 0xAC: return 0xFB; // ...
            case 0xAD: return 0xFF;
            case 0xAE: return 0xC7;
            case 0xAF: return 0x60;
            case 0xB0: return 0x83;
            case 0xB1: return 0x6A;
            case 0xB2: return 0x64;
            case 0xB3: return 0x38;
            case 0xB4: return 0x36;
            case 0xB5: return 0xD1;
            case 0xB6: return 0x19;
            case 0xB7: return 0x73;
            case 0xB8: return 0x00;
            case 0xB9: return 0x22;
            case 0xBA: return 0x80;
            case 0xBB: return 0x00;
            case 0xBC: return 0xD1;
            case 0xBD: return 0xF6;
            case 0xBE: return 0x09;
            case 0xBF: return 0x74;
            default: return 0x00;
        }
    }
}

uint8_t test_read_i2c_byte(uint8_t address) {
    return test_read_i2c_byte_datasheet_example(address);
    //return test_read_i2c_byte_manual(address);
}

void test_write_i2c_byte(uint8_t address, uint8_t value) {
    if (address == 0xF4) {
        if (value == 0x2E) {
            test_read_mode = TEST_READ_TEMP;
        } else if (value >= 0x34) {
            test_read_mode = TEST_READ_PRESSURE;
        }
    }
}

int test_main() {
    printf("start bmp180_log:main_test\n");
    read_calibration_data();
    int32_t ut = read_uncompensated_temperature_value();
    uint8_t pressure_oversampling = 0;
    int32_t up = read_uncompensated_pressure_value(pressure_oversampling);
    int32_t temp = calculate_true_temperature(ut, up);

    float temp_c = temp / 10;
    float temp_f = temp_c * (9.0/5) + 32;
    printf("temp: c: %f, f: %f\n", temp_c, temp_f);

    int32_t pressure = calculate_true_pressure(ut, up, pressure_oversampling);
    printf("pressure: %d pa\n", pressure);
    printf("exit bmp180_log:main_test\n");
}
#endif
