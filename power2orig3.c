/*
# gcc compilation command
$ gcc -Wall -Wextra -Wpedantic -o power2orig3 power2orig3.c

!!! this application DOES NOT consider function reentrancy-safe !!!
*/
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* checkpoint filename */
#define CHECKPOINT_FILENAME "checkpoint"

/* global lock variables */
#define LOCK_FILENAME "power2orig.lock"
#define LOCK_FILENAME_PERM 0644
int lock_filename_fd;
short int lock_status;

/* initialized number */
unsigned long long int init_num = 1000;

/*
a function to set exclusive lock
*/
static short int set_lock(int fd) {
    /* set exclusive lock */
    int lock_return = flock(fd, LOCK_EX | LOCK_NB);

    if (lock_return == -1) {
        printf("WARNING: failed to set exclusive lock on lock file: %s\n", LOCK_FILENAME);
        close(fd);
        return -1;
    }

    printf("Exclusive lock is set on lock file: %s\n", LOCK_FILENAME);
    return 1;
}

/*
a function to unlock lock
*/
static short int unlock_lock(int fd) {
    /* unlock lock */
    int lock_return = flock(fd, LOCK_UN);

    if (lock_return == -1) {
        printf("WARNING: failed to unlock lock on lock file: %s\n", LOCK_FILENAME);
        close(fd);
        return -1;
    }

    printf("Exclusive lock is unlocked on lock file: %s\n", LOCK_FILENAME);
    close(fd);
    return 1;
}

/*
a function to write processing number into checkpoint file
*/
static void write_checkpoint() {
    FILE *checkpoint;
    checkpoint = fopen(CHECKPOINT_FILENAME, "w");

    if (checkpoint == NULL) {
        printf("WARNING: unable to open checkpoint file: %s\n", CHECKPOINT_FILENAME);
        return;
    }

    int write_return = fprintf(checkpoint, "%llu", init_num);

    fflush(checkpoint);
    fclose(checkpoint);

    if (write_return > 0) {
        printf("Processing number %llu is written to checkpoint file: %s\n", init_num, CHECKPOINT_FILENAME);
    } else {
        printf("WARNING: failed to write number to checkpoint file: %s\n", CHECKPOINT_FILENAME);
    }

    return;
}

/*
a function to read processing number from checkpoint file
*/
static unsigned long long int read_checkpoint() {
    unsigned long long int number;
    FILE *checkpoint;
    checkpoint = fopen(CHECKPOINT_FILENAME, "r");

    if (checkpoint == NULL) {
        printf("WARNING: unable to open checkpoint file: %s\n", CHECKPOINT_FILENAME);
        return init_num;
    }

    int read_return = fscanf(checkpoint, "%llu", &number);

    fclose(checkpoint);

    if (read_return == 1) {
        printf("Retrieved checkpoint number %llu from checkpoint file: %s\n", number, CHECKPOINT_FILENAME);
        return number;
    } else {
        printf("WARNING: failed to retrieve checkpoint number from checkpoint file: %s\n", CHECKPOINT_FILENAME);
        return init_num;
    }
}

/*
a signal handler to handle SIGINT signal, following operations will be triggered:
1. print the final processing number
2. trigger write_checkpoint() function to write processing number to checkpoint file 
*/
static void sigint_handler(int signo) {
    printf("\nSIGINT received, stopped at number: %llu\n", init_num);
    if (lock_status == 1) {
        unlock_lock(lock_filename_fd);
    }
    write_checkpoint(CHECKPOINT_FILENAME);
    exit (EXIT_SUCCESS);
}

/*
a signal handler to handle SIGUSR1 signal, following operations will be triggered:
1. print the current processing number
*/
static void sigusr1_handler(int signo) {
    printf("\nSIGUSR1 received, current processing number: %llu\n", init_num);
}

/*
a function to calculate power function in integer type
*/
static unsigned long long int powi(unsigned long int base, unsigned short int exponent) {
    unsigned long long int result = base;

    while (--exponent) {
        result *= base;
    }

    return result;
}

/*
a function to return length of an integer
*/
static unsigned short int integer_length(unsigned long long int integer) {
    unsigned short int counter = 1;
    unsigned long int multiplier = 1;

    while (1) {
        if (integer / multiplier < 10) {
            break;
        }

        ++counter;
        multiplier *= 10;
    }

    return counter;
}

int main() {
    /* initial variables */
    time_t start_time = time(NULL);
    time_t stop_time;
    pid_t current_pid = getpid();

    /* print current process PID */
    printf("current process PID: %d\n", current_pid);

    /* print maximum value of unsigned long long int type */
    printf("Maximum value of unsigned long long int type: %llu\n", ULLONG_MAX);

    /* set exclusive lock */
    lock_filename_fd = open(LOCK_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, LOCK_FILENAME_PERM);
    if (lock_filename_fd == -1) {
        printf("WARNING: failed to open lock file: %s\n", LOCK_FILENAME);
        lock_status = -1;
    } else {
        lock_status = set_lock(lock_filename_fd);
        if (lock_status == -1) {
            printf("Exclusive lock detected or set lock failed, bail out the program\n");
            close(lock_filename_fd);
            exit (EXIT_FAILURE);
        }
    }

    /* re-read initialized number from checkpoint file, using original value if the call fails*/
    init_num = read_checkpoint();
    printf("Using initialized number %llu\n", init_num);

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        printf("ERROR: failed to register SIGINT signal handler\n");
        if (lock_filename_fd != -1) {
            close(lock_filename_fd);
        }
        exit (EXIT_FAILURE);
    }

    if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
        printf("ERROR: failed to register SIGUSR1 signal handler\n");
        if (lock_filename_fd != -1) {
            close(lock_filename_fd);
        }
        exit (EXIT_FAILURE);
    }

    while (1) {
        /* exit loop once initialized number is equal to or greater than ULLONG_MAX */
        if (init_num >= ULLONG_MAX) {
            printf("Hits maximum value of unsigned long long int type: %llu\n", ULLONG_MAX);
            if (lock_filename_fd != -1) {
                close(lock_filename_fd);
            }
            exit (EXIT_SUCCESS);
        }

        if (integer_length(init_num) % 2 != 0) {
            init_num *= 10;
        }

        unsigned long int partial_multiplier = powi(10, integer_length(init_num) / 2);
        unsigned long int first = init_num / partial_multiplier;
        unsigned long int post = init_num % partial_multiplier;

        if ((first + post) * (first + post) == init_num) {
            stop_time = time(NULL);
            printf("%ld %ld %llu %ld\n", first, post, init_num, stop_time - start_time);
        }

        ++init_num;
    }

    return 0;
}
