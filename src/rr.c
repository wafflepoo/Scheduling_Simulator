<<<<<<< HEAD
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

void rr_schedule(Process *processes, int count, ScheduleResult *result) {
    int quantum = 2;   // ← MODIFIER ICI : 1, 2 ou 3 selon l'exemple

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

    // Estimation de la durée max pour la timeline
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

    printf("\n=== Round Robin (quantum = %d ms) ===\n", quantum);

    while (next_arrival < count || ready_tail != ready_head || io_head || running) {
        // 1. Prochain événement
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

        // 2. Mise à jour des temps d'attente
        for (int i = ready_head; i != ready_tail; i = (i+1)%100)
            ready[i]->total_wait_time += elapsed;

        // 3. Remplir la timeline
        for (int t = current_time; t < next_event; t++) {
            if (running)
                timeline[running->pid-1][t] = 'U';
            for (int i = ready_head; i != ready_tail; i = (i+1)%100)
                timeline[ready[i]->pid-1][t] = 'W';
            for (IOEvent *e = io_head; e; e = e->next)
                timeline[e->process->pid-1][t] = 'O';
        }

        // 4. Avancer le temps
        current_time = next_event;
        if (running) {
            running->remaining_burst -= elapsed;
            remaining_quantum -= elapsed;
        }

        // 5. Arrivées
        while (next_arrival < count && proc[next_arrival].arrival_time <= current_time) {
            ready[ready_tail] = &proc[next_arrival];
            ready_tail = (ready_tail+1)%100;
            next_arrival++;
        }

        // 6. Fins d'E/S
        while (io_head && io_head->finish_time <= current_time) {
            Process *p = io_head->process;
            p->remaining_burst = p->cpu_bursts[p->current_burst_index];
            ready[ready_tail] = p;
            ready_tail = (ready_tail+1)%100;
            IOEvent *tmp = io_head;
            io_head = io_head->next;
            free(tmp);
        }

        // 7. Gestion de la fin de burst ou fin de quantum
        if (running && running->remaining_burst == 0) {
            // Burst terminé
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
        } else if (running && remaining_quantum == 0) {
            // Quantum écoulé, remettre en file
            ready[ready_tail] = running;
            ready_tail = (ready_tail+1)%100;
            printf("[T=%d] P%d quantum écoulé (reste %d), remis en file\n", current_time, running->pid, running->remaining_burst);
            running = NULL;
        }

        // 8. Si CPU libre, prendre le prochain processus
        if (running == NULL && ready_tail != ready_head) {
            running = ready[ready_head];
            ready_head = (ready_head+1)%100;
            if (running->response_time == -1)
                running->response_time = current_time - running->arrival_time;
            remaining_quantum = (running->remaining_burst < quantum) ? running->remaining_burst : quantum;
            printf("[T=%d] P%d démarre (burst restant %d, quantum %d)\n",
                   current_time, running->pid, running->remaining_burst, remaining_quantum);
        }
    }

    // Affichage de la chronologie
    printf("\nChronologie (U=CPU, O=E/S, W=Attente, .=inactif)\n");
    for (int i = 0; i < count; i++) {
        printf("P%d: ", proc[i].pid);
        for (int t = 0; t < max_time; t++) {
            if (t % 20 == 0 && t) printf(" ");
            printf("%c", timeline[i][t]);
        }
        printf("\n");
    }

    // Calcul des résultats individuels et globaux
    int total_wait = 0, total_turn = 0, total_resp = 0;
    printf("\nDétail par processus :\n");
    for (int i = 0; i < count; i++) {
        int turn = proc[i].finish_time - proc[i].arrival_time;
        total_turn += turn;
        total_wait += proc[i].total_wait_time;
        total_resp += proc[i].response_time;
        printf("P%d: arrivée=%d, fin=%d, turnaround=%d, attente=%d, réponse=%d\n",
               proc[i].pid, proc[i].arrival_time, proc[i].finish_time,
               turn, proc[i].total_wait_time, proc[i].response_time);
    }

    result->avg_wait_time = (float)total_wait / count;
    result->avg_turnaround_time = (float)total_turn / count;
    result->avg_response_time = (float)total_resp / count;
    result->cpu_utilization = (float)total_cpu / current_time * 100;

    printf("\n=== Résultats Round Robin (q=%d) ===\n", quantum);
    printf("Temps d'attente moyen : %.2f\n", result->avg_wait_time);
    printf("Turnaround moyen      : %.2f\n", result->avg_turnaround_time);
    printf("Temps de réponse moyen: %.2f\n", result->avg_response_time);
    printf("Utilisation CPU       : %.2f%%\n", result->cpu_utilization);

    for (int i = 0; i < count; i++) free(timeline[i]);
    free(timeline);
    free(proc);
}
=======
/*
 * rr.c — Algorithme d'ordonnancement Round Robin
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

#define QUANTUM_DEFAUT  2
#define MAX_GANTT    1000

/* Tranche du diagramme de Gantt (locale à ce fichier) */
typedef struct {
    int pid;
    int debut;
    int fin;
} Tranche;

