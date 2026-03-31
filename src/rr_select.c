#include "scheduler.h"

int rr_select(Process **ready, int n, int quantum, int current_time) {
    (void)quantum; (void)current_time;
    return 0; // Round Robin utilise une file FIFO (le prochain est toujours le premier)
}