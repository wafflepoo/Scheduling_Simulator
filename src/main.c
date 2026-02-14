#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier_processus> [algorithme]\n", argv[0]);
        fprintf(stderr, "Algorithmes: fifo, sjf, sjrf, rr\n");
        return 1;
    }
    
    int process_count;
    Process *processes = read_processes_from_file(argv[1], &process_count);
    
    if (!processes) {
        fprintf(stderr, "Erreur de lecture du fichier\n");
        return 1;
    }
    
    printf("Lecture réussie : %d processus chargés\n", process_count);
    for (int i = 0; i < process_count; i++) {
        printf("PID %d: arrivee=%d, bursts=", processes[i].pid, processes[i].arrival_time);
        for (int j = 0; j < processes[i].num_bursts; j++) {
            printf("%d ", processes[i].cpu_bursts[j]);
            if (j < processes[i].num_bursts - 1) printf("(IO:%d) ", processes[i].io_bursts[j]);
        }
        printf("\n");
    }
    
    // Choisir l'algorithme (par défaut FIFO)
    char *algo = "fifo";
    if (argc >= 3) {
        algo = argv[2];
    }
    
    ScheduleResult result;
    
    if (strcmp(algo, "fifo") == 0) {
        fifo_schedule(processes, process_count, &result);
    } else {
        fprintf(stderr, "Algorithme non implémenté: %s\n", algo);
    }
    
    free_processes(processes, process_count);
    return 0;
}

void free_processes(Process *processes, int count) {
    for (int i = 0; i < count; i++) {
        free(processes[i].cpu_bursts);
        free(processes[i].io_bursts);
    }
    free(processes);
}