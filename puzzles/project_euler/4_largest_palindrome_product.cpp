/*
g++ -std=c++17 ./4_largest_palindrome_product.cpp && ./a.out

https://projecteuler.net/problem=4
*/

#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cmath>

/*
First thoughts:
* Create 2 pointers, a "left" pointer, which starts out by pointing at the most significant
digit, and a "right" pointer, which starts out by pointing at the least significant digit.
* Then compare the digits each the left and right pointer are pointing at and move them inward
if they match or short circuit and return early if they do not.

It turned out to be non-trivial to access the most significant digit of a number in an
efficient way. You end up with a lot of expensive exponent and division math. I ended up with
a non-functional solution going down this path that couldn't palindrome inputs with 0's in 
them. There's probably a way to get the below working by keeping track of significant places,
but I think going a different route altogether is more appropriate (see second pass below).
*/
inline int count_digits(int number) { 
    int digits = 0;
    while (number != 0) {
        number = number / 10;
        digits += 1;
    }
    return digits;
}

inline int get_leading(int number) {
    while (number >= 10) {
        number = number / 10;
    }
    return number;
}

bool is_palindrome_bad(int number) { // DOESN'T WORK!
    if (number < 10) { return true; }
    int digits = count_digits(number);
    // NOTE:
    // left/right here is in relation to decimal/tens place significance.
    // left -> 100 <- right (left_index = 3, right_index = 0)
    int left_place = digits;
    int left_running = number;
    int right_place = 0;
    int right_running = number;
    while (left_place >= right_place) {
        int left = get_leading(left_running);
        //printf("   pow: %f = %d\n", std::pow(10, left_place), (int)std::pow(10, left_place));
        left_running = number % (int)std::pow(10, left_place); // DOESN'T WORK! cuts off too much when their are 0s in the middle of the number...
        //printf("   left_running: %d\n", left_running);
        left_place -= 1;

        int right = right_running % 10;
        right_running = right_running / 10;
        printf("l: %d, r: %d (li: %d, ri: %d, lr: %d, rr: %d)\n", 
           left, right, left_place, right_place, left_running, right_running);
        right_place += 1;

        if (left != right) {
            return false;
        }
    }
    return true;
}

void first_pass_asserts() {
    assert(count_digits(1) == 1);
    assert(count_digits(10) == 2);
    assert(count_digits(999) == 3);

    assert(get_leading(1) == 1);
    assert(get_leading(20) == 2);
    assert(get_leading(321) == 3);
}

/*
Second pass:
There are a lot of divisions going down this route. If there was a clever way to get the number
of digits in the integer beforehand, maybe exploiting the incrementing nature of the loop or
the multiplication somewhow, the above solution might work out well. However, I couldn't find
a good solution going this route and for a second pass. Doubling back a bit, it's probably
simpler to just reverse the number and then check if the reverse is equal. I still think there
is something clever to be done with the 2 pointer route if I mulled over it enough, however I 
am trying to limit the time spent on these problems.
*/

int reverse_number(int number) {
    if (number < 10) { return number; }

    int reversed_number = 0;
    while (number != 0) {
        reversed_number = reversed_number * 10;
        reversed_number += number % 10;
        number = number / 10;
    }
    return reversed_number;
}

bool is_palindrome(int number) {
    int reversed_number = reverse_number(number);
    if (number == reversed_number) {
        return true;
    }
    return false;
}

int largest_palindrome_product(int up_to) {
    int largest_palindrome = 0;

    for (int i = 0; i <= up_to; i++) {
        for (int j = 0; j <= up_to; j++) {
            int num = i * j;
            if (num > largest_palindrome) {
                if (is_palindrome(num)) {
                    largest_palindrome = num;
                }
            }
        }
    }

    return largest_palindrome;
}

int main(int argc, char* arv[]) {
    assert(is_palindrome(1) == true);
    assert(is_palindrome(20) == false);
    assert(is_palindrome(22) == true);
    assert(is_palindrome(303) == true);
    assert(is_palindrome(9009) == true);
    assert(is_palindrome(9409) == false);
    assert(is_palindrome(123321) == true);
    assert(is_palindrome(1234321) == true);
    
    assert(largest_palindrome_product(99) == 9009);
    printf("largest_palindrome_product(999): %d\n", largest_palindrome_product(999));
}
