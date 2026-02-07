// Integration test: Multi-process execution

#include "types.h"
#include "user.h"

#define NUM_PROCS 5

int main(void) {
    int i, pid;
    int pids[NUM_PROCS];

    printf("test_multiproc: starting\n");

    // Create multiple child processes
    for (i = 0; i < NUM_PROCS; i++) {
        pid = fork();
        if (pid < 0) {
            printf("test_multiproc: fork failed\n");
            exit();
        }
        if (pid == 0) {
            // Child process
            printf("test_multiproc: child %d (pid %d) running\n", i, getpid());
            // Do some work
            for (int j = 0; j < 1000; j++) {
                // Busy work
            }
            printf("test_multiproc: child %d (pid %d) exiting\n", i, getpid());
            exit();
        }
        pids[i] = pid;
    }

    // Wait for all children
    for (i = 0; i < NUM_PROCS; i++) {
        int wpid = wait();
        printf("test_multiproc: child %d exited\n", wpid);
    }

    printf("test_multiproc: all children exited\n");
    printf("test_multiproc: TEST PASSED\n");
    exit();
}
