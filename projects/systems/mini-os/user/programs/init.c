#include "types.h"
#include "user.h"

// init: The initial user-level program

char *argv[] = { "sh", 0 };

int main(void) {
    int pid, wpid;

    printf("init: starting\n");

    // Start shell
    if (fork() == 0) {
        exec("sh", argv);
        printf("init: exec sh failed\n");
        exit();
    }

    // Wait for children
    while (1) {
        wpid = wait();
        if (wpid < 0) {
            printf("init: no children\n");
            break;
        }
        printf("init: process %d exited\n", wpid);
    }

    printf("init: exiting\n");
    exit();
}
