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
    IOEvent *new_event = malloc(sizeof(IOEvent));
    new_event->finish_time = finish_time;
    new_event->process = p;
    new_event->next = NULL;

    if (*head == NULL || (*head)->finish_time > finish_time) {
        new_event->next = *head;
        *head = new_event;
    } else {
        IOEvent *curr = *head;
        while (curr->next && curr->next->finish_time <= finish_time)
            curr = curr->next;
        new_event->next = curr->next;
        curr->next = new_event;
    }
}

static int cmp_arrival(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    return p1->arrival_time - p2->arrival_time;
}

// Sélection du processus avec le plus court prochain burst CPU
static int select_shortest_job(Process **ready_queue, int ready_count) {
    if (ready_count == 0) return -1;
    int idx = 0;
    int shortest = ready_queue[0]->cpu_bursts[ready_queue[0]->current_burst_index];
    for (int i = 1; i < ready_count; i++) {
        int burst = ready_queue[i]->cpu_bursts[ready_queue[i]->current_burst_index];
        if (burst < shortest) {
            shortest = burst;
            idx = i;
        }
    }
    return idx;
}

void sjf_schedule(Process *processes, int count, ScheduleResult *result) {
    if (count == 0) return;

    Process *proc = malloc(count * sizeof(Process));
    memcpy(proc, processes, count * sizeof(Process));
    qsort(proc, count, sizeof(Process), cmp_arrival);

    // Estimation de la durée totale pour la timeline
    int max_time = 0;
    for (int i = 0; i < count; i++) {
        int total = proc[i].arrival_time;
        for (int j = 0; j < proc[i].num_bursts; j++) total += proc[i].cpu_bursts[j];
        for (int j = 0; j < proc[i].num_bursts - 1; j++) total += proc[i].io_bursts[j];
        if (total > max_time) max_time = total;
    }
    max_time += 10;

    char **timeline = malloc(count * sizeof(char *));
    for (int i = 0; i < count; i++) {
        timeline[i] = malloc(max_time * sizeof(char));
        memset(timeline[i], '.', max_time);
    }

    Process *ready_queue[100];
    int ready_count = 0;
    IOEvent *io_head = NULL;
    int current_time = 0;
    int next_arrival_idx = 0;
    Process *running = NULL;

    int total_cpu_time = 0;
    for (int i = 0; i < count; i++)
        for (int j = 0; j < proc[i].num_bursts; j++)
            total_cpu_time += proc[i].cpu_bursts[j];

    printf("\n=== Simulation SJF (non préemptif) ===\n");

    while (next_arrival_idx < count || ready_count > 0 || io_head || running) {
        // 1. Prochain événement
        int next_event = INT_MAX;
        if (next_arrival_idx < count)
            next_event = proc[next_arrival_idx].arrival_time;
        if (io_head && io_head->finish_time < next_event)
            next_event = io_head->finish_time;
        if (running && running->remaining_burst > 0) {
            int burst_end = running->start_time + running->remaining_burst;
            if (burst_end < next_event)
                next_event = burst_end;
        }
        if (next_event == INT_MAX) break;

        // 2. Remplir la timeline
        for (int t = current_time; t < next_event; t++) {
            if (running != NULL)
                timeline[running->pid - 1][t] = 'U';
            for (int i = 0; i < ready_count; i++)
                timeline[ready_queue[i]->pid - 1][t] = 'W';
            for (IOEvent *e = io_head; e != NULL; e = e->next)
                timeline[e->process->pid - 1][t] = 'O';
        }

        // 3. Mettre à jour les temps d'attente
        int elapsed = next_event - current_time;
        for (int i = 0; i < ready_count; i++)
            ready_queue[i]->total_wait_time += elapsed;
        current_time = next_event;

        // 4. Arrivées
        while (next_arrival_idx < count && proc[next_arrival_idx].arrival_time <= current_time) {
            ready_queue[ready_count++] = &proc[next_arrival_idx];
            next_arrival_idx++;
        }

        // 5. Fins d'E/S
        while (io_head && io_head->finish_time <= current_time) {
            ready_queue[ready_count++] = io_head->process;
            IOEvent *tmp = io_head;
            io_head = io_head->next;
            free(tmp);
        }

        // 6. Fin du burst du processus en cours ?
        if (running && running->remaining_burst > 0 &&
            current_time == running->start_time + running->remaining_burst) {
            running->remaining_burst = 0;
        }
        if (running && running->remaining_burst == 0) {
            running->current_burst_index++;
            if (running->current_burst_index == running->num_bursts) {
                running->finish_time = current_time;
                printf("[T=%d] P%d terminé\n", current_time, running->pid);
                running = NULL;
            } else {
                int io_duration = running->io_bursts[running->current_burst_index - 1];
                insert_io_event(&io_head, current_time + io_duration, running);
                printf("[T=%d] P%d part en E/S (fin %d)\n",
                       current_time, running->pid, current_time + io_duration);
                running = NULL;
            }
        }

        // 7. Démarrer un nouveau processus selon SJF
        if (running == NULL && ready_count > 0) {
            int idx = select_shortest_job(ready_queue, ready_count);
            running = ready_queue[idx];
            // Retirer de la file
            for (int i = idx; i < ready_count - 1; i++)
                ready_queue[i] = ready_queue[i + 1];
            ready_count--;

            if (running->response_time == -1)
                running->response_time = current_time - running->arrival_time;

            running->remaining_burst = running->cpu_bursts[running->current_burst_index];
            running->start_time = current_time;
            printf("[T=%d] P%d commence burst de %d (SJF)\n", current_time, running->pid,
                   running->remaining_burst);
        }
    }

    // Affichage chronologie
    printf("\nChronologie (U=CPU, O=E/S, W=Attente, .=inactif)\n");
    for (int i = 0; i < count; i++) {
        printf("P%d: ", proc[i].pid);
        for (int t = 0; t < max_time; t++) {
            if (t > 0 && t % 20 == 0) printf(" ");
            printf("%c", timeline[i][t]);
        }
        printf("\n");
    }

    // Calcul des indicateurs
    int total_wait = 0, total_turnaround = 0, total_response = 0;
    for (int i = 0; i < count; i++) {
        Process *p = &proc[i];
        int turnaround = p->finish_time - p->arrival_time;
        total_turnaround += turnaround;
        total_wait += p->total_wait_time;
        total_response += p->response_time;
    }

    result->avg_wait_time = (float)total_wait / count;
    result->avg_turnaround_time = (float)total_turnaround / count;
    result->avg_response_time = (float)total_response / count;
    result->cpu_utilization = (float)total_cpu_time / current_time * 100;

    printf("\n=== Résultats SJF ===\n");
    printf("Temps d'attente moyen: %.2f\n", result->avg_wait_time);
    printf("Turnaround moyen: %.2f\n", result->avg_turnaround_time);
    printf("Temps de réponse moyen: %.2f\n", result->avg_response_time);
    printf("Taux d'occupation CPU: %.2f%%\n", result->cpu_utilization);

    // Libération
    for (int i = 0; i < count; i++) free(timeline[i]);
    free(timeline);
    free(proc);
}