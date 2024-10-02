/*
We are playing the "Guess Game". The game is as follows:
* I Pick a number from 1 to 'n'. You have to guess which number I picked.
* Every time you guess wrong, I will tellyou whether the number I picked is higher
or lower than your guess.
* You call a pre-defined API `int guess(int num)`, which returns there possible
results:
    * `-1`: Your guess is higher than the number I picked (num > pick).
    * `1`: Your guess is lower than the number I picked (num < pick).
    * `0`: Your guess is equal to the number I picked (num == pick).

Scratch
* Binary search problem.
* Guess halfway between 0 and max_range. Then guess in the corresponding direction
half way again.

Ex: n = 10, pick = 6

low = 1
high = 10
middle = low + ((high - low) / 2) 
middle = 0 + ((10 - 0) / 2) 
middle = 5

1. guess 5 == return 1
    * 5 = 0 + ((10 - 0) / 2)
2. guess 7 == return -1
    * 7 = 5 + ((10 - 5) / 2)
3. guess 6 == return 0 :
    * 6 = 5 + ((7 - 5) / 2)
*/

#include <cstdio>
#include <cstdlib>


int guess(int n);

// HIDDEN FUNCTION IMPLEMENTATION
// The question depends on not being able to see the implementation in the section below
// or access it's data.
static int number_max_range = 10;
static int number_to_guess = 0;
int guess(int n) {
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

// Function to implement.
int guessNumber(int n) {
    int low = 1;
    int high = n;

    // Guess the bounds first.
    if (guess(1) == 0) {
        return 1;
    } else if (guess(n) == 0) {
        return n;
    }

    while(true) {
        int middle = low + ((high - low) / 2);
        int guess_result = guess(middle);

        if (guess_result == -1) {
            // Guess is too high, shift binary search upper window down to middle.
            high = middle;
        } else if (guess_result == 1) {
            // Guess is too low, shift binary search lower window up to middle.
            low = middle;
        } else {
            // Guess is correct, return.
            return middle;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    number_max_range = 10;
    number_to_guess = 6;
    int result = guessNumber(number_max_range);
    printf("guess: %d, result: %d\n", number_to_guess, result);
}
