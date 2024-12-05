/*
g++ ./9_palindrome_number.cpp && ./a.out

https://leetcode.com/problems/palindrome-number/description/

Given an integer x, return true if x is a palindrome, and false otherwise.

Example 1:

Input: x = 121
Output: true
Explanation: 121 reads as 121 from left to right and from right to left.

Example 2:

Input: x = -121
Output: false
Explanation: From left to right, it reads -121. From right to left, it becomes 121-. Therefore it is not a palindrome.

Example 3:

Input: x = 10
Output: false
Explanation: Reads 01 from right to left. Therefore it is not a palindrome.


Scratchpad:
* Easy case, all negative numbers are not palindromes
* Easy case 2, single digit numbers (0 - 9) are palindromes
* Find number of decimal places.
* Use modulo/divide to strip of the right most number, plop it onto a stack,
when you get half way, start removing from the stack and comparing.
*/


#include <string>
#include <cassert>
#include <cstdio>

int getDecimalPlaces(int x) {
    int cur = x;
    int decimalPlaces = 0;
    while (true) {
        if (cur == 0) {
            return decimalPlaces;
        }
        cur = cur / 10;
        decimalPlaces += 1;
    }
    return decimalPlaces;
}

bool isPalindrome(int x) {
    if (x < 0) {
        return false;
    } else if (x < 10) {
        return true;
    }

    int cur = x;
    int decimalPlaces = getDecimalPlaces(x);
    int stack[10];
    int stackIndex = 0;
    // Digit by digit, parse the rightmost half of the number and 
    // add it to a stack.
    for (int i = 0; i < decimalPlaces/2; i++) {
        stack[stackIndex] = cur % 10;
        cur = cur / 10;
        printf("   stack add [%d]: %d\n" , stackIndex, stack[stackIndex]);
        stackIndex += 1;
    }

    // If the number has an odd number of places, skip the middle.
    if (decimalPlaces % 2 == 1) {
        cur = cur / 10;
    }

    // Digit by digit, parse the leftmost half of the number and
    // compare it with whats on the stack.
    for (int i = 0; i < decimalPlaces/2; i++) {
        stackIndex -= 1;
        int temp = cur % 10;
        cur = cur / 10;
        printf("   stack remove [%d]: %d (actual: %d)\n" , stackIndex, temp, stack[stackIndex]);
        if (temp != stack[stackIndex]) {
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    int test_1_input = 121;
    bool test_1_output = isPalindrome(test_1_input);
    printf("test_1_input: '%d' - test_1_output: %d\n", test_1_input, test_1_output);
    assert(test_1_output == true);

    int test_2_input = -121;
    bool test_2_output = isPalindrome(test_2_input);
    printf("test_2_input: '%d' - test_2_output: %d\n", test_2_input, test_2_output);
    assert(test_2_output == false);

    int test_3_input = 1001;
    bool test_3_output = isPalindrome(test_3_input);
    printf("test_3_input: '%d' - test_3_output: %d\n", test_3_input, test_3_output);
    assert(test_3_output == true);

    int test_4_input = 11;
    bool test_4_output = isPalindrome(test_4_input);
    printf("test_4_input: '%d' - test_4_output: %d\n", test_4_input, test_4_output);
    assert(test_4_output == true);

    int test_5_input = 5512355;
    bool test_5_output = isPalindrome(test_5_input);
    printf("test_5_input: '%d' - test_5_output: %d\n", test_5_input, test_5_output);
    assert(test_5_output == false);
}
