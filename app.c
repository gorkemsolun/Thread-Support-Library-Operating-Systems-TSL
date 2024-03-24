#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "tsl.h"

// This is a sample application that is using tsl library to 
// create and work with threads. 

int tids[TSL_MAXTHREADS];
int numberOfThreads = 0;
int alg = ALG_FCFS;
int maxcount = 0;
int yieldperiod = 0;
int exitperiod = 0;
int cancelperiod = 0;

void* foo(void* v) {
    int mytid = tsl_gettid();
    printf("Thread %d started running.\n", mytid);

    int count = 1;
    while (count != maxcount) {
        printf("Thread %d is running (count = %d).\n", mytid, count);
        srand(time(NULL) ^ getpid());
        int random_number = rand();
        int tidIndex = random_number % numberOfThreads;

        if (count % yieldperiod == 0) {
            // if odd, pick a random tid to yield, this thread may be terminated so in this case it does not yield
            if (random_number & 1) {
                printf("Thread %d is yielding to thread %d.\n", mytid, tids[tidIndex]);
                int res = tsl_yield(tids[tidIndex]);
                if (res == -1) {
                    printf("Thread %d cannot be yielded.\n", tids[tidIndex]);
                }
            } else { // if even, yielding will be done by selecting any from the queue
                printf("Thread %d is yielding.\n", mytid);
                tsl_yield(TSL_ANY);
            }
        }

        if (count % exitperiod == 0) {
            printf("Thread %d is exiting.\n", mytid);
            tsl_exit();
        }

        // cancel a random thread, if it is not terminated
        if (count % cancelperiod == 0) {
            printf("Thread %d is cancelling thread %d.\n", mytid, tids[tidIndex]);
            int res = tsl_cancel(tids[tidIndex]);
            if (res == -1) {
                printf("Thread %d cannot be cancelled.\n", tids[tidIndex]);
            }
        }

        count++;
    }
    return (NULL);
}


int main(int argc, char** argv) {
    /* // for testing 
    argv[1] = "5"; // number of threads
    argv[2] = "2"; // scheduling algorithm
    argc = 7; // number of arguments
    argv[3] = "14"; // max count
    argv[4] = "500"; // yield period
    argv[5] = "100"; // exit period 
    argv[6] = "10"; // cancel period */

    if (argc != 7) {
        printf("Usage: ./app [number of threads] [schedule algorithm, 1 for FCFS and 2 for RANDOM] ");
        printf("[Maxiumum count to count] [yield period of counting] [exit period of counting] [cancel period of counting, for cancelling a random thread]\n");
        exit(1);
    }

    clock_t start, end;
    double cpu_time_used;
    start = clock();

    numberOfThreads = atoi(argv[1]);
    alg = atoi(argv[2]);
    maxcount = atoi(argv[3]);
    yieldperiod = atoi(argv[4]);
    exitperiod = atoi(argv[5]);
    cancelperiod = atoi(argv[6]);

    if (alg != ALG_FCFS && alg != ALG_RANDOM) {
        printf("Invalid scheduling algorithm. Use 1 for FCFS and 2 for RANDOM\n");
        exit(1);
    }

    // The id of the main thread
    tids[0] = tsl_init(alg);

    for (int i = 1; i < numberOfThreads; ++i) {
        tids[i] = tsl_create_thread((void*)&foo, NULL);
        printf("Thead %d created\n", (int)tids[i]);
    }

    for (int i = 1; i < numberOfThreads; ++i) {
        printf("Main: Waiting for thead %d\n", (int)tids[i]);
        tsl_join(tids[i]);
        printf("Main: Thead %d finished\n", (int)tids[i]);

    }

    printf("Main thread calling tlib_exit\n");
    tsl_exit();

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time elapsed is %f seconds\n", cpu_time_used);

    return 0;
}
