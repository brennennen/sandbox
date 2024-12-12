#include <stdio.h>

#include "lib_a.h"
#include "lib_b.h"

int main()
{
   printf("Hello from module_b\n");
   lib_a_log_hello();
   lib_b_log_hello();
   return 0;
}