/*
g++ ./28_find_index_of_first_substring.cpp && ./a.out

https://leetcode.com/problems/find-the-index-of-the-first-occurrence-in-a-string

Given two strings needle and haystack, return the index of the first occurrence of needle 
in haystack, or -1 if needle is not part of haystack.

Example 1:

Input: haystack = "sadbutsad", needle = "sad"
Output: 0
Explanation: "sad" occurs at index 0 and 6.
The first occurrence is at index 0, so we return 0.

Example 2:

Input: haystack = "leetcode", needle = "leeto"
Output: -1
Explanation: "leeto" did not occur in "leetcode", so we return -1.
*/

#include <string>
#include <vector>
#include <cassert>
#include <cstdio>

using namespace std;

int strStr(string haystack, string needle) {
    for (int i = 0; i < haystack.length(); i++) {
        if (haystack[i] != needle[0]) {
            continue;
        } else if (i + needle.length() > haystack.length()) {
            //printf("needle doesn't fit, return -1...\n");
            return -1;
        }

        bool match = true;
        for (int j = 0; j < needle.length(); j++) {
            //printf("%c == %c\n", haystack[i + j], needle[j]);
            if (haystack[i + j] != needle[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char* argv[]) {
    string test_1_haystack = "sadbutsad";
    string test_1_needle = "sad";
    int test_1_output = strStr(test_1_haystack, test_1_needle);
    printf("test_1: haystack: %s, needle: %s, output: %d\n", 
        test_1_haystack.c_str(), test_1_needle.c_str(), test_1_output);
    assert(test_1_output == 0);

    string test_2_haystack = "leetcode";
    string test_2_needle = "leeto";
    int test_2_output = strStr(test_2_haystack, test_2_needle);
    printf("test_2: haystack: %s, needle: %s, output: %d\n", 
        test_2_haystack.c_str(), test_2_needle.c_str(), test_2_output);
    assert(test_2_output == -1);

    string test_3_haystack = "abcdefghijklmnopqrstuvwxyz";
    string test_3_needle = "jkl";
    int test_3_output = strStr(test_3_haystack, test_3_needle);
    printf("test_3: haystack: %s, needle: %s, output: %d\n", 
        test_3_haystack.c_str(), test_3_needle.c_str(), test_3_output);
    assert(test_3_output == 9);
}
