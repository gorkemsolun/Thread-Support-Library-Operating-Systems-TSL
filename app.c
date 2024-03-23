#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "tsl.h"

// This is a sample application that is using tsl library to 
// create and work with threads. 

#define MAXCOUNT 15
#define YIELDPERIOD 5

int tids[TSL_MAXTHREADS];
int numberOfThreads = 0;
int alg = ALG_FCFS;

void* foo(void* v) {
    int mytid = tsl_gettid();
    printf("Thread %d started running.\n", mytid);

    int count = 1;
    while (count != MAXCOUNT) {
        printf("Thread %d is running (count = %d).\n", mytid, count);
        if (count % YIELDPERIOD == 0) {
            printf("Thread %d is yielding.\n", mytid);
            tsl_yield(TSL_ANY);
        }

        count++;
    }
    return (NULL);
}


int main(int argc, char** argv) {
    /* argv[1] = "5";
    argv[2] = "2";
    argc = 3; */

    if (argc != 3) {
        printf("Usage: ./app [number of threads] [schedule algorithm, 1 for FCFS and 2 for RANDOM]\n");
        exit(1);
    }

    numberOfThreads = atoi(argv[1]);
    alg = atoi(argv[2]);

    if (alg != ALG_FCFS && alg != ALG_RANDOM) {
        printf("Invalid scheduling algorithm. Use 1 for FCFS and 2 for RANDOM\n");
        exit(1);
    }

    // tid[0] is the id of the main thread
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

    return 0;
}
