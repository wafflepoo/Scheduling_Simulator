#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

void fifo_schedule(Process *processes, int count, ScheduleResult *result) {
    int current_time = 0;
    int completed = 0;
    int total_wait = 0;
    int total_turnaround = 0;
    int total_response = 0;
    
    // Trier les processus par arrival_time (déjà fait normalement)
    // FIFO = First In First Out = ordre d'arrivée
    
    printf("\n=== Simulation FIFO ===\n");
    
    for (int i = 0; i < count; i++) {
        Process *p = &processes[i];
        
        // Si le processeur est inactif jusqu'à l'arrivée du processus
        if (current_time < p->arrival_time) {
            current_time = p->arrival_time;
        }
        
        // Temps de réponse = début - arrivée
        int response = current_time - p->arrival_time;
        p->response_time = response;
        
        printf("[T=%d] Début P%d (arrivé à %d, réponse=%d)\n", 
               current_time, p->pid, p->arrival_time, response);
        
        // Simuler l'exécution de tous les bursts du processus
        int process_time = 0;
        for (int j = 0; j < p->num_bursts; j++) {
            process_time += p->cpu_bursts[j];
            // On ignore les I/O pour l'instant (juste le CPU)
        }
        
        current_time += process_time;
        p->finish_time = current_time;
        
        // Turnaround = fin - arrivée
        int turnaround = p->finish_time - p->arrival_time;
        p->total_wait_time = turnaround - process_time; // Attente = turnaround - temps CPU
        
        printf("[T=%d] Fin P%d (turnaround=%d, attente=%d)\n", 
               current_time, p->pid, turnaround, p->total_wait_time);
        
        total_wait += p->total_wait_time;
        total_turnaround += turnaround;
        total_response += response;
        completed++;
    }
    
    // Calculer les moyennes
    if (completed > 0) {
        result->avg_wait_time = (float)total_wait / completed;
        result->avg_turnaround_time = (float)total_turnaround / completed;
        result->avg_response_time = (float)total_response / completed;
        
        // Taux d'occupation CPU = temps CPU total / temps total
        int total_cpu = 0;
        for (int i = 0; i < count; i++) {
            for (int j = 0; j < processes[i].num_bursts; j++) {
                total_cpu += processes[i].cpu_bursts[j];
            }
        }
        result->cpu_utilization = (float)total_cpu / current_time * 100;
    }
    
    // Afficher les résultats
    printf("\n=== Résultats FIFO ===\n");
    printf("Temps d'attente moyen: %.2f\n", result->avg_wait_time);
    printf("Turnaround moyen: %.2f\n", result->avg_turnaround_time);
    printf("Temps de réponse moyen: %.2f\n", result->avg_response_time);
    printf("Taux d'occupation CPU: %.2f%%\n", result->cpu_utilization);
}