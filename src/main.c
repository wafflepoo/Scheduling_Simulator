#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

// Fonction d’export CSV (à compléter selon vos besoins)
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
    fprintf(file, "\nMoyennes,%.2f,%.2f,%.2f\n",
            result->avg_response_time,
            result->avg_wait_time,
            result->avg_turnaround_time);
    fprintf(file, "CPU Utilization,%%.2f\n", result->cpu_utilization);
    fclose(file);
    printf("Résultats exportés dans %s\n", filename);
}

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
    
    // Choix de l’algorithme
    char *algo = "fifo";
    if (argc >= 3) algo = argv[2];
    
    ScheduleResult result = {0};
    
    if (strcmp(algo, "fifo") == 0) {
        fifo_schedule(processes, process_count, &result);
    } else if (strcmp(algo, "sjf") == 0) {
    sjf_schedule(processes, process_count, &result);
        free_processes(processes, process_count);
        return 1;
    } else if (strcmp(algo, "sjrf") == 0) {
    sjrf_schedule(processes, process_count, &result);
        free_processes(processes, process_count);
        return 1;
    } else if (strcmp(algo, "rr") == 0) {
        rr_schedule(processes, process_count, &result);
        free_processes(processes, process_count);
        return 1;
    } else {
        fprintf(stderr, "Algorithme inconnu: %s\n", algo);
        free_processes(processes, process_count);
        return 1;
    }
    
    // Export CSV si un troisième argument est fourni
    if (argc >= 4) {
        export_to_csv(argv[3], &result, processes, process_count);
    }
    
    free_processes(processes, process_count);
    return 0;
}

// Définition de free_processes (si pas déjà dans un autre fichier)
void free_processes(Process *processes, int count) {
    for (int i = 0; i < count; i++) {
        free(processes[i].cpu_bursts);
        free(processes[i].io_bursts);
    }
    free(processes);
}