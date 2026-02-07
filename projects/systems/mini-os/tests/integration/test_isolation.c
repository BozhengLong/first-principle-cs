// Integration test: Process isolation

#include "types.h"
#include "user.h"

int global_var = 100;

int main(void) {
    int pid;
    int local_var = 200;

    printf("test_isolation: starting\n");
    printf("test_isolation: parent: global_var = %d, local_var = %d\n",
           global_var, local_var);

    pid = fork();
    if (pid < 0) {
        printf("test_isolation: fork failed\n");
        exit();
    }

    if (pid == 0) {
        // Child process - modify variables
        global_var = 999;
        local_var = 888;
        printf("test_isolation: child: modified global_var = %d, local_var = %d\n",
               global_var, local_var);
        exit();
    } else {
        // Parent process - wait and check variables
        wait();
        printf("test_isolation: parent: after child exit: global_var = %d, local_var = %d\n",
               global_var, local_var);

        // Verify isolation
        if (global_var == 100 && local_var == 200) {
            printf("test_isolation: TEST PASSED - variables unchanged in parent\n");
        } else {
            printf("test_isolation: TEST FAILED - variables modified in parent\n");
        }
    }

    exit();
}
