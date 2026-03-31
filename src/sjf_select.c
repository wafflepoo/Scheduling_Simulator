#include "scheduler.h"

int sjf_select(Process **ready, int n, int quantum, int current_time) {
    (void)quantum; (void)current_time;
    int idx = 0;
    int min_burst = ready[0]->cpu_bursts[ready[0]->current_burst_index];
    for (int i = 1; i < n; i++) {
        int b = ready[i]->cpu_bursts[ready[i]->current_burst_index];
        if (b < min_burst) {
            min_burst = b;
            idx = i;
        }
    }
    return idx;
}