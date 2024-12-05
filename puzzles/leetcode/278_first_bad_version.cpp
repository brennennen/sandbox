/*
g++ ./278_first_bad_version.cpp && ./a.out

https://leetcode.com/problems/first-bad-version/

You are a product manager and currently leading a team to develop a new product. 
Unfortunately, the latest version of your product fails the quality check. Since 
each version is developed based on the previous version, all the versions after a 
bad version are also bad.

Suppose you have n versions [1, 2, ..., n] and you want to find out the first bad 
one, which causes all the following ones to be bad.

You are given an API bool isBadVersion(version) which returns whether version is 
bad. Implement a function to find the first bad version. You should minimize the 
number of calls to the API.

Example 1:
    Input: n = 5, bad = 4
    Output: 4
    Explanation:
    call isBadVersion(3) -> false
    call isBadVersion(5) -> true
    call isBadVersion(4) -> true
    Then 4 is the first bad version.

Example 2:
    Input: n = 1, bad = 1
    Output: 1


Scratch:
* Need to minimize calls to "isBadversion(version)".
* Looks like a binary search problem.
* First take the entire array of versions as your window with the middle most element
being the first version to test.
* If the version is bad, then shift the window such that the top most element is now
the last element tested (the middle) and repeat the process.
* If the version is good, then shift the window such that the bottom most element is
now the last element tested (the middle) and repeat the process.
*/


#include <cstdio>

bool isBadVersion(int version);

// HIDDEN FUNCTION IMPLEMENTATION
// The question depends on not being able to see the implementation in the section below
// or access it's data.
static int number_max_range = 10;
static int bad_ = 5;
bool isBadVersion(int version) {
    if (number_to_guess == 0) {
        number_to_guess = (rand() % number_max_range - 1) + 1; // range: 1 - max_range
    }

    if (n > number_to_guess)
        return -1;
    else if (n < number_to_guess)
        return 1;
    else // ==
        return 0;
}
// END HIDDEN FUNCTION IMPLEMENTATION

int firstBadVersion(int n) {
    int low = 1;
    int high = n;
    bool last_mid_bad = isBadVersion(n);
    int last_mid = n;

    while(true) {
        int mid = low + ((high - low) / 2);
        bool bad_version = isBadVersion(mid);

        if (last_mid_bad != bad_version) {
            if (abs(last_mid - mid) == 1) {
                
            }
        }


        if (isBadVersion(mid)) {
            high = mid;
        } else {
            low = mid;
        }
    }
}

int main(int argc, char* argv[]) {

}
