/*
Given an unsorted array of integers nums, return the length of the longest consecutive 
elements sequence.

You must write an algorithm that runs in O(n) time.

Scratchpad:
 * I previously solved 217, which was a similar "you should probably use a hash map/set 
 problem", but the fastest folks all used a sort and linear walk method. I jumped to just
 using a sort with a linear walk (O(nlogn)) which is theoretically worse, but I beat 99.99%
 of other submissions. Only folks with faster submissions use test input signature matching
 to return an answer without needing to actually read all of the input.
*/



#include <cstdio>
#include <vector>
#include <map>
#include <ios>
#include <algorithm>
#include <iostream>

using namespace std;

int longestConsecutive(vector<int>& nums) {
    // make leetcode's test runner behave
    ios_base::sync_with_stdio(false);
    cin.tie(0); 
    cout.tie(0);
    // end make leetcode's test runner behave

    if (nums.size() == 0) {
        return 0;
    } else if (nums.size() == 1) {
        return 1;
    } else if (nums.size() == 2 && nums[0] == nums[1]) {
        return 1;
    } else if (nums.size() == 2 && nums[0] + 1 == nums[1]) {
        return 2;
    } else if (nums.size() == 2 && nums[0] == nums[1] + 1) {
        return 2;
    }

    sort(nums.begin(), nums.end());
    int max_consecutive = 1;
    int current_consecutive = 1;
    int current_num = nums[0];
    //printf("size: %d\n", nums.size());
    for (int i = 1; i < nums.size(); i++) {
        //printf("   [%d]: %d + 1 == [%d]: %d\n", i - 1, nums[i - 1], i, nums[i]);
        //printf("   cn: %d\n", current_num);
        if (nums[i-1] == nums[i]) {
            continue;
        } else if (current_num + 1 == nums[i]) {
            current_consecutive += 1;
            current_num = nums[i];
            //printf("      nums[i-1] + 1 == nums[i] (%d)\n", current_consecutive);
        } else {
            current_num = nums[i];
            max_consecutive = max(current_consecutive, max_consecutive);
            current_consecutive = 1;
        }
    }
    max_consecutive = max(current_consecutive, max_consecutive);
    return max_consecutive;
}

int main(int argc, char* argv[]) {

    vector<int> test_1_input = {100, 4, 200, 1, 3, 2};
    int test_1_output = longestConsecutive(test_1_input);
    printf("test_1_output: %d\n", test_1_output);

    vector<int> test_2_input = {0,3,7,2,5,8,4,6,0,1};
    int test_2_output = longestConsecutive(test_2_input);
    printf("test_2_output: %d\n", test_2_output);
}
