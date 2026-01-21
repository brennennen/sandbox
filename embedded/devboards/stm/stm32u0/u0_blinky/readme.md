# STM32U0 Blinky
Minimal STM32U0 blinky LED (GPIOA Pin5, tied to the NUCLEO-U083RC LED4) program.

## Tools
* arm-non-eabi-gcc toolchain: https://developer.arm.com/downloads/-/gnu-rm
* Flashing/Debugging:
  * pyocd: `python -m pip install pyocd`
    * `pyocd pack install stm32u0`
  * OR: STM32_Programmer_CLI: https://www.st.com/en/development-tools/stm32cubeprog.html#st-get-software

## Building
`arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -g -O0 -T linker_script.ld -nostdlib -o blink.elf ./source/main.c ./source/startup.s`

## Flashing
* openocd: (not supported as of 2025-11)
* pyocd:
  * 1st time: manually install needed pack: `https://github.com/Open-CMSIS-Pack/STM32U0xx_DFP/releases/download/v2.1.0/Keil.STM32U0xx_DFP.2.1.0.pack`
    * `mkdir ./../.temp && wget -P ./../.temp/ https://github.com/Open-CMSIS-Pack/STM32U0xx_DFP/releases/download/v2.1.0/Keil.STM32U0xx_DFP.2.1.0.pack`
  * `pyocd flash --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx ./blink.elf`
* STM's Flash Utility:
  * `STM32_Programmer_CLI.exe -c port=swd freq=4000 mode=UR -e all -w blink.elf -v -rst`

## Debugging:
terminal 1:
`pyocd gdbserver --pack ../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx`
or with slower freq:
`pyocd gdbserver --pack ../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx --frequency 500000`
`pyocd gdbserver --pack ../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx --frequency 100000 --connect under-reset`
terminal 2:
`arm-none-eabi-gdb.exe -x stm32u0_init.gdb ./blink.elf`
`target remote localhost:3333`

```sh
arm-none-eabi-gdb.exe .\blink.elf
target remote localhost:3333
# monitor reset halt
# info registers pc
# si
# ...
```
* Visual debugger (vscode)
  * Install plugin: "Cortex-Debug"
  * Start gdb server: `pyocd gdbserver --pack ../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx`
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Attach to Local PyOCD",
            "cwd": "${workspaceFolder}",
            "executable": "./u0_uart_dma.elf",
            "request": "launch",
            "type": "cortex-debug",
            "servertype": "external",
            "gdbTarget": "localhost:3333",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "raw",
            "svdFile": "${workspaceFolder}/embedded/devboards/stm/stm32u0/.temp/Keil.STM32U0xx_DFP.2.1.0/CMSIS/SVD/STM32U083.svd"
        }
    ]
}
```

## Misc Diagnostics/Tools
* pyocd:
  * Interactive mode:
    * `pyocd commander -t stm32u083rctx --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack`
      * `read32 0x40021000`
      * `write32 0x50000014 0x20`
      * `reset`
      * `halt`
      * `continue`
      * `help`
  * Peek:
    * `pyocd cmd -t stm32u083rctx --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -c "read32 0x40021000"`
  * Erase:
    * `pyocd erase -t stm32u083rctx --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack --chip`
* STM's Flash Utility:
  * Peek
    * `STM32_Programmer_CLI.exe -c port=swd mode=HOTPLUG -r32 <address> <bytes to read>`
      * ex: check if GPIOA clock is on: `STM32_Programmer_CLI.exe -c port=swd mode=HOTPLUG -r32 0x40021094 4`
  * Poke
    * `STM32_Programmer_CLI.exe -c port=swd -w32 <address> <value>`
    * ex: turn on the LED when it's off:
      * `STM32_Programmer_CLI.exe -c port=swd -w32 0x50000014 0x00000020`
  * Option Bytes Dump
    * `STM32_Programmer_CLI.exe -c port=swd mode=UR -ob displ`
  * Option Bytes Write
    * `STM32_Programmer_CLI.exe -c port=swd -ob RDP=0xAA`
  * Mass Erase
    * `STM32_Programmer_CLI.exe -c port=swd mode=UR -e all`

## Resources
* Programming Manual: https://www.st.com/resource/en/programming_manual/pm0223-stm32-cortexm0-mcus-programming-manual-stmicroelectronics.pdf
* Baremetal without vendor tooling with similar(ish) chip for context/ideas: https://kleinembedded.com/stm32-without-cubeide-part-1-the-bare-necessities/