/* Durée CPU totale = somme de tous les cpu_bursts */
static int duree_cpu_totale(Process *p) {
    int total = 0;
    for (int i = 0; i < p->num_bursts; i++) total += p->cpu_bursts[i];
    return total;
}

/* Affichage ASCII du diagramme de Gantt */
static void afficher_gantt(Tranche gantt[], int nb) {
    printf("\n=== DIAGRAMME DE GANTT (Round Robin) ===\n\n+");
    for (int i = 0; i < nb; i++) printf("-----+");
    printf("\n|");
    for (int i = 0; i < nb; i++) printf(" P%-2d |", gantt[i].pid);
    printf("\n+");
    for (int i = 0; i < nb; i++) printf("-----+");
    printf("\n%d", gantt[0].debut);
    for (int i = 0; i < nb; i++) printf("    %d", gantt[i].fin);
    printf("\n");
}

/* Sauvegarde CSV */
static void sauvegarder_csv(Process *processes, int count, ScheduleResult *result) {
    FILE *f = fopen("resultats_RR.csv", "w");
    if (!f) { printf("Erreur : impossible de créer resultats_RR.csv\n"); return; }
    fprintf(f, "PID;Arrivee;Duree;Reponse;Attente;Restitution\n");
    for (int i = 0; i < count; i++) {
        int duree = result->turnaround_times[i] - result->wait_times[i];
        fprintf(f, "%d;%d;%d;%d;%d;%d\n",
                processes[i].pid, processes[i].arrival_time, duree,
                result->response_times[i], result->wait_times[i],
                result->turnaround_times[i]);
    }
    fclose(f);
    printf("\nRésultats sauvegardés dans : resultats_RR.csv\n");
}

/**
 * @brief Simule l'ordonnancement Round Robin.
 *
 * Chaque processus reçoit au plus QUANTUM_DEFAUT ms de CPU par tour.
 * S'il n'est pas terminé, il est remis en fin de file.
 * Les E/S sont parallélisées : elles ne bloquent pas le CPU.
 *
 * @param processes Tableau des processus
 * @param count     Nombre de processus
 * @param result    Structure résultats à remplir
 */
