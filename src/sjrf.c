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

static int select_shortest_remaining(Process **ready, int n) {
    int idx = 0;
    int min = ready[0]->remaining_burst;
    for (int i = 1; i < n; i++) {
        if (ready[i]->remaining_burst < min) {
            min = ready[i]->remaining_burst;
            idx = i;
        }
    }
    return idx;
}

void sjrf_schedule(Process *processes, int count, ScheduleResult *result) {
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

    // Estimation de la durée maximale pour la timeline
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
    int ready_cnt = 0;
    IOEvent *io_head = NULL;
    int current_time = 0;
    int next_arrival = 0;
    Process *running = NULL;

    int total_cpu = 0;
    for (int i = 0; i < count; i++)
        for (int j = 0; j < proc[i].num_bursts; j++)
            total_cpu += proc[i].cpu_bursts[j];

    printf("\n=== SRJF (préemptif) ===\n");

    while (next_arrival < count || ready_cnt > 0 || io_head || running) {
        // 1. Déterminer le prochain événement
        int next_event = INT_MAX;
        if (next_arrival < count) next_event = proc[next_arrival].arrival_time;
        if (io_head && io_head->finish_time < next_event) next_event = io_head->finish_time;
        if (running && running->remaining_burst > 0) {
            int burst_end = current_time + running->remaining_burst;
            if (burst_end < next_event) next_event = burst_end;
        }
        if (next_event == INT_MAX) break;

        int elapsed = next_event - current_time;

        // 2. Remplir la timeline et ajouter les temps d'attente
        for (int t = current_time; t < next_event; t++) {
            if (running) timeline[running->pid-1][t] = 'U';
            for (int i = 0; i < ready_cnt; i++) timeline[ready[i]->pid-1][t] = 'W';
            for (IOEvent *e = io_head; e; e = e->next) timeline[e->process->pid-1][t] = 'O';
        }
        for (int i = 0; i < ready_cnt; i++) ready[i]->total_wait_time += elapsed;

        // 3. Mettre à jour le temps restant du processus en cours
        if (running) {
            running->remaining_burst -= elapsed;
        }

        current_time = next_event;

        // 4. Traiter les arrivées
        while (next_arrival < count && proc[next_arrival].arrival_time <= current_time) {
            ready[ready_cnt++] = &proc[next_arrival];
            next_arrival++;
        }

        // 5. Traiter les fins d'E/S
        while (io_head && io_head->finish_time <= current_time) {
            Process *p = io_head->process;
            p->remaining_burst = p->cpu_bursts[p->current_burst_index];
            ready[ready_cnt++] = p;
            IOEvent *tmp = io_head;
            io_head = io_head->next;
            free(tmp);
        }

        // 6. Vérifier si le burst du processus en cours est terminé
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
        }

        // 7. Préemption : si un processus ready a un temps restant plus petit
        if (running && ready_cnt > 0) {
            int best_idx = select_shortest_remaining(ready, ready_cnt);
            Process *best = ready[best_idx];
            if (best->remaining_burst < running->remaining_burst) {
                printf("[T=%d] Préemption : P%d (restant %d) -> P%d (restant %d)\n",
                       current_time, running->pid, running->remaining_burst,
                       best->pid, best->remaining_burst);
                // Remettre running dans ready
                ready[ready_cnt++] = running;
                // Enlever best de ready
                for (int i = best_idx; i < ready_cnt-1; i++) ready[i] = ready[i+1];
                ready_cnt--;
                // Nouveau running
                running = best;
                if (running->response_time == -1)
                    running->response_time = current_time - running->arrival_time;
                printf("[T=%d] P%d reprend (burst restant %d)\n", current_time, running->pid, running->remaining_burst);
            }
        }

        // 8. Démarrer un nouveau processus si CPU libre
        if (running == NULL && ready_cnt > 0) {
            int idx = select_shortest_remaining(ready, ready_cnt);
            running = ready[idx];
            for (int i = idx; i < ready_cnt-1; i++) ready[i] = ready[i+1];
            ready_cnt--;
            if (running->response_time == -1)
                running->response_time = current_time - running->arrival_time;
            printf("[T=%d] P%d démarre (burst %d)\n", current_time, running->pid, running->remaining_burst);
        }
    }

    // Affichage de la chronologie
    printf("\nChronologie (U=CPU, O=E/S, W=Attente)\n");
    for (int i = 0; i < count; i++) {
        printf("P%d: ", proc[i].pid);
        for (int t = 0; t < max_time; t++) {
            if (t % 20 == 0 && t) printf(" ");
            printf("%c", timeline[i][t]);
        }
        printf("\n");
    }

    // Calcul des résultats
    int total_wait = 0, total_turn = 0, total_resp = 0;
    for (int i = 0; i < count; i++) {
        int turn = proc[i].finish_time - proc[i].arrival_time;
        total_turn += turn;
        total_wait += proc[i].total_wait_time;
        total_resp += proc[i].response_time;
    }

    result->avg_wait_time = (float)total_wait / count;
    result->avg_turnaround_time = (float)total_turn / count;
    result->avg_response_time = (float)total_resp / count;
    result->cpu_utilization = (float)total_cpu / current_time * 100;

    printf("\n=== Résultats SRJF ===\n");
    printf("Temps d'attente moyen : %.2f\n", result->avg_wait_time);
    printf("Turnaround moyen      : %.2f\n", result->avg_turnaround_time);
    printf("Temps de réponse moyen: %.2f\n", result->avg_response_time);
    printf("Utilisation CPU       : %.2f%%\n", result->cpu_utilization);

    for (int i = 0; i < count; i++) free(timeline[i]);
    free(timeline);
    free(proc);
}