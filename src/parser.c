#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

// Comparateur pour qsort (tri par arrival_time)
static int compare_arrival(const void *a, const void *b) {
    Process *pa = (Process *)a;
    Process *pb = (Process *)b;
    return pa->arrival_time - pb->arrival_time;
}

Process* read_processes_from_file(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur d'ouverture");
        return NULL;
    }
    
    // Première passe : compter les processus
    int lines = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] != '#' && strlen(buffer) > 1) lines++;
    }
    
    *count = lines;
    if (lines == 0) {
        fclose(file);
        return NULL;
    }
    
    Process *processes = malloc(lines * sizeof(Process));
    if (!processes) {
        fclose(file);
        return NULL;
    }
    
    // Seconde passe : lire les données
    rewind(file);
    int index = 0;
    while (fgets(buffer, sizeof(buffer), file) && index < lines) {
        if (buffer[0] == '#' || strlen(buffer) <= 1) continue;
        
        // Format : PID arrival cpu1 io1 cpu2 io2 ... cpuN
        int values[100];
        int val_count = 0;
        char *token = strtok(buffer, " \t\n");
        while (token && val_count < 100) {
            values[val_count++] = atoi(token);
            token = strtok(NULL, " \t\n");
        }
        
        if (val_count < 3) continue; // au moins PID, arrival, un CPU burst
        
        Process *p = &processes[index];
        p->pid = values[0];
        p->arrival_time = values[1];
        p->num_bursts = (val_count - 2 + 1) / 2; // car cpu1 io1 cpu2 io2 ... cpuN (nombre de cpu = (n+1)/2)
        // Mais pour simplifier, on va considérer que le fichier alterne CPU / IO,
        // et le dernier est un CPU sans IO après.
        // On alloue les tableaux.
        p->cpu_bursts = malloc(p->num_bursts * sizeof(int));
        p->io_bursts = malloc((p->num_bursts - 1) * sizeof(int));
        
        int idx = 2; // après PID et arrival
        for (int i = 0; i < p->num_bursts; i++) {
            p->cpu_bursts[i] = values[idx++];
            if (i < p->num_bursts - 1) {
                p->io_bursts[i] = values[idx++];
            }
        }
        
        // Initialisation des autres champs
        p->current_burst_index = 0;
        p->remaining_burst = p->cpu_bursts[0];
        p->total_wait_time = 0;
        p->response_time = -1; // pas encore démarré
        p->finish_time = 0;
        p->state = NEW;
        
        index++;
    }
    
    fclose(file);
    
    // Trier les processus par arrival_time pour faciliter la simulation
    qsort(processes, *count, sizeof(Process), compare_arrival);
    
    return processes;
}