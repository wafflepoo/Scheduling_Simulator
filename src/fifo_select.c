#include "scheduler.h"

int fifo_select(Process **ready, int n, int quantum, int current_time) {
    (void)quantum; (void)current_time; // inutilisés
    return 0; // premier de la file
}