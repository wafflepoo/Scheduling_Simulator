#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "scheduler.h"

typedef struct IOEvent {
    int finish_time;
    Process *process;
    struct IOEvent *next;
} IOEvent;

static void insert_io_event(IOEvent **head, int finish_time, Process *p) {
    IOEvent *new = malloc(sizeof(IOEvent));
    new->finish_time = finish_time;
    new->process = p;
    new->next = NULL;
    if (*head == NULL || (*head)->finish_time > finish_time) {
        new->next = *head;
        *head = new;
    } else {
        IOEvent *cur = *head;
        while (cur->next && cur->next->finish_time <= finish_time)
            cur = cur->next;
        new->next = cur->next;
        cur->next = new;
    }
}

static int cmp_arrival(const void *a, const void *b) {
    return ((Process*)a)->arrival_time - ((Process*)b)->arrival_time;
}

void simulate(Process *processes, int count, SchedPolicy *policy, ScheduleResult *result) {
    if (count == 0) return;

    Process *proc = malloc(count * sizeof(Process));
    memcpy(proc, processes, count * sizeof(Process));
    qsort(proc, count, sizeof(Process), cmp_arrival);

    for (int i = 0; i < count; i++) {
        proc[i].remaining_burst = proc[i].cpu_bursts[0];
        proc[i].current_burst_index = 0;
        proc[i].finish_time = 0;
        proc[i].total_wait_time = 0;
        proc[i].response_time = -1;
        proc[i].state = NEW;
    }

    int max_time = 0;
    for (int i = 0; i < count; i++) {
        int total = proc[i].arrival_time;
        for (int j = 0; j < proc[i].num_bursts; j++) total += proc[i].cpu_bursts[j];
        for (int j = 0; j < proc[i].num_bursts - 1; j++) total += proc[i].io_bursts[j];
        if (total > max_time) max_time = total;
    }
    max_time += 10;

    char **timeline = malloc(count * sizeof(char*));
    for (int i = 0; i < count; i++) {
        timeline[i] = malloc(max_time);
        memset(timeline[i], '.', max_time);
    }

    Process *ready[100];
    int ready_head = 0, ready_tail = 0;
    IOEvent *io_head = NULL;
    int current_time = 0;
    int next_arrival = 0;
    Process *running = NULL;
    int remaining_quantum = 0;

    int total_cpu = 0;
    for (int i = 0; i < count; i++)
        for (int j = 0; j < proc[i].num_bursts; j++)
            total_cpu += proc[i].cpu_bursts[j];

    printf("\n=== Simulation %s ===\n", policy->name);
    if (policy->quantum > 0)
        printf("Quantum = %d ms\n", policy->quantum);

    while (next_arrival < count || ready_tail != ready_head || io_head || running) {
        int next_event = INT_MAX;
        if (next_arrival < count)
            next_event = proc[next_arrival].arrival_time;
        if (io_head && io_head->finish_time < next_event)
            next_event = io_head->finish_time;
        if (running && remaining_quantum > 0) {
            int fin_quantum = current_time + remaining_quantum;
            if (fin_quantum < next_event)
                next_event = fin_quantum;
        }
        if (running && running->remaining_burst > 0) {
            int fin_burst = current_time + running->remaining_burst;
            if (fin_burst < next_event)
                next_event = fin_burst;
        }
        if (next_event == INT_MAX) break;

        int elapsed = next_event - current_time;

        for (int i = ready_head; i != ready_tail; i = (i+1)%100)
            ready[i]->total_wait_time += elapsed;

        for (int t = current_time; t < next_event; t++) {
            if (running)
                timeline[running->pid-1][t] = 'U';
            for (int i = ready_head; i != ready_tail; i = (i+1)%100)
                timeline[ready[i]->pid-1][t] = 'W';
            for (IOEvent *e = io_head; e; e = e->next)
                timeline[e->process->pid-1][t] = 'O';
        }

        current_time = next_event;
        if (running) {
            running->remaining_burst -= elapsed;
            remaining_quantum -= elapsed;
        }

        while (next_arrival < count && proc[next_arrival].arrival_time <= current_time) {
            ready[ready_tail] = &proc[next_arrival];
            ready_tail = (ready_tail+1)%100;
            next_arrival++;
        }

        while (io_head && io_head->finish_time <= current_time) {
            Process *p = io_head->process;
            p->remaining_burst = p->cpu_bursts[p->current_burst_index];
            ready[ready_tail] = p;
            ready_tail = (ready_tail+1)%100;
            IOEvent *tmp = io_head;
            io_head = io_head->next;
            free(tmp);
        }

        // ========== AJOUT : PRÉEMPTION POUR SRJF ==========
        if (running != NULL && strcmp(policy->name, "SRJF") == 0) {
            int ready_count = (ready_tail - ready_head + 100) % 100;
            if (ready_count > 0) {
                // Trouver l'indice du processus prêt avec le plus court temps restant
                int idx = policy->select_next(ready, ready_count, 0, current_time);
                if (idx >= 0) {
                    int real_idx = (ready_head + idx) % 100;
                    Process *shortest = ready[real_idx];
                    // Si le processus prêt a un temps restant strictement plus court,
                    // on préempte le processus courant
                    if (shortest->remaining_burst < running->remaining_burst) {
                        // Remettre le processus courant dans la file des prêts
                        ready[ready_tail] = running;
                        ready_tail = (ready_tail + 1) % 100;
                        printf("[T=%d] P%d préempté par P%d (burst restant %d < %d)\n",
                               current_time, running->pid, shortest->pid,
                               shortest->remaining_burst, running->remaining_burst);
                        running = NULL;
                        remaining_quantum = 0; // SRJF n'utilise pas de quantum
                    }
                }
            }
        }
        // ========== FIN AJOUT ==========

        if (running && running->remaining_burst == 0) {
            running->current_burst_index++;
            if (running->current_burst_index == running->num_bursts) {
                running->finish_time = current_time;
                printf("[T=%d] P%d terminé\n", current_time, running->pid);
                running = NULL;
            } else {
                int io_dur = running->io_bursts[running->current_burst_index-1];
                insert_io_event(&io_head, current_time + io_dur, running);
                printf("[T=%d] P%d -> E/S jusqu'à %d\n", current_time, running->pid, current_time+io_dur);
                running = NULL;
            }
        } else if (running && remaining_quantum == 0 && policy->quantum > 0) {
            ready[ready_tail] = running;
            ready_tail = (ready_tail+1)%100;
            printf("[T=%d] P%d quantum écoulé (reste %d), remis en file\n", current_time, running->pid, running->remaining_burst);
            running = NULL;
        }

        if (running == NULL && ready_tail != ready_head) {
            int idx = policy->select_next(ready, ready_tail - ready_head, policy->quantum, current_time);
            int real_idx = (ready_head + idx) % 100;
            running = ready[real_idx];
            for (int i = real_idx; i != ready_tail-1; i = (i+1)%100)
                ready[i] = ready[(i+1)%100];
            ready_tail = (ready_tail - 1 + 100) % 100;
            if (ready_tail < ready_head) ready_head = 0;

            if (running->response_time == -1)
                running->response_time = current_time - running->arrival_time;

            if (policy->quantum > 0) {
                remaining_quantum = (running->remaining_burst < policy->quantum) ? running->remaining_burst : policy->quantum;
                printf("[T=%d] P%d démarre (burst restant %d, quantum %d)\n",
                       current_time, running->pid, running->remaining_burst, remaining_quantum);
            } else {
                printf("[T=%d] P%d démarre (burst restant %d)\n",
                       current_time, running->pid, running->remaining_burst);
            }
        }
    }

    // Chronologie (optionnelle)
    printf("\nChronologie (U=CPU, O=E/S, W=Attente, .=inactif)\n");
    for (int i = 0; i < count; i++) {
        printf("P%d: ", proc[i].pid);
        for (int t = 0; t < max_time; t++) {
            if (t % 20 == 0 && t) printf(" ");
            printf("%c", timeline[i][t]);
        }
        printf("\n");
    }

    // --- Sortie tabulée (copiable dans Excel) ---
    printf("\n=== Résultats individuels (copiez-collez dans Excel) ===\n");
    printf("PID\tArrivée\tTurnaround\tAttente\tRéponse\n");
    int total_wait = 0, total_turn = 0, total_resp = 0;
    for (int i = 0; i < count; i++) {
        int turn = proc[i].finish_time - proc[i].arrival_time;
        total_turn += turn;
        total_wait += proc[i].total_wait_time;
        total_resp += proc[i].response_time;
        printf("%d\t%d\t%d\t%d\t%d\n",
               proc[i].pid, proc[i].arrival_time,
               turn, proc[i].total_wait_time, proc[i].response_time);
    }

    result->avg_wait_time = (float)total_wait / count;
    result->avg_turnaround_time = (float)total_turn / count;
    result->avg_response_time = (float)total_resp / count;
    result->cpu_utilization = (float)total_cpu / current_time * 100;

    printf("\n=== Résultats synthétiques ===\n");
    if (policy->quantum > 0)
        printf("Quantum = %d ms\n", policy->quantum);
    printf("Temps d'attente moyen\t%.2f ms\n", result->avg_wait_time);
    printf("Turnaround moyen\t%.2f ms\n", result->avg_turnaround_time);
    printf("Temps de réponse moyen\t%.2f ms\n", result->avg_response_time);
    printf("Taux d'occupation CPU\t%.2f %%\n", result->cpu_utilization);

    for (int i = 0; i < count; i++) free(timeline[i]);
    free(timeline);
    free(proc);
}