# MSPM0C1104
Low energy mcu with extremely small package options (1.38mm^2, black pepper flake).
24hz CPU, 1K SRAM, 16K flash
Peripherals: SPI, I2C, UART, 18 GPIO, ADC,
$0.16 per unit

# Resources
* Product page - https://www.ti.com/product/MSPM0C1104
* Datasheet - https://www.ti.com/lit/ds/symlink/mspm0c1104.pdf
* https://www.ti.com/lit/ug/slau908b/slau908b.pdf?ts=1742760576521&ref_url=https%253A%252F%252Fwww.ti.com%252Ftool%252FLP-MSPM0C1104

## Lanchpad
The MSPM0C1104 is the chip labeled "U1" on the right side of the board.

## Out of the box
* Came with a blinky program.
* Flashed an updated version through the new vscode/theia CCS (cloud composer studio).
* Installed a "CCS Cloud Agent" service and a firefox plugin to forward serial data to the web browser gui that sends commands to control how frequently the LED blinks.

## IDE
* CCS 20.1.0
  * Code Composer Studio. Vscode based.

## Projects
### I2C Echo
Echo I2C messages to test I2C functionality on both this and other I2C boards.
* In CCS, select board, select "i2c_target_rw_multibyte_fifo_poll" as base project template.
