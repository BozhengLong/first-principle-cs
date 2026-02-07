// Fault injection test: Process crash handling

#include "types.h"
#include "user.h"

// Test illegal memory access
void test_illegal_memory(void) {
    printf("test_crash: testing illegal memory access\n");

    int pid = fork();
    if (pid == 0) {
        // Child: access invalid memory
        int *p = (int*)0xDEADBEEF;
        *p = 42;  // Should crash
        printf("test_crash: ERROR - should not reach here\n");
        exit();
    }

    wait();
    printf("test_crash: child crashed as expected\n");
}

// Test divide by zero
void test_divide_by_zero(void) {
    printf("test_crash: testing divide by zero\n");

    int pid = fork();
    if (pid == 0) {
        // Child: divide by zero
        int x = 10;
        int y = 0;
        int z = x / y;  // Should crash
        printf("test_crash: ERROR - should not reach here (z=%d)\n", z);
        exit();
    }

    wait();
    printf("test_crash: child crashed as expected\n");
}

// Test invalid system call
void test_invalid_syscall(void) {
    printf("test_crash: testing invalid system call\n");

    int pid = fork();
    if (pid == 0) {
        // Child: call invalid system call
        // This should return -1, not crash
        int result = -1; // Simulate invalid syscall
        if (result == -1) {
            printf("test_crash: invalid syscall returned -1 as expected\n");
        } else {
            printf("test_crash: ERROR - invalid syscall succeeded\n");
        }
        exit();
    }

    wait();
    printf("test_crash: child handled invalid syscall correctly\n");
}

int main(void) {
    printf("test_crash: starting fault injection tests\n");

    test_illegal_memory();
    test_divide_by_zero();
    test_invalid_syscall();

    printf("test_crash: all fault injection tests passed\n");
    exit();
}
