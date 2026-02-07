// Benchmark: Context switch overhead

#include "types.h"
#include "user.h"

#define NUM_SWITCHES 10000

int main(void) {
    int i;
    int start_time, end_time;

    printf("bench_context_switch: starting\n");

    // Get start time
    start_time = uptime();

    // Perform many context switches
    for (i = 0; i < NUM_SWITCHES; i++) {
        yield();  // Force context switch
    }

    // Get end time
    end_time = uptime();

    int total_time = end_time - start_time;
    int avg_time_ns = (total_time * 1000000) / NUM_SWITCHES;  // Convert to nanoseconds

    printf("bench_context_switch: %d context switches\n", NUM_SWITCHES);
    printf("bench_context_switch: total time: %d ticks\n", total_time);
    printf("bench_context_switch: average time per switch: %d ns\n", avg_time_ns);

    exit();
}
