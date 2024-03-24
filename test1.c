#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "tsl.h"

int tids[TSL_MAXTHREADS];
int numberOfThreads = 10;
int alg = ALG_FCFS;

void* foo(void* v) {
    int mytid = tsl_gettid();
    printf("Thread %d started running.\n", mytid);
    while (*((int*)v) >= 0)
    {
        printf("in foo %d\n", *((int*)v));
        *((int*)v) = *((int*)v) - 1;
    }
    *((int*)v) = 4;
}
    
int main(int argc, char const *argv[])
{
    // The id of the main thread
    tids[0] = tsl_init(alg);
    int a = 4;
    for (int i = 1; i < numberOfThreads; ++i) {
        tids[i] = tsl_create_thread((void*)&foo, &a);
    }

    for (int i = 1; i < numberOfThreads; ++i) {
        tsl_join(tids[i]);
    }
    tsl_exit();

    return 0;
}
