/**
 * https://leetcode.com/problems/remove-duplicates-from-sorted-array/
 * Given an integer array 'nums' sorted in non-decreasing order, remove the
 * duplicates in-place such that each unique element appears only once. The
 * relative order of the elements should be kept the same. Then return the
 * number of unique elements in 'nums'.
 * 
 * Consider the number of unique elements of 'nums' to be 'k', to get accepted,
 * you need to do the following things:
 * * Change the array 'nums' such that the first 'k' elements of 'nums' contain
 * the unique elements in the order they were present in 'nums' initially. The
 * remaining elements of 'nums' arre not important as well as the size of 'nums'.
 * * Return 'k'.
 * 
 * Example 1: 
 *      input: [1, 1, 2]
 *      output: 2, nums = [1, 2, _]
 * 
 * Example 2:
 *      input: [0, 0, 1, 1, 1, 2, 2, 3, 3, 4]
 *      output: 5, nums = [0, 1, 2, 3, 4, _, _, _, _, _]
 */

// g++ ./26_remove_duplicates_from_sorted_array.cpp && ./a.out

#include <cstdio>
#include <vector>

using namespace std;


int removeDuplicates(vector<int>& nums) {
    // make leetcode's test runner behave
    //ios_base::sync_with_stdio(false);
    //cin.tie(0); 
    //cout.tie(0);
    // end make leetcode's test runner behave

    int unique_count = 1;
    for (int i = 1; i < nums.size(); i++) {
        if (nums[i] != nums[i - 1]) {
            nums[unique_count] = nums[i];
            unique_count += 1;
        }
    }
    nums.resize(unique_count);

    return unique_count;
}

void print_int_vec(vector<int>& nums) {
    printf("vec: ");
    for (int i: nums) {
        printf("%d, ", i);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    vector<int> test_1_input = {1, 1, 2};
    int test_1_output = removeDuplicates(test_1_input);
    printf("test_1_output: %d\n", (int) test_1_output);
    print_int_vec(test_1_input);

    vector<int> test_2_input = {0, 0, 1, 1, 1, 2, 2, 3, 3, 4};
    int test_2_output = removeDuplicates(test_2_input);
    printf("test_2_output: %d\n", (int) test_2_output);
    print_int_vec(test_2_input);
}
