/*
g++ ./67_add_binary.cpp && ./a.out

https://leetcode.com/problems/add-binary

Given two binary strings a and b, return their sum as a binary string.

Example 1:
Input: a = "11", b = "1"
Output: "100"

Example 2:
Input: a = "1010", b = "1011"
Output: "10101"

Constraints:
    1 <= a.length, b.length <= 10^4
    a and b consist only of '0' or '1' characters.
    Each string does not contain leading zeros except for the zero itself.


Notes:
* Initial thought was to be confused why binary is being stored as a string...
* Input strings can be longer than 64, so can't just convert to ints and write an adder.
* I used a vector for output below because I was feeling lazy. Could just use an array,
given the output size should be at most the longest input size + 1. If using an array,
then could also place directly into the correct place and not need the reverse.
*/
#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <cstdio>

using namespace std;

char addChar(char a, char b, bool inCarry, bool *outCarry) {
    if (a == '1' && b == '1') {
        *outCarry = true;
        if (inCarry) {
            return '1';
        } else {
            return '0';
        }
    } else if (a == '0' && b == '0') {
        *outCarry = false;
        if (inCarry) {
            return '1';
        } else {
            return '0';
        }
    } else {
        if (inCarry) {
            *outCarry = true;
            return '0';
        } else {
            *outCarry = false;
            return '1';
        }
    }
}

string addBinary(string a, string b) {
    vector<char> out;

    int aIndex = a.length() - 1;
    int bIndex = b.length() - 1;
    int longest = aIndex;
    if (bIndex > aIndex) {
        longest = bIndex;
    }

    bool inCarry = false;
    bool outCarry = false;

    for (int i = longest; i >= 0; i--) {
        char aChar = '0';
        if (aIndex >= 0) {
            aChar = a[aIndex];
        }
        char bChar = '0';
        if (bIndex >= 0) {
            bChar = b[bIndex];
        }
        
        char charOut = addChar(aChar, bChar, inCarry, &outCarry);
        // printf("i: %d, a: %c, b: %c, out: %c, inCarry: %d, outCarry: %d\n", 
        //     i, aChar, bChar, charOut, int(inCarry), int(outCarry));
        inCarry = outCarry;
        outCarry = false;
        out.push_back(charOut);
        aIndex -= 1;
        bIndex -= 1;
    }
    if (inCarry) {
        out.push_back('1');
    }
    reverse(out.begin(), out.end());
    return string(out.begin(), out.end());
}

int main(int argc, char* argv[]) {
    {
        string a = "11";
        string b = "1";
        string output = addBinary(a, b);
        printf("test: a: %s, b: %s, output: %s\n", a.c_str(), b.c_str(), output.c_str());
        assert(output.compare("100") == 0);
    }
    {
        string a = "1010";
        string b = "1011";
        string output = addBinary(a, b);
        printf("test: a: %s, b: %s, output: %s\n", a.c_str(), b.c_str(), output.c_str());
        assert(output.compare("10101") == 0);
    }
}
