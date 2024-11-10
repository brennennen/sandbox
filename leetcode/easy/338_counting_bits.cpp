/*
g++ ./338_counting_bits.cpp && ./a.out

https://leetcode.com/problems/counting-bits/

Given an integer n, return an array ans of length n + 1 such that
for each i (0 <= i <= n), ans[i] is the number of 1's in the binary 
representation of i.

Example 1:

Input: n = 2
Output: [0,1,1]
Explanation:
0 --> 0
1 --> 1
2 --> 10

Example 2:

Input: n = 5
Output: [0,1,1,2,1,2]
Explanation:
0 --> 0
1 --> 1
2 --> 10
3 --> 11
4 --> 100
5 --> 101
*/

#include <cstdio>
#include <vector>
#include <cassert>

using namespace std;

int countIntBits(int n) {
    int count = 0;
    for (int i = 0; i < 32; i++) {
        count += n & 0x01;
        n = n >> 1;
    }
    return count;
}

vector<int> countBits(int n) {
    vector<int> bitCounts(n + 1);
    for (int i = 0; i < n + 1; i++) {
        bitCounts[i] = countIntBits(i);
    }
    return bitCounts;
}

int main(int argc, char* argv[]) {
    {
        int input = 2;
        vector<int> output = countBits(input);
        printf("test: input: %d, output: [%d, %d, %d]\n", input, output[0], output[1], output[2]);
        assert(0 == output[0]);
        assert(1 == output[1]);
        assert(1 == output[2]);
    }
    {
        int input = 5;
        vector<int> output = countBits(input);
        printf("test: input: %d, output: [%d, %d, %d, %d, %d, %d]\n", 
            input, output[0], output[1], output[2], output[3], output[4], output[5]);
        assert(0 == output[0]);
        assert(1 == output[1]);
        assert(1 == output[2]);
        assert(2 == output[3]);
        assert(1 == output[4]);
        assert(2 == output[5]);
    }
}