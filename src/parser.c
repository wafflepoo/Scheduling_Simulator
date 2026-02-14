#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

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
    Process *processes = malloc(lines * sizeof(Process));
    
    // Seconde passe : lire les données
    rewind(file);
    int index = 0;
    while (fgets(buffer, sizeof(buffer), file) && index < lines) {
        if (buffer[0] == '#' || strlen(buffer) <= 1) continue;
        
        // Format proposé : PID arrivee cpu1 io1 cpu2 io2 cpu3 ...
        int pid, arrivee;
        int bursts[100]; // Maximum 100 bursts
        int burst_count = 0;
        
        char *token = strtok(buffer, " \t\n");
        while (token && burst_count < 100) {
            bursts[burst_count++] = atoi(token);
            token = strtok(NULL, " \t\n");
        }
        
        if (burst_count >= 3) {
            pid = bursts[0];
            arrivee = bursts[1];
            
            processes[index].pid = pid;
            processes[index].arrival_time = arrivee;
            processes[index].num_bursts = burst_count - 2;
            
            // Allouer les tableaux de bursts
            processes[index].cpu_bursts = malloc((processes[index].num_bursts + 1) * sizeof(int));
            processes[index].io_bursts = malloc(processes[index].num_bursts * sizeof(int));
            
            for (int i = 0; i < processes[index].num_bursts; i++) {
                processes[index].cpu_bursts[i] = bursts[i + 2];
                if (i < processes[index].num_bursts - 1) {
                    processes[index].io_bursts[i] = bursts[i + 3];
                }
            }
            
            // Initialiser les autres champs
            processes[index].current_burst_index = 0;
            processes[index].remaining_time = processes[index].cpu_bursts[0];
            processes[index].total_wait_time = 0;
            
            index++;
        }
    }
    
    fclose(file);
    return processes;
}