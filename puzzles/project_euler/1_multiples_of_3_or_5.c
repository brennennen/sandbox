/*
gcc ./1_multiples_of_3_or_5.c && ./a.out

https://projecteuler.net/problem=1
*/

#include <stdio.h>

// First brute force approach without putting too much thought in.
int multiples_of_3_or_5__naive(int up_to) {
    int sum = 0;
    for(int i = 0; i < up_to; i++) {
        if (i % 3 == 0 || i % 5 == 0) {
            sum += i;
        }
    }
    return sum;
}

/*
Thinking about the first approach, divides/modulos are expensive and I was curious if 
there was maybe a route that avoided them. This has too many branches now though.
*/
int multiples_of_3_or_5__no_divide(int up_to) {
    int sum = 0;
    int threes_counter = 0;
    int fives_counter = 0;
    for(int i = 1; i < up_to; i++) {
        threes_counter += 1;
        fives_counter += 1;
        if (threes_counter == 3 && fives_counter == 5) {
            sum += i;
            threes_counter = 0;
            fives_counter = 0;
        } else if (threes_counter == 3) {
            sum += i;
            threes_counter = 0;
        } else if (fives_counter == 5) {
            sum += i;
            fives_counter = 0;
        }
    }
    return sum;
}

/*
Thinking about the "no divide" approach, branches are expensive and I came up with the
below as a way to eliminate them and separate the "chunks" of work to maybe allow for
parallelization.

Optimization wise, without spending many hours benchmarking, this looks like a good
"stopping point". If I wanted to go further, I'd probably start with loop unrolling 
and some SIMD. If each loop took a non-trivial amount of time, they could be each run 
on their own thread and then composed at the end. "up to 1000", is probably too few 
iterations to justify the expensive OS syscalls needed to spawn threads though.
*/
int multiples_of_3_or_5__no_branch(int up_to) {
    int threes_sum = 0;
    for (int i = 0; i < up_to; i += 3) {
        threes_sum += i;
    }

    int fives_sum = 0;
    for (int i = 0; i < up_to; i += 5) {
        fives_sum += i;
    }

    // multiples of both get double counted, double back at the end and subtract them.
    int fifteens_sum = 0;
    for (int i = 0; i < up_to; i += 15) {
        fifteens_sum += i;
    }
    return threes_sum + fives_sum - fifteens_sum;
}

int main(int argc, char* arv[]) {
    int naive = multiples_of_3_or_5__naive(1000);
    int no_divide = multiples_of_3_or_5__no_divide(1000);
    int no_branch = multiples_of_3_or_5__no_branch(1000);
    
    printf("Multiples of 3 or 5 (naive): %d\n", naive);
    printf("Multiples of 3 or 5 (no division): %d\n", no_divide);
    printf("Multiples of 3 or 5 (no branch): %d\n", no_branch);
}
