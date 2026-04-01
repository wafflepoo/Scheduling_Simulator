#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

void free_processes(Process *processes, int count) {
    for (int i = 0; i < count; i++) {
        free(processes[i].cpu_bursts);
        free(processes[i].io_bursts);
    }
    free(processes);
}

void export_to_csv(const char *filename, ScheduleResult *result, Process *processes, int count) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erreur création fichier CSV");
        return;
    }
    fprintf(file, "PID,Response Time,Wait Time,Turnaround Time\n");
    for (int i = 0; i < count; i++) {
        Process *p = &processes[i];
        fprintf(file, "%d,%d,%d,%d\n",
                p->pid,
                p->response_time,
                p->total_wait_time,
                p->finish_time - p->arrival_time);
    }
    // Lignes commentées
    fprintf(file, "#Moyennes,%.2f,%.2f,%.2f\n",
            result->avg_response_time,
            result->avg_wait_time,
            result->avg_turnaround_time);
    fprintf(file, "#CPU Utilization,%.2f\n", result->cpu_utilization);
    fclose(file);
    printf("Résultats exportés dans %s\n", filename);
}