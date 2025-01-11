/*
g++ -std=c++23 ./202_happy_number.cpp && ./a.out

https://leetcode.com/problems/happy-number

Write an algorithm to determine if a number n is happy.

A happy number is a number defined by the following process:

    Starting with any positive integer, replace the number by the sum of the squares of its digits.
    Repeat the process until the number equals 1 (where it will stay), or it loops endlessly in a cycle which does not include 1.
    Those numbers for which this process ends in 1 are happy.

Return true if n is a happy number, and false if not.

Example 1:
Input: n = 19
Output: true
Explanation:
12 + 92 = 82
82 + 22 = 68
62 + 82 = 100
12 + 02 + 02 = 1

Example 2:
Input: n = 2
Output: false

Constraints:
    1 <= n <= 231 - 1
*/

#include <set>

#include <cstdio>
#include <cassert>


using namespace std;

int happy_iteration(int n) {
    int result = 0;
    while (n != 0) {
        int digit = n % 10;
        result += digit * digit;
        n = n / 10;
    }
    return result;
}

bool isHappy(int n) {
    set<int> visited;

    while (true) {
        n = happy_iteration(n);
        if (n == 1) { 
            return true;
        }
        if (visited.contains(n)) {
            return false;
        }
        visited.insert(n);
    }
}

int main(int argc, char* argv[]) {
    {
        int input = 19;
        bool output = isHappy(input);
        printf("test: input: %d, output: %d\n", input, output);
        assert(output == true);
    }
    {
        int input = 2;
        bool output = isHappy(input);
        printf("test: input: %d, output: %d\n", input, output);
        assert(output == false);
    }
}
