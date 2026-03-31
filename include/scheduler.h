#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <limits.h>

// États possibles d’un processus
typedef enum { NEW, READY, RUNNING, WAITING, TERMINATED } ProcessState;

// Structure pour un processus (inchangée)
typedef struct {
    int pid;
    int arrival_time;
    int *cpu_bursts;
    int *io_bursts;
    int num_bursts;
    int current_burst_index;
    int start_time;
    int finish_time;
    int total_wait_time;
    int response_time;
    int remaining_burst;
    int last_exec_time;
    ProcessState state;
} Process;

// Structure pour les résultats (inchangée)
typedef struct {
    float avg_wait_time;
    float avg_turnaround_time;
    float avg_response_time;
    float cpu_utilization;
    int *wait_times;
    int *turnaround_times;
    int *response_times;
} ScheduleResult;

// Politique d'ordonnancement (fonctions de sélection)
typedef struct {
    const char *name;
    int (*select_next)(Process **ready_queue, int ready_count, int quantum, int current_time);
    int quantum;   // pour RR, sinon 0
} SchedPolicy;

// Déclarations des politiques
extern SchedPolicy FIFO_POLICY;
extern SchedPolicy SJF_POLICY;
extern SchedPolicy SRJF_POLICY;
extern SchedPolicy RR_POLICY;

// Fonctions de sélection (à implémenter dans les fichiers *_select.c)
int fifo_select(Process **ready, int n, int quantum, int current_time);
int sjf_select(Process **ready, int n, int quantum, int current_time);
int sjrf_select(Process **ready, int n, int quantum, int current_time);
int rr_select(Process **ready, int n, int quantum, int current_time);

// Simulateur générique
void simulate(Process *processes, int count, SchedPolicy *policy, ScheduleResult *result);

// Utilitaires (déjà existants)
Process* read_processes_from_file(const char *filename, int *count);
void export_to_csv(const char *filename, ScheduleResult *result, Process *processes, int count);
void free_processes(Process *processes, int count);

#endif