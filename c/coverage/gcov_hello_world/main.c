/**
 * gcc ./main.c -O0 --coverage
 * a.exe
 * gcov ./a.main
 * cat ./main.c.gcov
 */
#include "stdio.h"

int main(int argc, char* argv[]) {
    printf("called once!\n");
    int x = 6;
    for (int i = 0; i < 10; i++) {
        printf("called 10 times!\n");
        if (i % 5 == 0) {
            printf("called twice!\n");
        }
        if (i > 15) {
            printf("never called!\n");
        }
    }
}
