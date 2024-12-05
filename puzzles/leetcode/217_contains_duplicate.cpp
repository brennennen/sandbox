/*
g++ ./217_contains_duplicate.cpp && ./a.out

https://leetcode.com/problems/contains-duplicate/

Given an integer array nums, return true if any value appears more than once in 
the array, otherwise return false.

Example 1: 
     input: [1, 2, 3, 3]
     output: true

Example 2:
     input: [1, 2, 3, 4]
     output: false
*/

#include <cstdio>
#include <vector>
#include <map>

using namespace std;


bool hasDuplicate(vector<int>& nums) {
    // 1. A simple hash set, where the integer is the key would be a straight forward simple solution. c++ doesn't have 
    //hash sets, so a hash map might be the best call.
    // 2. If the size of nums is reasonably small (< ~100), it might be faster to just iterate over each num every nums 
    // and use a simple loop. Maybe with the addition of some simple caching.
    // 3. If the data was sorted, you could just walk the array and check if the current index is the same as the 
    // previous. Sorting would probably take longer than any of the methods above though.
    // 4. Along the same line of thought as 3, you could use a data structure that sorts as you insert (some kind 
    // of self balancing bst).
    //
    // If the range of nums is small, ex 0 - 30. Then you could just create an array of 30 elements and use the array 
    // index as hash id for each number.

    // Going with the simple hash solution as it has the best theoretical big o, which is probably what interview folks 
    // are typically looking for.
    map<int, int> hash_map;
    for (const int& num : nums) {
        if (hash_map.count(num) == 0) {
            hash_map[num] = 1;
        } else {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[]) {

    vector<int> test_1_input = {1, 2, 3, 3};
    bool test_1_output = hasDuplicate(test_1_input);
    printf("test_1_output: %d\n", (int) test_1_output);

    vector<int> test_2_input = {1, 2, 3, 4};
    bool test_2_output = hasDuplicate(test_2_input);
    printf("test_2_output: %d\n", (int) test_2_output);
}

/*
Analysis on the top answers.
 * Leetcode has a major performance penalty around setting up the "Solution" class and how the use iostream to handle 
 test runners. You seem to be able to speed things up a bit with the couple calls below.
```
 auto init = []()
{
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    return 0;
}();
```
 * 'set' solutions which use a tree (O(logn) insertions) under the hood were generally faster than map solutions 
 (O(1) insertions). Just a reminder that big O notation is misleading and questions this abstract should probably not 
 be asked.
 * Sorting with quicksort and then just checking i vs i+1 seems to have been the fastest sane solution.
```
class Solution {
public:
    bool containsDuplicate(vector<int>& nums) {
        ios_base::sync_with_stdio(false);
        cin.tie(0); 
        cout.tie(0);
        sort(nums.begin(), nums.end());
        for (int i = 1; i < nums.size(); ++i){
            if (nums[i] == nums[i-1]) 
                return true;
        }
        return false;
    }
};

```
 * The absolute fastest solutions all exploited knowledge they gained of the test data. They'd identify a pattern in 
 the numbers in a test case then just short circuit and not even bother reading the rest of the input data, which is a 
 level of gamifaction that I'm not willing to take part in.

In my opinion, in the real world, you would analyze the data you have and be able to make more concrete assertions about 
it before choosing your data structures. The fact that sorting is faster than a set or a map here isn't an indication
a candidate is unintelligent. It's because the test cases, which are not provided to the candidate, just so happen
to be written in a way that favor sorting. If the data was more randomized or flip-flopped betwen forward and reverse
sorted (to hit worst case scenarios for sorting algorithms), the bst set or hash set solutions may have became the fastest.
*/
