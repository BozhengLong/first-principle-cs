#include "types.h"
#include "user.h"

// fork_test: Test fork and process isolation

int main(void) {
    int x = 1;
    int pid;

    printf("fork_test: starting\n");
    printf("fork_test: x = %d\n", x);

    pid = fork();
    if (pid < 0) {
        printf("fork_test: fork failed\n");
        exit();
    }

    if (pid == 0) {
        // Child process
        x = 2;
        printf("fork_test: child: x = %d (should be 2)\n", x);
        printf("fork_test: child: pid = %d\n", getpid());
        exit();
    } else {
        // Parent process
        wait();
        printf("fork_test: parent: x = %d (should be 1)\n", x);
        printf("fork_test: parent: pid = %d\n", getpid());
        printf("fork_test: parent: child pid was %d\n", pid);
    }

    printf("fork_test: test passed\n");
    exit();
}
