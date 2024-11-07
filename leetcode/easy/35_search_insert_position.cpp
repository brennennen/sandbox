/*
g++ ./35_search_insert_position.cpp && ./a.out

https://leetcode.com/problems/search-insert-position/

Given a sorted array of distinct integers and a target value, return the index if the target is found. If not, return the index where it would be if it were inserted in order.

You must write an algorithm with O(log n) runtime complexity.

 
Example 1:

Input: nums = [1,3,5,6], target = 5
Output: 2

Example 2:

Input: nums = [1,3,5,6], target = 2
Output: 1

Example 3:

Input: nums = [1,3,5,6], target = 7
Output: 4
*/


#include <vector>
#include <cassert>
#include <cstdio>

using namespace std;

int searchInsert(vector<int>& nums, int target) {
    if (target > nums.back()) {
        return nums.size();
    } else if (target < nums.front()) {
        return 0;
    }

    int left_index = 0;
    int right_index = nums.size() - 1;

    while(true) {
        if (target == nums[left_index]) {
            return left_index;
        } else if (target == nums[right_index]) {
            return right_index;
        }

        int middle_index = left_index + ((right_index - left_index) / 2);

        // printf("left: %d (i: %d), right: %d (i: %d), middle: %d (i: %d)\n", 
        //     nums[left_index], left_index, nums[right_index], right_index, 
        //     nums[middle_index], middle_index);

        if (target == nums[middle_index]) {
            return middle_index;
        } else if (target > nums[middle_index]) {
            //printf("moving left index: %d -> %d\n", left_index, middle_index);
            left_index = middle_index;
        } else {
            //printf("moving right index: %d -> %d\n", right_index, middle_index);
            right_index = middle_index;
        }

        if (left_index + 1 == right_index) {
            return left_index + 1;
        }
    }
}

int main(int argc, char* argv[]) {
    {
        vector<int> input = {1, 3, 5, 6};
        int target = 5;
        int output = searchInsert(input, target);
        printf("test_1: %d\n", output);
        assert(output == 2);
    }

    {
        vector<int> input = {1, 3, 5, 6};
        int target = 2;
        int output = searchInsert(input, target);
        printf("test_2: %d\n", output);
        assert(output == 1);
    }

    {
        vector<int> input = {1, 3, 5, 6};
        int target = 7;
        int output = searchInsert(input, target);
        printf("test_3: %d\n", output);
        assert(output == 4);
    }
}
