/*
g++ -std=c++17 ./2_even_fibonacci_numbers.cpp && ./a.out

https://projecteuler.net/problem=2
*/

#include <cassert>
#include <cstdio>
#include <cstdint>

/*
This felt pretty straight forward, evaluate the fibonacci sequence up to a set
value and keep a running sum of fibonacci sequence numbers that are even.

It's been awhile since I've done anything with a fibonacci series, so I'm not
sure if this is the most optimal way to go about it, but it seems simple/fast
enough.
*/
uint64_t even_fibonacci_numbers(uint64_t up_to) {
    uint64_t prev_prev = 1;
    uint64_t prev = 1;
    uint64_t current = 1;
    uint64_t evens = 0;

    while (true) {
        current += prev_prev;

        if (current > up_to) {
            break;
        }
        
        if (current % 2 == 0) {
            evens += current;
        }

        prev_prev = prev;
        prev = current;
    }

    return evens;
}

int main(int argc, char* arv[]) {
    assert(even_fibonacci_numbers(2) == 2);
    assert(even_fibonacci_numbers(8) == 10);
    assert(even_fibonacci_numbers(34) == 44);
    
    printf("even_fibonacci_numbers(4000000): %'lu\n", even_fibonacci_numbers(4000000));
}
