// Simple test program to test the toy debugger.

#include <stdio.h>
#include <unistd.h>

int foobar(int x) {
    int y = 1;
    return x + y;
}

int main(void) {
    printf("[%d] hello\n", getpid());
    int z = foobar(2);
    sleep(1);
    printf("[%d] world!\n", getpid());
}
