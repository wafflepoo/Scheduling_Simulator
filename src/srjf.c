#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "shared.c"

void sjrf_schedule(Process *processes, int count, ScheduleResult *result) {
    printf("\n=== Simulation SRJF ===\n");

    int *remaining   = malloc(count * sizeof(int));  
    int *io_restant     = malloc(count * sizeof(int)); //IO pas encore fonctionnel
    int *burst_index   = malloc(count * sizeof(int));
    int *done        = calloc(count, sizeof(int)); //comme dans sjf
    int *responded   = calloc(count, sizeof(int));
    int *started     = calloc(count, sizeof(int));

    if (!remaining || !io_restant || !burst_index || !done || !responded || !started) { //test erreur allocation
        perror("malloc"); return;
    }

    for (int i = 0; i < count; i++) { //calloc à la place?
        burst_index[i]  = 0;
        remaining[i]  = processes[i].cpu_bursts[0];
        io_restant[i]    = 0;
        processes[i].total_wait_time = 0;
        processes[i].response_time   = -1;
    }

    int current_time    = 0;
    int completed       = 0;
    int total_wait      = 0;
    int total_turnaround = 0;
    int total_response  = 0;
    int last_running    = -1;

    while (completed < count) { //on continue tant qu´on a pas parcouru tout les process, y compris si on a rien de disponible

        
        int chosen = -1;
        int shortest = -1;

        for (int i = 0; i < count; i++) {//on parcours tout nos processes et on regarde ceux dans notre fourchette temporelle actuelle
            if ((done[i])||(processes[i].arrival_time > current_time)||(io_restant[i] > 0)){ //IO pas encore implémentés totalement

            }
            else if (chosen == -1 || remaining[i] < shortest) { //si aucun process élu ou le temps restant du process est plus court que 
                //notre shortest actuel, on update le shortest. Tout initialisé à -1 donc on prendra toujours un process à la première itération
                //normalement pas de cas où shortest =-1 mais chosen != -1
                shortest = remaining[i];
                chosen   = i;
            }
        }

        if (chosen == -1) { //arrive si pas de changement dans la boucle plus haut
            int next_event = -1;
            for (int i = 0; i < count; i++) {
                if (done[i]){

                }
                else{
                    int candidat = -1; //
                    if (io_restant[i] > 0) {
                        candidat = current_time+io_restant[i];
                    } else if (processes[i].arrival_time > current_time) {
                        candidat = processes[i].arrival_time;
                    }
                    if (candidat != -1 && (next_event == -1 || candidat < next_event))
                        next_event = candidat;
                }
            }
            if (next_event == -1) break;  //On pourrait faire sans rupture

            //I/O counters, pas mon idée
            int skip = next_event - current_time;
            for (int i = 0; i < count; i++) {
                if (io_restant[i] > 0) {
                    io_restant[i] -= skip;
                    if (io_restant[i] < 0) io_restant[i] = 0;
                }
            }
            current_time = next_event;
        }

        Process *p = &processes[chosen];

        /* Stats */
        if (!responded[chosen]) {
            p->response_time  = current_time - p->arrival_time;
            p->start_time     = current_time;
            responded[chosen] = 1;
            printf("[T=%d] Première exécution P%d (réponse=%d)\n",
                   current_time, p->pid, p->response_time);
        }

        /* Switch de process*/
        if (last_running != p->pid) {
            if (last_running != -1)
                printf("[T=%d] Préemption → P%d prend le CPU (restant=%d)\n",
                       current_time, p->pid, remaining[chosen]);
            last_running = p->pid;
        }

        /*  Stats temps d´attente des process prêts */
        for (int i = 0; i < count; i++) {
            if ((i == chosen)||(done[i])||(processes[i].arrival_time > current_time)||(io_restant[i] > 0))
            processes[i].total_wait_time++;
        }

        /* Incrémentation par 1 */
        remaining[chosen]--;
        current_time++;

        /* gestion entrées/sortis, pas fini, nécéssite quelques améliorations et corrections*/
        for (int i = 0; i < count; i++) {
            if (io_restant[i] > 0) {
                io_restant[i]--;
                if (io_restant[i] == 0) {
                    burst_index[i]++;
                    if (burst_index[i] < processes[i].num_bursts) {
                        remaining[i] = processes[i].cpu_bursts[burst_index[i]];
                    }
                    printf("[T=%d] P%d fin I/O, retour prêt (prochain burst=%d)\n",
                           current_time, processes[i].pid, remaining[i]);
                }
            }
        }

        if (remaining[chosen] == 0) {
            int index = burst_index[chosen];

            int cpu_burst_num = index;
            if ((cpu_burst_num * 2 + 1) < processes[chosen].num_bursts) {
                int io_duration = processes[chosen].io_bursts[cpu_burst_num];
                io_restant[chosen] = io_duration;
                printf("[T=%d] P%d entre en I/O pour %d unités\n",
                       current_time, p->pid, io_duration);
                last_running = -1;
            } else {
                p->finish_time = current_time;
                int turnaround = p->finish_time - p->arrival_time;

                printf("[T=%d] Fin P%d (turnaround=%d, attente=%d)\n",
                       current_time, p->pid, turnaround, p->total_wait_time);

                total_wait       += p->total_wait_time;
                total_turnaround += turnaround;
                total_response   += p->response_time;
                done[chosen]      = 1;
                completed++;
                last_running      = -1;
            }
        }
    }

    compute_averages(processes, count, total_wait, total_turnaround, total_response, current_time, result);
    print_results("SRJF", result);
    //lib mémoire finale
    free(remaining); free(io_restant); free(burst_index);
    free(done); free(responded); free(started);
}