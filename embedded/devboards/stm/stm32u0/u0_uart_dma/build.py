import subprocess

cmd = """arm-none-eabi-gcc
-mcpu=cortex-m0plus -mthumb
-g -O0 -nostdlib
-ffunction-sections -fdata-sections
-Wl,--gc-sections
-DSTM32U083xx
-DDEBUG
-I./../.build/STM32CubeU0/Drivers/CMSIS/Device/ST/STM32U0xx/Include
-I./../.build/STM32CubeU0/Drivers/CMSIS/Core/Include
-o u0_uart_dma.elf
-T linker_script.ld
./source/main.c
./source/startup.s""".split()
subprocess.run(cmd, check=True)
