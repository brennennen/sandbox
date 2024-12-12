
#include <stdio.h>

#include "libraries/lib_a/include/lib_a.h"
#include "lib_b.h"
#include "common.h"

int main(int argc, char* argv[]) {
    printf("hello from module b!\n");
    printf("common: %f\n", SPECIAL_SHARED_CONST);
    lib_b_log_hello();
}
