/*
g++ ./49_group_anagrams.cpp && ./a.out

https://leetcode.com/problems/group-anagrams

49. Group Anagrams
Solved
Medium
Topics
Companies

Given an array of strings strs, group the
anagrams
together. You can return the answer in any order.

Example 1:
Input: strs = ["eat","tea","tan","ate","nat","bat"]
Output: [["bat"],["nat","tan"],["ate","eat","tea"]]

Explanation:
    There is no string in strs that can be rearranged to form "bat".
    The strings "nat" and "tan" are anagrams as they can be rearranged to form each other.
    The strings "ate", "eat", and "tea" are anagrams as they can be rearranged to form each other.

Example 2:
Input: strs = [""]
Output: [[""]]

Example 3:
Input: strs = ["a"]
Output: [["a"]]
 

Constraints:
    1 <= strs.length <= 104
    0 <= strs[i].length <= 100
    strs[i] consists of lowercase English letters.
*/

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include <cstdio>
#include <cstring>
#include <cassert>

using namespace std;

vector<vector<string>> groupAnagrams(vector<string>& strs) {
    vector<vector<string>> result;
    unordered_map<string, vector<string>> anagramMap;

    for (int i = 0; i < strs.size(); i++) {
        string sortedWord = strs[i];
        sort(sortedWord.begin(), sortedWord.end());
        anagramMap[sortedWord].push_back(strs[i]);
    }

    for(auto& [key, value]: anagramMap) {
        result.push_back(value);
    }

    return result;
}

void debug_vector_string_print(vector<string>& strings) {
    printf("[");
    for (int i = 0; i < strings.size() - 1; i++) {
        printf("%s, ", strings[i].c_str());
    }
    printf("%s]\n", strings[strings.size() - 1].c_str());
}

void debug_vector_vector_string_print(vector<vector<string>>& strings) {
    printf("[");
    for (int i = 0; i < strings.size(); i++) {
        printf("[");
        for (int j = 0; j < strings[i].size(); j++) {
            printf("%s, ", strings[i][j].c_str());
        }
        printf("]");
    }
    printf("]\n");
}

int main(int argc, char* argv[]) {
    {
        vector<string> input = {"eat", "tea", "tan", "ate", "nat", "bat"};
        vector<vector<string>> output = groupAnagrams(input);
        printf("input: ");debug_vector_string_print(input);printf("\n");
        printf("output: ");debug_vector_vector_string_print(output);printf("\n");
        assert(3 == output.size());
        assert(strcmp("bat", output[0][0].c_str()) == 0);
    }
}
