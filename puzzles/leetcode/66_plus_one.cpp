/*
g++ ./66_plus_one.cpp && ./a.out

https://leetcode.com/problems/plus-one

66. Plus One

You are given a large integer represented as an integer array digits, where each digits[i] is the 
ith digit of the integer. The digits are ordered from most significant to least significant in 
left-to-right order. The large integer does not contain any leading 0's.
Increment the large integer by one and return the resulting array of digits.

 

Example 1:
Input: digits = [1,2,3]
Output: [1,2,4]
Explanation: The array represents the integer 123.
Incrementing by one gives 123 + 1 = 124.
Thus, the result should be [1,2,4].

Example 2:
Input: digits = [4,3,2,1]
Output: [4,3,2,2]
Explanation: The array represents the integer 4321.
Incrementing by one gives 4321 + 1 = 4322.
Thus, the result should be [4,3,2,2].

Example 3:
Input: digits = [9]
Output: [1,0]
Explanation: The array represents the integer 9.
Incrementing by one gives 9 + 1 = 10.
Thus, the result should be [1,0].

Constraints:
    1 <= digits.length <= 100
    0 <= digits[i] <= 9
    digits does not contain any leading 0's.

 */

#include <vector>

#include <cassert>
#include <cstdio>

using namespace std;

vector<int> plusOne(vector<int>& digits) {
    int carry = 1;
    for (int i = digits.size() - 1; i >= 0; i--) {
        if (carry == 0) {
            break;
        }
        carry = 0;
        if (digits[i] == 9) {
            carry = 1;
            digits[i] = 0;
            if (i == 0) {
                digits.insert(digits.begin(), 1);
            }
        } else {
            digits[i] += 1;
        }
    }
    return digits;
}

int main(int argc, char* argv[]) {
    {
        vector<int> input = {1, 2, 3};
        printf("test: input: [%d, %d, %d]\n", input[0], input[1], input[2]);
        vector<int> output = plusOne(input);
        vector<int> expected = {1, 2, 4};
        assert(output == expected);
    }
    {
        vector<int> input = {9};
        printf("test: input: [%d]\n", input[0]);
        vector<int> output = plusOne(input);
        vector<int> expected = {1, 0};
        assert(output == expected);
    }
}
