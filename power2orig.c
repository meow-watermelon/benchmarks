/*
# gcc compilation command
$ gcc -Wall -o power2orig power2orig.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define STRING_SIZE 2048

/* initial variables */
time_t start_time;

long int init_num = 1000;
char init_num_str[STRING_SIZE];

/* struct to store number partials (half / half) */
struct split_int {
    long int first;
    long int post;
};

struct split_int get_split_int(char *number_str) {
    struct split_int output;
    char *first_part;
    char *post_part;
    size_t str_length = strlen(number_str);
    int splitter = str_length / 2;
    
    /* split a number into half */
    post_part = number_str + splitter;
    output.post = atol(post_part);

    *(number_str + splitter) = '\0';
    first_part = number_str;
    output.first = atol(first_part);

    /* 
    DO NOT call free() on pointers that are not created by malloc() or related functions.
    ref.: https://wiki.sei.cmu.edu/confluence/display/c/MEM34-C.+Only+free+memory+allocated+dynamically
    */
    first_part = NULL;
    post_part = NULL;

    return output;
}

int main() {
    start_time = time(NULL);
    time_t result_time;

    while (1) {
        sprintf(init_num_str, "%ld", init_num);

        if (strlen(init_num_str) % 2 != 0) {
            init_num *= 10;
        }

        struct split_int split_output;
        split_output = get_split_int(init_num_str);
        long int compute_part = split_output.first + split_output.post;

        if (compute_part * compute_part  == init_num) {
            result_time = time(NULL);
            printf("%ld %ld %ld %ld\n", split_output.first, split_output.post, init_num, result_time - start_time);
        }

        init_num += 1;
    }

    return 0;
}
