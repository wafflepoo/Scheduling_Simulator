#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

/* ─────────────────────────────────────────────
 *  SHARED HELPERS
 * ───────────────────────────────────────────── */


static int total_remaining_cpu(Process *p) {
    int total = 0;
    for (int i = p->current_burst_index; i < p->num_bursts; i += 2) {
        total += p->cpu_bursts[i];
    }
    return total;
}

static void print_results(const char *algo_name, ScheduleResult *result) {
    printf("\n=== Résultats %s ===\n", algo_name);
    printf("Temps d'attente moyen     : %.2f\n", result->avg_wait_time);
    printf("Turnaround moyen          : %.2f\n", result->avg_turnaround_time);
    printf("Temps de réponse moyen    : %.2f\n", result->avg_response_time);
    printf("Taux d'occupation CPU     : %.2f%%\n", result->cpu_utilization);
}

static void compute_averages(Process *processes, int count,
                              int total_wait, int total_turnaround,
                              int total_response, int end_time,
                              ScheduleResult *result) {
    if (count == 0) return;

    result->avg_wait_time       = (float)total_wait       / count;
    result->avg_turnaround_time = (float)total_turnaround / count;
    result->avg_response_time   = (float)total_response   / count;

    int total_cpu = 0;
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < processes[i].num_bursts; j += 2) {
            total_cpu += processes[i].cpu_bursts[j];
        }
    }
    result->cpu_utilization = (end_time > 0) ? (float)total_cpu / end_time * 100.0f : 0.0f; //à changer pour plus de clareté, code généré
}