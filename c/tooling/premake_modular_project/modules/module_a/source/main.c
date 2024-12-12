
#include <stdio.h>

#include "module_a_foobar.h"

int main(int argc, char* argv[]) {
    printf("hello from module a!\n");
    printf("foobar: %d\n", a_foobar());

#ifdef DEBUG
    printf("DEBUG BUILD!\n");
#elif NDEBUG
    printf("RELEASE BUILD!\n");
#endif
}
