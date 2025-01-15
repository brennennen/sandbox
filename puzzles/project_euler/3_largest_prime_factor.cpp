/*
g++ -std=c++17 ./3_largest_prime_factor.cpp && ./a.out

https://projecteuler.net/problem=3
*/

#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cmath>

/*
First thoughts:
I don't remember much math around this subject, so it's an interesting one for me.
My first thought would be to just iterate up from 1 to the sqrt(target), moduloing each
number by the target and checking if it's equal to 0. Then checking if that number
is a prime. Complexity: O(sqrt(n)^2) = O(n).

Second thoughts:
Do the above, but start at sqrt(target) and iterate backwards. If any number is 0, then 
we short circuit. Still O(n) worst case, but assuming the number isn't 
prime, the average case should be reduced.

This got me to the answer and ran fast enough, so I don't think I'll go further, but it 
might be fun to re-visit this and jump down the rabbit hole of prime numbers and integer 
factorization algorithms in the future.
*/
bool is_prime(uint64_t number) {
    if (number <= 2) {
        return true;
    }
    if (number % 2 == 0) {
        return false;
    }
    uint64_t number_sqrt = std::sqrt(number);
    for (uint64_t i = 2; i < number_sqrt + 1; i++) {
        if (number % i == 0) {
            return false;
        }
    }
    return true;
}

uint64_t largest_prime_factor(uint64_t target) {
    uint64_t target_sqrt = std::sqrt(target);
    for (uint64_t i = target_sqrt + 1; i > 0; i--) {
        if (target % i == 0 && is_prime(i)) {
            return i;
        }
    }
    return target;
}

int main(int argc, char* arv[]) {
    assert(is_prime(1) == true);
    assert(is_prime(2) == true);
    assert(is_prime(4) == false);
    assert(is_prime(7) == true);
    assert(is_prime(35) == false);
    assert(is_prime(100) == false);
    assert(is_prime(101) == true);

    assert(largest_prime_factor(13195) == 29);
    printf("largest_prime_factor(600851475143): %ld\n", largest_prime_factor(600851475143)); // 600,851,475,143
}
