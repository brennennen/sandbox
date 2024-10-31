/*
g++ ./26_remove_duplicates_from_sorted_array.cpp && ./a.out

https://leetcode.com/problems/roman-to-integer/

Roman numerals are represented by seven different symbols: I, V, X, L, C, D and M.

Symbol       Value
I             1
V             5
X             10
L             50
C             100
D             500
M             1000

For example, 2 is written as II in Roman numeral, just two ones added together. 12 is written as XII, which is simply X + II. The number 27 is written as XXVII, which is XX + V + II.

Roman numerals are usually written largest to smallest from left to right. However, the numeral for four is not IIII. Instead, the number four is written as IV. Because the one is before the five we subtract it making four. The same principle applies to the number nine, which is written as IX. There are six instances where subtraction is used:

    I can be placed before V (5) and X (10) to make 4 and 9. 
    X can be placed before L (50) and C (100) to make 40 and 90. 
    C can be placed before D (500) and M (1000) to make 400 and 900.

Given a roman numeral, convert it to an integer.

Example 1:

Input: s = "III"
Output: 3
Explanation: III = 3.

Example 2:

Input: s = "LVIII"
Output: 58
Explanation: L = 50, V= 5, III = 3.

Example 3:

Input: s = "MCMXCIV"
Output: 1994
Explanation: M = 1000, CM = 900, XC = 90 and IV = 4.
*/

#include <string>
#include <cassert>

using namespace std;

int romanToInt(string s) {
    int number = 0;
    int index = 0;

    char prev = '\0';
    char cur = s[index];

    bool done = false;
    while (!done) {
        if (index == s.length() - 1) {
            done = true;
        }

        if (cur == 'I') {
            number += 1;
        } else if (cur == 'V') {
            if (prev == 'I') {
                number += 4;
                number -= 1;
            } else {
                number += 5;
            }
        } else if (cur == 'X') {
            if (prev == 'I') {
                number += 9;
                number -= 1;
            } else {
                number += 10;
            }
        } else if (cur == 'L') {
            if (prev == 'X') {
                number += 40;
                number -= 10;
            } else {
                number += 50;
            }
        } else if (cur == 'C') {
            if (prev == 'X') {
                number += 90;
                number -= 10;
            } else {
                number += 100;
            }
        } else if (cur == 'D') {
            if (prev == 'C') {
                number += 400;
                number -= 100;
            } else {
                number += 500;
            }
        } else if (cur == 'M') {
            if (prev == 'C') {
                number += 900;
                number -= 100;
            } else {
                number += 1000;
            }
        }
        index += 1;

        prev = cur;
        cur = s[index];
    }
    return number;
}

int main(int argc, char* argv[]) {
    string test_1_input = "III";
    int test_1_output = romanToInt(test_1_input);
    printf("test_1_input: '%s' - test_1_output: %d\n", test_1_input.c_str(), test_1_output);
    assert(test_1_output == 3);

    string test_2_input = "LVIII";
    int test_2_output = romanToInt(test_2_input);
    printf("test_2_input: '%s' - test_2_output: %d\n", test_2_input.c_str(), test_2_output);
    assert(test_2_output == 58);

    string test_3_input = "MCMXCIV";
    int test_3_output = romanToInt(test_3_input);
    printf("test_3_input: '%s' - test_3_output: %d\n", test_3_input.c_str(), test_3_output);
    assert(test_3_output == 1994);
}
