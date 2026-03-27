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
