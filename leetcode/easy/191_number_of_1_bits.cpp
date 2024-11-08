/*
g++ ./191_number_of_1_bits.cpp && ./a.out

https://leetcode.com/problems/number-of-1-bits

Given a positive integer n, write a function that returns the number of
set bits
in its binary representation (also known as the Hamming weight).

Example 1:
Input: n = 11
Output: 3
Explanation:
The input binary string 1011 has a total of three set bits.

Example 2:
Input: n = 128
Output: 1
Explanation:
The input binary string 10000000 has a total of one set bit.

Example 3:
Input: n = 2147483645
Output: 30
Explanation:
The input binary string 1111111111111111111111111111101 has a total of thirty set bits.
*/

#include <cassert>
#include <cstdio>

int hammingWeight(int n) {
    int count = 0;
    for (int i = 0; i < 32; i++) {
        count += n & 0x01;
        n = n >> 1;
    }
    return count;
}

int main(int argc, char* argv[]) {
    {
        int input = 11;
        int output = hammingWeight(input);
        printf("test: input: %d, output: %d\n", input, output);
        assert(3 == output);
    }
    {
        int input = 128;
        int output = hammingWeight(input);
        printf("test: input: %d, output: %d\n", input, output);
        assert(1 == output);
    }
    {
        int input = 2147483645;
        int output = hammingWeight(input);
        printf("test: input: %d, output: %d\n", input, output);
        assert(30 == output);
    }
}