void rr_schedule(Process *processes, int count, ScheduleResult *result) {
    int quantum = QUANTUM_DEFAUT;

    /* Allouer les tableaux de résultats par processus */
    result->wait_times       = calloc(count, sizeof(int));
    result->turnaround_times = calloc(count, sizeof(int));
    result->response_times   = calloc(count, sizeof(int));
    if (!result->wait_times || !result->turnaround_times || !result->response_times) {
        printf("Erreur : allocation mémoire échouée\n");
        return;
    }

    /* Initialisation */
    for (int i = 0; i < count; i++) {
        processes[i].remaining_time      = processes[i].cpu_bursts[0];
        processes[i].current_burst_index = 0;
        processes[i].total_wait_time     = 0;
        processes[i].finish_time         = 0;
        processes[i].start_time          = -1;  /* -1 = jamais eu le CPU */
    }

    int temps     = 0;
    int termines  = 0;
    int total_cpu = 0;

    int file[MAX_GANTT];
    int debut_file = 0, fin_file = 0;

    Tranche gantt[MAX_GANTT];
    int nb_gantt = 0;

    /* Ajouter les processus arrivant à t=0 */
    for (int i = 0; i < count; i++) {
        if (processes[i].arrival_time == 0)
            file[fin_file++] = i;
    }

    printf("\n=== Simulation Round Robin (quantum = %d ms) ===\n\n", quantum);

    /* Boucle principale */
    while (termines < count) {

        /* File vide : avancer au prochain arrivant */
        if (debut_file == fin_file) {
            int prochain = -1;
            for (int i = 0; i < count; i++) {
                if (processes[i].finish_time == 0 && processes[i].arrival_time > temps)
                    if (prochain == -1 || processes[i].arrival_time < prochain)
                        prochain = processes[i].arrival_time;
            }
            if (prochain == -1) break;
            temps = prochain;
            for (int i = 0; i < count; i++) {
                if (processes[i].arrival_time == temps && processes[i].finish_time == 0)
                    file[fin_file++] = i;
            }
        }

        /* Prendre le processus en tête de file */
        int idx = file[debut_file++];
        Process *p = &processes[idx];

        /* Enregistrer le temps de réponse au 1er accès CPU */
        if (p->start_time == -1) {
            p->start_time    = temps;
            p->response_time = temps - p->arrival_time;
        }

        /* Temps d'exécution pour ce tour */
        int exec = (p->remaining_time < quantum) ? p->remaining_time : quantum;

        printf("[t=%d] P%d s'exécute %d ms (burst %d/%d, reste=%d)\n",
               temps, p->pid, exec, p->current_burst_index + 1,
               p->num_bursts, p->remaining_time);

        /* Enregistrer dans le Gantt */
        gantt[nb_gantt].pid   = p->pid;
        gantt[nb_gantt].debut = temps;
        gantt[nb_gantt].fin   = temps + exec;
        nb_gantt++;

        temps             += exec;
        p->remaining_time -= exec;
        total_cpu         += exec;

        /* Ajouter les processus arrivés pendant cette exécution */
        for (int i = 0; i < count; i++) {
            if (i == idx || processes[i].finish_time != 0) continue;
            if (processes[i].arrival_time > temps - exec
                && processes[i].arrival_time <= temps) {
                int deja = 0;
                for (int f = debut_file; f < fin_file; f++)
                    if (file[f] == i) { deja = 1; break; }
                if (!deja) {
                    file[fin_file++] = i;
                    printf("         → P%d arrive et rejoint la file\n", processes[i].pid);
                }
            }
        }

        /* Burst courant terminé ? */
        if (p->remaining_time == 0) {
            p->current_burst_index++;
            if (p->current_burst_index < p->num_bursts) {
                /* E/S parallélisée : passer au burst suivant et remettre en file */
                p->remaining_time = p->cpu_bursts[p->current_burst_index];
                file[fin_file++]  = idx;
                printf("         → P%d E/S parallélisée, burst suivant (%d ms)\n",
                       p->pid, p->remaining_time);
            } else {
                /* Processus complètement terminé */
                p->finish_time = temps;
                termines++;
                printf("         → P%d TERMINÉ à t=%d\n", p->pid, temps);
            }
        } else {
            /* Quantum épuisé : remettre en file */
            file[fin_file++] = idx;
            printf("         → P%d repart en file (reste %d ms)\n",
                   p->pid, p->remaining_time);
        }
        printf("         ----------------------------------------\n");
    }

    /* Calcul des métriques */
    float somme_att = 0, somme_rest = 0, somme_rep = 0;

    printf("\n=== Résultats Round Robin ===\n\n");
    printf("%-6s %-10s %-8s %-10s %-10s %-12s\n",
           "PID", "Arrivée", "Durée", "Réponse", "Attente", "Restitution");
    printf("--------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        int duree       = duree_cpu_totale(&processes[i]);
        int restitution = processes[i].finish_time - processes[i].arrival_time;
        int attente     = restitution - duree;

        result->turnaround_times[i] = restitution;
        result->wait_times[i]       = attente;
        result->response_times[i]   = processes[i].response_time;

        somme_rest += restitution;
        somme_att  += attente;
        somme_rep  += processes[i].response_time;

        printf("P%-5d %-10d %-8d %-10d %-10d %-12d\n",
               processes[i].pid, processes[i].arrival_time, duree,
               processes[i].response_time, attente, restitution);
    }

    printf("--------------------------------------------------------\n");

    result->avg_wait_time       = somme_att  / count;
    result->avg_turnaround_time = somme_rest / count;
    result->avg_response_time   = somme_rep  / count;
    result->cpu_utilization     = (float)total_cpu / temps * 100.0f;

    printf("\nTemps d'attente moyen      : %.2f ms\n", result->avg_wait_time);
    printf("Temps de restitution moyen : %.2f ms\n", result->avg_turnaround_time);
    printf("Temps de réponse moyen     : %.2f ms\n", result->avg_response_time);
    printf("Taux d'occupation CPU      : %.2f %%\n",  result->cpu_utilization);

    afficher_gantt(gantt, nb_gantt);
    sauvegarder_csv(processes, count, result);
}
>>>>>>> 02b4f8e2f4516f16dabc9f4e8fa311ff6b7e663a
