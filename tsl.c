#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <assert.h>

/* We want the extra information from these definitions */
#ifndef __USE_GNU
#define __USE_GNU
#endif /* __USE_GNU */

#include <ucontext.h>
#include "tsl.h"

// you will implement your library in this file. 
// you can define your internal structures, macros here. 
// such as: #define ...
// if you wish you can use another header file (not necessary). 
// but you should not change the tsl.h header file. 
// you can also define and use global variables here.
// you can define and use additional functions (as many as you wish) 
// in this file; besides the tsl library functions desribed in the assignment. 
// these additional functions will not be available for applications directly. 

// TCB structure
typedef struct TCB {
    int tid; // thread identifier
    unsigned int state; // thread state
    ucontext_t context; // pointer to context structure

    /*
        typedef struct ucontext_t
        {
        unsigned long int __ctx(uc_flags);
        struct ucontext_t *uc_link;
        stack_t uc_stack;
        mcontext_t uc_mcontext;
        sigset_t uc_sigmask;
        struct _libc_fpstate __fpregs_mem;
        } ucontext_t;
    */

    char* stack; // pointer to stack
} TCB;

// Queue implementation
typedef struct QueueNode {
    TCB* tcb; // pointer to TCB
    struct QueueNode* next; // pointer to next node
} QueueNode;

typedef struct tcbQueue {
    QueueNode* front; // pointer to front of the queue
    QueueNode* rear; // pointer to rear of the queue
} tcbQueue;

tcbQueue* createTCBQueue() {
    tcbQueue* queue = (tcbQueue*)malloc(sizeof(tcbQueue));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

// Create a new ready queue
void enqueue(tcbQueue* queue, TCB* tcb) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->tcb = tcb;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

// Dequeue from the queue
TCB* dequeue(tcbQueue* queue) {
    if (queue->front == NULL) {
        return NULL;
    }

    QueueNode* temp = queue->front;
    TCB* tcb = temp->tcb;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(temp);
    return tcb;
}

// Check if the queue is empty
int isReadyQueueEmpty(tcbQueue* queue) {
    return queue->front == NULL;
}

// Queue implementation end

// Define states
#define TSL_RUNNING 1
#define TSL_READY 2

// Global variables

unsigned int readyQueueSize = 0;
unsigned int schedulingAlg; // 1 is FCFS, 2 is random
int currentThreadCount = 0; // Current number of threads
tcbQueue* readyQueue; // ready queue
tcbQueue* runningQueue; // running queue, we could have used a global running TCB as well, there is only one running thread at a time
tcbQueue* exitedQueue; // when a process calls exit then it will be moved to this queue
int next_tid = 1; // next thread id
int tsl_init(int salg);
int tsl_create_thread(void (*tsf)(void*), void* targ);
int tsl_yield(int tid);
int tsl_exit();
int tsl_join(int tid);
int tsl_cancel(int tid);
int tsl_gettid();

// This is the main function of the library and library will be initialized here
// This function will be called exatcly once
// salg is the scheduling algorithm to be used
int tsl_init(int salg) {

    schedulingAlg = salg; //set the scheduling algorithm

    readyQueue = createTCBQueue();
    runningQueue = createTCBQueue();
    exitedQueue = createTCBQueue();

    // Add current(main) thread's TCB to running queue
    TCB* mainTCB = (TCB*)malloc(sizeof(TCB));
    mainTCB->tid = next_tid++; // Assuming main thread has tid 1
    mainTCB->state = TSL_RUNNING; // Set state to running
    mainTCB->stack = NULL; // No stack for main thread
    enqueue(runningQueue, mainTCB);

    int success = 1;// TODO: Success should be handled
    if (success) {
        return 0;
    } else {
        return TSL_ERROR;
    }

}

// creates a new thread
// tsf is the function(the thread start function or root function) to be executed by the new thread
// The application will define this thread start function, and its address will be the first argument
// Function can take one argument of type void*
// targ is a pointer to a value or structure that will be passed to the thread start function, if nothing is to be passed, it will be NULL
// The function will return the thread id(tid) of the new thread, unique among all threads in the application
int tsl_create_thread(void (*tsf)(void*), void* targ) {
    if (currentThreadCount >= TSL_MAXTHREADS) {
        return TSL_ERROR; // Maximum number of threads reached
    }

    // Create a new TCB for the new thread
    TCB* newTCB = (TCB*)malloc(sizeof(TCB));
    newTCB->tid = next_tid++;
    newTCB->state = TSL_READY;
    newTCB->stack = malloc(TSL_STACKSIZE); // TODO: this should be checked // Allocate memory for the stack

    enqueue(readyQueue, newTCB); // Add the new thread to the ready queue
    readyQueueSize++;

    currentThreadCount++; // Increment the number of threads

    // Create a new context for the new thread
    getcontext(&newTCB->context);

    // TODO: this should be checked
    newTCB->context.uc_mcontext.gregs[REG_EIP] = (unsigned int)stub; // Set the instruction pointer to the stub function
    newTCB->context.uc_mcontext.gregs[REG_ESP] = (unsigned int)newTCB->stack; // Set the stack pointer to the top of the stack
    newTCB->stack += TSL_STACKSIZE; // Move the stack pointer to the bottom of the stack

    // TODO: this should be checked // Initialize the stack by putting tsl and targ into the stack
    unsigned int* stack_ptr = (unsigned int*)newTCB->context.uc_mcontext.gregs[REG_ESP];
    *stack_ptr-- = (unsigned int)targ; // Push targ onto the stack
    *stack_ptr-- = (unsigned int)tsf; // Push tsf onto the stack

    int success = 1; // TODO: Success should be handled
    if (success) {
        return newTCB->tid;
    } else {
        return TSL_ERROR;
    }
}

//Scheduling algorithms
void FCFS(TCB* thread) {
    thread = dequeue(readyQueue);
    if (readyQueueSize > 0) {
       readyQueueSize--; 
    }
    
}

void Random(TCB* thread) {
    srand(time(NULL));
    int randomIndex = rand() % readyQueueSize + 1;
    int index = 1;
    QueueNode* temp = readyQueue->front;
    QueueNode* prev = NULL;
    while (temp != NULL) {
        if (index == randomIndex) {
                    
            if (temp == readyQueue->front) {
                thread = dequeue(readyQueue);
            } else {
                thread = temp->tcb;
                prev->next = temp->next;
                temp->next = NULL;
            }

            break;
        }
        prev = temp;
        temp = temp->next;
        index++;
    }
    
    if (readyQueueSize > 0) {
       readyQueueSize--; 
    }
    
}

// TODO: STUB should be implemented
// TODO: STUB SHOULD BE PUT IN THE CORRECT PLACE
// stub wrapper function for the thread start function
void stub(void (*tsf) (void*), void* targ) {
    // new thread will start its execution here
    tsf(targ); // then we will call the thread start function

    // tsf will retun to here
    tsl_exit(); // now ask for termination
}

// yields the processor to another thread, a context switch will occur
// tid is the thread id of the thread to which the processor will be yielded
// if tid is TSL_ANY, the processor will be yielded to any thread selected by the scheduling algorithm from the threads that are ready to run
// selected threads context will be loaded and it will start to execute
// if there is no ready thread to run other than calling thread, scheduler will select the calling thread to run next
// save calling threads context then restore it later
// if tid > 0, but no ready thread with the given tid, the function will return TSL_ERROR
// this function will not return until the calling thread is scheduled to run again
int tsl_yield(int tid) {
    int next_tid;
    TCB* calleeThread = NULL;
    TCB* callerThread = runningQueue->front->tcb;
    
    callerThread->state = TSL_READY;
    enqueue(readyQueue, callerThread); // add the caller thread to ready queue
    readyQueueSize++;
    dequeue(runningQueue); //remove the caller thread
    
    if (tid == TSL_ANY) { // pick according to the schedule algorithm
        if (schedulingAlg == ALG_FCFS) {
            FCFS(calleeThread);
        } else if (schedulingAlg == ALG_RANDOM) { //pick a random element from the ready queue (this could result in starvation)
            Random(calleeThread);
        }
    } else { //pick the thread specified by tid
        QueueNode* temp = readyQueue->front;
        QueueNode* prev = NULL;
        while (temp != NULL) {
            if (temp->tcb->tid == tid) {
                if (temp == readyQueue->front) {
                        calleeThread = dequeue(readyQueue);
                        readyQueueSize--;
                    } else {
                        calleeThread = temp->tcb;
                        prev->next = temp->next;
                        temp->next = NULL;
                    }

                break;
            }
            prev = temp;
            temp = temp->next;
        }

        if (calleeThread != NULL) {
            readyQueueSize--;
        } else {
            /*If tid parameter is a positive integer but there is no ready thread
            with that tid, the function will return immediately without yielding to any
            thread.*/
            return -1;
        }
    }


    //now that the thread to be run is set, put it into the running queue and run it
    next_tid = calleeThread->tid;

    getcontext(&(callerThread->context)); //save the callers context

    if (callerThread->state == TSL_READY) {
        //it means the first return, yield to the callee
        calleeThread->state = TSL_RUNNING;
        enqueue(runningQueue, calleeThread); //insert the callee thread
        setcontext(&(calleeThread->context)); //switch context
    } else {
        // it means the second return, some other thread has yielded to this thread
        return next_tid;
    }
    
}

// terminates the calling thread
// TCB and stack of the calling thread will be kept until another thread calls tsl_join with the tid of the calling thread
// this function will not return
int tsl_exit() {
    //move from running to exited queue
    TCB* currentThread = dequeue(runningQueue);
    //currentThread->state = ENDED; might be redundant
    enqueue(exitedQueue, currentThread);
    
    //This enclosed section could be replaced by yield function
    // I wrote this because I thought that we do not need to save the context of the thread that is exiting
    // I might be wrong...
    //---------------------------------------------------------------------------
    TCB* calleeThread = NULL;
    if (schedulingAlg == ALG_FCFS) {
        FCFS(calleeThread);
    } else if (schedulingAlg == ALG_RANDOM) { //pick a random element from the ready queue (this could result in starvation)
        Random(calleeThread);
    }
    if (calleeThread == NULL) {
        // readyQueue is empty, free all that is left in the exited queue (if there are any left due to not being joined)
        TCB* temp;
        do {
            temp = dequeue(exitedQueue);
            free(temp->stack);
            free(temp);
        } while(temp != NULL);
        
    } else {
        calleeThread->state = TSL_RUNNING;
        enqueue(runningQueue, calleeThread);
        setcontext(&(calleeThread->context));
    }
    //---------------------------------------------------------------------------
    return (0);
}

// waits for the termination of another thread with the given tid, then returns
// before returning, the TCB and stack of the terminated thread will be deallocated
int tsl_join(int tid) {
    
    int result = tsl_yield(tid);

    if (result == -1) {
        //no such thread in the ready queue
        return -1;
    }

    //the thread with tid has exited, free its resources
    QueueNode* exited = exitedQueue->front;
    QueueNode* prev = NULL;
    while(exited != NULL) {
        if (exited->tcb->tid == tid) {
            if (exited == exitedQueue->front) {
                TCB* tobeDeleted = dequeue(exitedQueue);
                free(tobeDeleted->stack);
                free(tobeDeleted);
            } else {
                prev->next = exited ->next;
                free(exited->tcb->stack);
                free(exited->tcb);
                free(exited);
            }
        }
        prev = exited;
        exited = exited->next;
    }
    
    return tid;
}

// cancels another thread with the given tid asynchronously, the target thread will be immediately terminated
int tsl_cancel(int tid) {
    int success = 1;
    if (success) {
        return tid;
    } else {
        return TSL_ERROR; // TODO: no thread with the given tid
    }
}

// returns the thread id of the calling thread
int tsl_gettid() {
    //Assumption: There is only one thread running at a time
    return runningQueue->front->tcb->tid; // TODO: return the thread id of the calling thread
}