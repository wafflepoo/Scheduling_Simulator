#include "scheduler.h"

int sjrf_select(Process **ready, int n, int quantum, int current_time) {
    (void)quantum; (void)current_time;
    int idx = 0;
    int min_rem = ready[0]->remaining_burst;
    for (int i = 1; i < n; i++) {
        if (ready[i]->remaining_burst < min_rem) {
            min_rem = ready[i]->remaining_burst;
            idx = i;
        }
    }
    return idx;
}