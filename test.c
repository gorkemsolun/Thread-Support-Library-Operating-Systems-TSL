#include <stdio.h>
#define __USE_GNU
#include <ucontext.h>
#include <stdlib.h>

#define STACK_SIZE 4096

void fun() {
    printf("fun\n");
}

int main() {
    ucontext_t uc_main, uc_fun;
    void *fun_stack, *main_stack;

    // Allocate stack for main
    main_stack = malloc(STACK_SIZE);
    if (main_stack == NULL) {
        perror("malloc");
        exit(1);
    }

    // Set up main context
    getcontext(&uc_main);
    uc_main.uc_link = NULL;
    uc_main.uc_stack.ss_sp = main_stack;
    uc_main.uc_stack.ss_size = STACK_SIZE;

    // Allocate stack for fun
    fun_stack = malloc(STACK_SIZE);
    if (fun_stack == NULL) {
        perror("malloc");
        exit(1);
    }

    // Set up fun context
    getcontext(&uc_fun);
    uc_fun.uc_link = &uc_main;
    uc_fun.uc_stack.ss_sp = fun_stack;
    uc_fun.uc_stack.ss_size = STACK_SIZE;
    uc_fun.uc_mcontext.gregs[REG_EIP] = (unsigned int)fun;

    printf("main\n");

    // Switch to fun context
    setcontext(&uc_fun);
    //swapcontext(&uc_main, &uc_fun);
    printf("back to main\n");

    // Free allocated stacks
    free(main_stack);
    free(fun_stack);

    return 0;
}
