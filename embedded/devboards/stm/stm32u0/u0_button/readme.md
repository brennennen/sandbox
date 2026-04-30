# STM32U0 Button
Minimal STM32U0 button LED (GPIOA Pin5, tied to the NUCLEO-U083RC LED4) program.

`arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -g -O0 -T linker_script.ld -nostdlib -o button.elf ./source/main.c ./source/startup.s`
`STM32_Programmer_CLI.exe -c port=swd freq=4000 mode=UR -e all -w button.elf -v -rst`
`pyocd gdbserver --pack ../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx`