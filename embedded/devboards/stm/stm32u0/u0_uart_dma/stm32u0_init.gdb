# stm32_init.gdb
set mem inaccessible-by-default off
target extended-remote :3333
monitor reset halt
# Enable Vector Catch
set *(int *)0xE000EDFC = *(int *)0xE000EDFC | 0x400
# Enable Debug Clocks
set *(unsigned int *)0x40015804 |= 0x6
