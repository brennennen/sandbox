/*
g++ ./1_two_sum.cpp && ./a.out

https://leetcode.com/problems/two-sum

1. Two Sum
Solved
Easy
Topics
Companies
Hint

Given an array of integers nums and an integer target, return indices of the two numbers such that they add up to target.

You may assume that each input would have exactly one solution, and you may not use the same element twice.

You can return the answer in any order.

Example 1:
Input: nums = [2,7,11,15], target = 9
Output: [0,1]
Explanation: Because nums[0] + nums[1] == 9, we return [0, 1].

Example 2:
Input: nums = [3,2,4], target = 6
Output: [1,2]

Example 3:
Input: nums = [3,3], target = 6
Output: [0,1]

Constraints:
    2 <= nums.length <= 104
    -109 <= nums[i] <= 109
    -109 <= target <= 109
    Only one valid answer exists.
*/

#include <vector>
#include <cassert>
#include <cstdio>

using namespace std;

vector<int> twoSum(vector<int>& nums, int target) {
    vector<int> result(2);
    for (int i = 0; i < nums.size(); i++) {
        for (int j = i + 1; j < nums.size(); j++) {
            if (nums[i] + nums[j] == target) {
                result[0] = i;
                result[1] = j;
                return result;
            } 
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    {
        vector<int> input = {2, 7, 11, 15};
        int target = 9;
        vector<int> output = twoSum(input, target);
        printf("test: output: [%d, %d]\n", output[0], output[1]);
        assert(0 == output[0]);
        assert(1 == output[1]);
    }
    {
        vector<int> input = {3, 2, 4};
        int target = 6;
        vector<int> output = twoSum(input, target);
        printf("test: output: [%d, %d]\n", output[0], output[1]);
        assert(1 == output[0]);
        assert(2 == output[1]);
    }
    {
        vector<int> input = {3, 3};
        int target = 6;
        vector<int> output = twoSum(input, target);
        printf("test: output: [%d, %d]\n", output[0], output[1]);
        assert(0 == output[0]);
        assert(1 == output[1]);
    }
}
