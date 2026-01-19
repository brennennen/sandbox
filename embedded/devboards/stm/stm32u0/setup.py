
import os
import subprocess

def install_stm32_cube_u0_firmware_package():
    """Download the full vendor provided support suite for this chip: hal, bsp, examples, etc."""
    if os.path.exists("./.build/STM32CubeU0"):
        return
    subprocess.run("git clone --depth 1 --recursive https://github.com/STMicroelectronics/STM32CubeU0.git ./.build/STM32CubeU0".split(), check=True)

def install_keil_stm32u0_debug_pack():
    # 
    # TODO: download https://github.com/Open-CMSIS-Pack/STM32U0xx_DFP/releases/download/v2.1.0/Keil.STM32U0xx_DFP.2.1.0.pack
    # mkdir ./../.temp && wget -P ./../.temp/ https://github.com/Open-CMSIS-Pack/STM32U0xx_DFP/releases/download/v2.1.0/Keil.STM32U0xx_DFP.2.1.0.pack
    pass

def main():
    if not os.path.exists("./.build"):
        #TODO: create .build
        pass
    install_stm32u0_device_cmsis()
    install_stm32_cube_u0_firmware_package()
    install_keil_stm32u0_debug_pack()

if __name__ == "__main__":
    main()
