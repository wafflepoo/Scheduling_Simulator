#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>

// Structure pour un processus
typedef struct {
    int pid;                    // Identifiant
    int arrival_time;           // Date d'arrivée
    int *cpu_bursts;            // Tableau des cycles CPU
    int *io_bursts;             // Tableau des cycles E/S
    int num_bursts;             // Nombre total de bursts
    int current_burst_index;    // Pour suivre où il en est
    
    // Pour les statistiques
    int start_time;             // Début de l'exécution (pour réponse)
    int finish_time;            // Fin du processus
    int total_wait_time;        // Temps d'attente cumulé
    int response_time;          // Premier démarrage - arrivée
    
    // Pour l'ordonnancement
    int remaining_time;         // Temps restant (pour SJRF)
    int last_exec_time;         // Dernier moment exécuté (pour RR)
} Process;

// Structure pour les résultats
typedef struct {
    float avg_wait_time;
    float avg_turnaround_time;
    float avg_response_time;
    float cpu_utilization;
    int *wait_times;            // Par processus
    int *turnaround_times;      // Par processus
    int *response_times;        // Par processus
} ScheduleResult;

// Structure pour l'ordonnanceur (pattern strategy)
typedef struct {
    const char *name;
    void (*schedule)(Process *processes, int count, ScheduleResult *result);
} Scheduler;

// Déclarations des algorithmes
void fifo_schedule(Process *processes, int count, ScheduleResult *result);
void sjf_schedule(Process *processes, int count, ScheduleResult *result);
void sjrf_schedule(Process *processes, int count, ScheduleResult *result);
void rr_schedule(Process *processes, int count, ScheduleResult *result);

// Fonctions utilitaires
Process* read_processes_from_file(const char *filename, int *count);
void export_to_csv(const char *filename, ScheduleResult *result, Process *processes, int count);
void free_processes(Process *processes, int count);

#endif