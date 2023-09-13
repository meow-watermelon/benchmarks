/*
# gcc compilation command
$ gcc -Wall -o power2orig2 power2orig2.c
*/
#include <stdio.h>
#include <time.h>

long int powi(long int base, short int exponent) {
    long int result = base;

    while (--exponent) {
        result *= base;
    }

    return result;
}

short int integer_length(long int integer) {
    short int counter = 1;
    long int multiplier = 1;

    while (1) {
        if (integer / multiplier >= 0 && integer / multiplier < 10) {
            break;
        }

        ++counter;
        multiplier *= 10;
    }

    return counter;
}

int main() {
    /* initial variables */
    long int init_num = 1000;
    time_t start_time = time(NULL);
    time_t stop_time;

    while (1) {
        if (integer_length(init_num) % 2 != 0) {
            init_num *= 10;
        }

        long int partial_multiplier = powi(10, integer_length(init_num) / 2);
        long int first = init_num / partial_multiplier;
        long int post = init_num % partial_multiplier;

        if ((first + post) * (first + post) == init_num) {
            stop_time = time(NULL);
            printf("%ld %ld %ld %ld\n", first, post, init_num, stop_time - start_time);
        }

        ++init_num;
    }

    return 0;
}
