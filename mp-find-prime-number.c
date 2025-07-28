/*
 * compilation: gcc -Wall -Wextra -Wpedantic -o mp-find-prime-number mp-find-prime-number.c -lm -lpthread -lrt 
 */

#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MIN_NUMBER 2
#define OUTPUT "prime-number"
#define SHM_FILENAME "/mp-find-prime-number-shm"
#define SEM_FILENAME "/mp-find-prime-number"

unsigned long long int min_num = MIN_NUMBER;

static volatile sig_atomic_t sigusr1_flag = 0;
static void sigusr1_handler(int signo) {
    (void)signo;

    sigusr1_flag = 1;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage: %s <parallelism count> <maximum number limit>\n", argv[0]);
        exit(1);
    }

    unsigned long int *prime_counter;

    // delete output
    if (unlink(OUTPUT) < 0) {
        perror("failed to unlink output file");
    }

    // set up semaphore
    sem_t *file_writer_sem;
    file_writer_sem = sem_open(SEM_FILENAME, O_CREAT | O_EXCL, 0644, 1);
    if (file_writer_sem == SEM_FAILED) {
        perror("failed to create a semaphore");
        exit(EXIT_FAILURE);
    }

    // set up shared memory
    int shm_fd;
    shm_fd = shm_open(SHM_FILENAME, O_CREAT | O_RDWR | O_EXCL, 0644);
    if (shm_fd < 0) {
        perror("failed to create a shared memory object");
        exit(EXIT_FAILURE);
    }

    ftruncate(shm_fd, sizeof(unsigned long int));

    prime_counter = mmap(NULL, sizeof(unsigned long int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (prime_counter == MAP_FAILED) {
        perror("failed to create mmap-backed counter");
        exit(EXIT_FAILURE);
    }

    close(shm_fd);

    *prime_counter = 0;

    // get maximum number limit
    unsigned long long int max_num = strtoll(argv[2], NULL, 10);

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

                int counter = 0;
                unsigned long long int boundary = (unsigned long long int)sqrtl(init_num);
                
                for (unsigned long long int c = 2; c <= boundary; ++c) {
                    if (init_num % c == 0) {
                        ++counter;
                    }

                    if (counter > 0) {
                        break;
                    }
                }

                // a prime number is found
                if (counter == 0) {
                    time_t stop_time;
                    time_t duration;
                    stop_time = time(NULL);
                    duration = stop_time - start_time;

                    // counter increment
                    int sem_value;

                    // get current semaphore value
                    sem_getvalue(file_writer_sem, &sem_value);

                    if (sem_value == 0) {
                        printf("child index [%d] - concurrent writers detected: semaphore value: %d\n", i, sem_value);
                    }

                    sem_wait(file_writer_sem);
                    ++(*prime_counter);
                    sem_post(file_writer_sem);

                    // write output
                    FILE *file;
                    file = NULL;

                    file = fopen(OUTPUT, "a");

                    if (file == NULL) {
                        perror("failed to open result file");
                    } else {
                        fprintf(file, "child index [%d]: %llu %ld\n", i, init_num, duration);
                        fflush(file);
                        fsync(fileno(file));
                        fclose(file);
                    }
                }

                ++init_num;
            }

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

    printf("prime counter: %lu\n", *prime_counter);

    // unmap shared memory region
    munmap(prime_counter, sizeof(unsigned long int));

    // close and remove semaphore
    sem_close(file_writer_sem);
    sem_unlink(SEM_FILENAME);

    // remove shared memory object
    shm_unlink(SHM_FILENAME);

    exit(0);
}
