#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MIN_NUMBER 1000
#define MAX_NUMBER ULLONG_MAX

unsigned long long int min_num = MIN_NUMBER;
unsigned long long int max_num = MAX_NUMBER;

static volatile sig_atomic_t sigusr1_flag = 0;
static void sigusr1_handler(int signo) {
    (void)signo;

    sigusr1_flag = 1;
}

static unsigned long long int powi(unsigned long int base, unsigned short int exponent) {
    unsigned long long int result = base;

    while (--exponent) {
        result *= base;
    }

    return result;
}

static unsigned short int integer_length(unsigned long long int integer) {
    unsigned short int counter = 1;
    unsigned long long int multiplier = 1;

    while (1) {
        if (integer / multiplier < 10) {
            break;
        }

        ++counter;
        multiplier *= 10;
    }

    return counter;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s <parallelism count>\n", argv[0]);
        exit(1);
    }

    time_t start_time = time(NULL);

    pid_t ppid;
    ppid = getppid();
    printf("parent PID: %d\n", ppid);

    long int procs = strtol(argv[1], NULL, 10);

    // get process group id
    pid_t proc_group_id = getpgrp();
    printf("process group ID: %d\n", proc_group_id);

    // get number of online processors
    int cpu_online = sysconf(_SC_NPROCESSORS_ONLN);

    if (procs > cpu_online) {
        printf("WARNING: parallelism count %ld is greater than number of online processors %d\n", procs, cpu_online);
        printf("Using %d as the parallelism count\n", cpu_online);

        procs = cpu_online;
    }

    unsigned long long int range = max_num - min_num + 1;
    unsigned long long int element = range / procs;
    unsigned long long int mod = range % procs;

    printf("range: %llu element: %llu mod %llu\n", range, element, mod);

    // create some children
    for (int i = 0; i < procs; ++i) {
        pid_t child_pid;

        child_pid = fork();

        if (child_pid > 0) {
            printf("child pid: %d\n", child_pid);
        }

        if (child_pid == 0) {
            if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
                perror("failed to install SIGUSR1 signal handler");
            }

            unsigned long long int init_num = min_num + (i * element);
            unsigned long long int end_num;

            if (i == (procs - 1)) {
                end_num = init_num + element + mod - 1;
            } else {
                end_num = init_num + element - 1;
            }

            printf("child index [%d]: init_num: %llu end_num: %llu\n", i, init_num, end_num);

            while (init_num <= end_num) {
                if (sigusr1_flag > 0) {
                    printf("child index [%d]: current number: %llu\n", i, init_num);
                    sigusr1_flag = 0;
                }

                unsigned short int length = integer_length(init_num);
                if (length % 2 != 0) {
                    init_num = powi(10, length);
                    printf("child index [%d]: new init_num: %llu\n", i, init_num);
                    if (init_num > end_num) {
                        printf("child index [%d]: init_num %llu > end_num %llu, quit\n", i, init_num, end_num);
                        break;
                    }
                }

                unsigned long long int partial_multiplier = powi(10, integer_length(init_num) / 2);
                unsigned long long int first = init_num / partial_multiplier;
                unsigned long long int post = init_num % partial_multiplier;

                if ((first + post) * (first + post) == init_num) {
                    if (integer_length(first) == integer_length(post)) {
                        time_t stop_time;
                        time_t duration;
                        stop_time = time(NULL);
                        duration = stop_time - start_time;

                        printf("child index [%d]: %llu %llu %llu %ld\n", i, first, post, init_num, duration);

                        FILE *file;
                        file = NULL;

                        file = fopen("result", "a");

                        if (file == NULL) {
                            perror("failed to open result file");
                        } else {
                            fprintf(file, "child index [%d]: %llu %llu %llu %ld\n", i, first, post, init_num, duration);
                            fflush(file);
                            fsync(fileno(file));
                            fclose(file);
                        }
                    }
                }

                ++init_num;
            }

            // must exit or child will go to loop again
            exit(0);
        }

        if (child_pid == -1) {
            perror("fork");
        }
    }

    // wait until all processes are reaped
    pid_t reaped_pid;
    int status;

    while ((reaped_pid = waitpid(-1, &status, 0)) > 0) {
        printf("child process PID %d is reaped with status %d\n", reaped_pid, status);
    }

    exit(0);
}
