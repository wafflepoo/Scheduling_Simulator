#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

// Déclaration externe des politiques (définies ailleurs)
extern SchedPolicy FIFO_POLICY;
extern SchedPolicy SJF_POLICY;
extern SchedPolicy SRJF_POLICY;
extern SchedPolicy RR_POLICY;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier_processus> [algorithme] [fichier_csv]\n", argv[0]);
        fprintf(stderr, "Algorithmes: fifo, sjf, sjrf, rr (défaut: fifo)\n");
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
            if (j < processes[i].num_bursts - 1)
                printf("(IO:%d) ", processes[i].io_bursts[j]);
        }
        printf("\n");
    }

    char *algo = (argc >= 3) ? argv[2] : "fifo";
    SchedPolicy *policy = NULL;

    if (strcmp(algo, "fifo") == 0) policy = &FIFO_POLICY;
    else if (strcmp(algo, "sjf") == 0) policy = &SJF_POLICY;
    else if (strcmp(algo, "sjrf") == 0) policy = &SRJF_POLICY;
    else if (strcmp(algo, "rr") == 0) policy = &RR_POLICY;
    else {
        fprintf(stderr, "Algorithme inconnu: %s\n", algo);
        free_processes(processes, process_count);
        return 1;
    }

    ScheduleResult result = {0};
    simulate(processes, process_count, policy, &result);

    if (argc >= 4)
        export_to_csv(argv[3], &result, processes, process_count);

    free_processes(processes, process_count);
    return 0;
}