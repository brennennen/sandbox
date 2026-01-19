import subprocess

cmd = """arm-none-eabi-gcc 
-mcpu=cortex-m0plus -mthumb 
-Os -nostdlib 
-DSTM32U083xx 
-DDEBUG 
-ffunction-sections -fdata-sections 
-Wl,--gc-sections
-flto 
-I./../.build/STM32CubeU0/Drivers/CMSIS/Device/ST/STM32U0xx/Include 
-I./../.build/STM32CubeU0/Drivers/CMSIS/Core/Include 
-o u0_uart_it.elf 
-T linker_script.ld 
./source/main.c 
./source/startup.s""".split()
subprocess.run(cmd, check=True)
