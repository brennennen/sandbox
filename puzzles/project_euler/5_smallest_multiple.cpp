/*
g++ -std=c++17 ./5_smallest_multiple.cpp && ./a.out

https://projecteuler.net/problem=5
*/

#include <cassert>
#include <cstdio>
#include <cstdint>

/*
First thought, brute force? iterate from 1 to `20!`, checking that the remainder of division
with each member in the set (1 - 20) is 0. `20!` is over a quintillion though, which makes me
a bit hesitant to just jump right into code.

There are many common factors between numbers of 1 to 20 (20 = 2 * 10 = 4 * 5, so we can remove
2, 10, 4, and 5 from the set, just keeping the 20 to capture all 5 numbers). Removing common 
factors might be a good first step to reduce/simplify the problem space.

Original list:
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20

Common factors removed list:
11, 12, 13, 14, 15, 16, 17, 18, 19, 20 (product: 670442572800)

This is maybe a bit more reasonable. Ends up taking less than a second on my machine, so I'm
fine with this solution. I imagine there is a more clever mathy approach to this, but it's
been a long time since college and I've taken any higher level math classes... 
*/

// 10 example common factors removed list: 6, 7, 8, 9, 10 (product: 30240)
int smallest_multiple_10() {
    int smallest_multiple = 30240;
    for (int i = 1; i < 30240; i++) {
        if (i % 6 == 0 &&
            i % 7 == 0 &&
            i % 8 == 0 &&
            i % 9 == 0 &&
            i % 10 == 0) 
        {
            return i;
        }
    }
    return 30240;
}

uint64_t smallest_multiple_20() {
    for (uint64_t i = 1; i < 670442572800; i++) {
        if (i % 11 == 0 &&
            i % 12 == 0 &&
            i % 13 == 0 &&
            i % 14 == 0 &&
            i % 15 == 0 &&
            i % 16 == 0 &&
            i % 17 == 0 &&
            i % 18 == 0 &&
            i % 19 == 0 &&
            i % 20 == 0) 
        {
            return i;
        }
    }
    return 670442572800;
}

int main(int argc, char* argv[]) {
    assert(smallest_multiple(10) == 2520);
    printf("smallest_multiple_20: %lu\n", smallest_multiple_20());
}
