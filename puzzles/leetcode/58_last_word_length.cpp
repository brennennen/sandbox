/*
g++ ./58_last_word_length.cpp && ./a.out

https://leetcode.com/problems/length-of-last-word/description/

58. Length of Last Word

Given a string s consisting of words and spaces, return the length of the last word in the string. 
A word is a maximal substring consisting of non-space characters only.

Example 1:
Input: s = "Hello World"
Output: 5
Explanation: The last word is "World" with length 5.

Example 2:
Input: s = "   fly me   to   the moon  "
Output: 4
Explanation: The last word is "moon" with length 4.

Example 3:
Input: s = "luffy is still joyboy"
Output: 6
Explanation: The last word is "joyboy" with length 6.

Constraints:
    1 <= s.length <= 104
    s consists of only English letters and spaces ' '.
    There will be at least one word in s.
*/

#include <string>

#include <cassert>

using namespace std;

int lengthOfLastWord(string s) {
    bool found_word = false;
    int end = s.length() - 1;
    for (int i = s.length() - 1; i >= 0; i--) {
        if (!found_word && s[i] != ' ') {
            found_word = true;
            end = i;
        }
        if (found_word && s[i] == ' ') {
            return end - i;
        }
    }
    return end + 1;
}

int main(int argc, char* argv[]) {
    {
        string input = "Hello World";
        int output = lengthOfLastWord(input);
        printf("test: input: '%s', output: %d\n", input.c_str(), output);
        assert(output == 5);
    }
    {
        string input = "   fly me   to   the moon  ";
        int output = lengthOfLastWord(input);
        printf("test: input: '%s', output: %d\n", input.c_str(), output);
        assert(output == 4);
    }
    {
        string input = "luffy is still joyboy";
        int output = lengthOfLastWord(input);
        printf("test: input: '%s', output: %d\n", input.c_str(), output);
        assert(output == 6);
    }
}
