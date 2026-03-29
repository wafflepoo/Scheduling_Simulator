#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "shared.c"


void sjf_schedule(Process *processes, int count, ScheduleResult *result) {
    printf("\n=== Simulation SJF ===\n");
    int *done = calloc(count, sizeof(int)); //init à 0
    if (!done) { perror("calloc"); return; } //test erreur d´alloc

    int current_time    = 0;
    int completed       = 0;
    int total_wait      = 0;
    int total_turnaround = 0;
    int total_response  = 0;

    while (completed < count) {

        int chosen = -1;
        int shortest = -1;

        for (int i = 0; i < count; i++) {
            if ((done[i])||(processes[i].arrival_time > current_time)){

            }
            else{
                int restant = total_remaining_cpu(&processes[i]);
                if (chosen == -1 || restant < shortest) {
                shortest = restant;
                chosen   = i;
                }
            }

        }
        if (chosen == -1) {
            int next_arrival = -1;
            for (int i = 0; i < count; i++) {
                if (done[i]){

                }
                else if (next_arrival == -1 || processes[i].arrival_time < next_arrival)
                    next_arrival = processes[i].arrival_time;
            }
            current_time = next_arrival;
        }

        Process *p = &processes[chosen];

        p->response_time = current_time - p->arrival_time;
        p->start_time    = current_time;

        printf("[T=%d] Début P%d (arrivé=%d, CPU restant=%d, réponse=%d)\n",
               current_time, p->pid, p->arrival_time,
               total_remaining_cpu(p), p->response_time);

        /*Pas I/O pour l´instant*/
        int process_cpu = 0;
        for (int j = 0; j < p->num_bursts; j += 2) {
            process_cpu += p->cpu_bursts[j];
        }
        current_time += process_cpu;
        p->finish_time = current_time;

        int turnaround        = p->finish_time - p->arrival_time;
        p->total_wait_time    = turnaround - process_cpu;

        printf("[T=%d] Fin P%d (turnaround=%d, attente=%d)\n",
               current_time, p->pid, turnaround, p->total_wait_time);

        total_wait        += p->total_wait_time;
        total_turnaround  += turnaround;
        total_response    += p->response_time;
        done[chosen]       = 1;
        completed++;
    }

    compute_averages(processes, count,
                     total_wait, total_turnaround, total_response,
                     current_time, result);
    print_results("SJF", result);
    free(done);
}