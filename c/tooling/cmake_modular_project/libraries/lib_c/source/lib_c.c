
#include <stdio.h>

#include "lib_c.h"

#include "lib_d.h"
#include "lib_e.h"

void lib_c_log_hello(void)
{
    printf("Hello from lib_c\n");
    lib_d_log_hello();
    lib_e_log_hello();
}