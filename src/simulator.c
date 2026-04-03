/**
 * @file simulator.c
 * @brief Cœur du simulateur d'ordonnancement : boucle evenementielle et gestion des files
 * @authors MISSAOUI Alissa (50%), TAKKA Kamelia (25%), HORNUNG Thomas (25%)
 * @date 2026-04-02
 * 
 * Ce module implemente la fonction simulate() qui execute la simulation
 * pour une politique d'ordonnancement donnee. Il gere :
 *   - Les arrivees de processus
 *   - Les files d'attente READY (tableau circulaire)
 *   - Les entrees/sorties (liste chaînee triee)
 *   - La preemption (notamment pour SRJF)
 *   - La generation de la timeline et des metriques
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "scheduler.h"


#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir(p, 0755)
#endif

/* ========================================================================
 * Structure interne pour la gestion des evenements d'E/S
 * ======================================================================== */

/**
 * @brief Structure representant un evenement de fin d'entree/sortie.
 * 
 * Les evenements sont organises dans une liste chaînee triee par finish_time.
 */
typedef struct IOEvent {
    int finish_time;           /**< Instant de fin de l'E/S */
    Process *process;          /**< Processus concerne */
    struct IOEvent *next;      /**< element suivant dans la liste */
} IOEvent;

/* ========================================================================
 * Fonctions statiques internes
 * ======================================================================== */

/**
 * @brief Insere un evenement d'E/S dans la liste triee par finish_time.
 * 
 * @param head         Pointeur vers la tête de la liste (modifiable).
 * @param finish_time  Instant auquel l'E/S se termine.
 * @param p            Processus qui sort d'E/S.
 */
static void insert_io_event(IOEvent **head, int finish_time, Process *p) {
    IOEvent *new = malloc(sizeof(IOEvent));
    new->finish_time = finish_time;
    new->process = p;
    new->next = NULL;
    
    /* Insertion en tête si la liste est vide ou si le nouvel evenement est le plus proche */
    if (*head == NULL || (*head)->finish_time > finish_time) {
        new->next = *head;
        *head = new;
    } else {
        IOEvent *cur = *head;
        while (cur->next && cur->next->finish_time <= finish_time) {
            cur = cur->next;
        }
        new->next = cur->next;
        cur->next = new;
    }
}

/**
 * @brief Comparateur pour qsort : tri par temps d'arrivee.
 * 
 * @param a Premier processus.
 * @param b Second processus.
 * @return int Difference des temps d'arrivee.
 */
static int cmp_arrival(const void *a, const void *b) {
    return ((Process*)a)->arrival_time - ((Process*)b)->arrival_time;
}

/* ========================================================================
 * Fonction publique d'export de la timeline
 * ======================================================================== */

/**
 * @brief Exporte la timeline (etats de chaque processus) dans un fichier CSV.
 * 
 * @param filename  Nom du fichier de sortie.
 * @param timeline  Tableau 2D des etats (count lignes, max_time colonnes).
 * @param count     Nombre de processus.
 * @param max_time  Duree totale simulee (nombre de colonnes).
 */
void export_timeline_csv(const char *filename, char **timeline, int count, int max_time) {
    FILE *f = fopen(filename, "w");
    if (!f) return;

    /* En-tête : premiere colonne "PID", puis les temps 0,1,2,... */
    fprintf(f, "PID");
    for (int t = 0; t < max_time; t++)
        fprintf(f, ",%d", t);
    fprintf(f, "\n");

    /* Pour chaque processus, ecrire son PID puis ses etats */
    for (int i = 0; i < count; i++) {
        fprintf(f, "P%d", i+1);
        for (int t = 0; t < max_time; t++) {
            fprintf(f, ",%c", timeline[i][t]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

/* ========================================================================
 * Fonction principale de simulation
 * ======================================================================== */

/**
 * @brief Lance la simulation d'ordonnancement pour une politique donnee.
 * 
 * Cette fonction implemente un moteur a evenements discrets. Elle gere :
 *   - Les arrivees de nouveaux processus (ajout dans la file READY)
 *   - Les fins de burst CPU (passage a l'E/S ou terminaison)
 *   - Les fins d'E/S (reinsertion dans la file READY)
 *   - Les fins de quantum (Round Robin)
 *   - La preemption SRJF (comparaison des remaining_burst)
 * 
 * @param processes Tableau original des processus (non modifie).
 * @param count     Nombre de processus.
 * @param policy    Politique d'ordonnancement (FIFO, SJF, SRJF, RR).
 * @param result    Structure qui recevra les metriques de performance.
 */
void simulate(Process *processes, int count, SchedPolicy *policy, ScheduleResult *result) {
    // Créer le dossier results/ s'il n'existe pas
    MKDIR("results");

    if (count == 0) return;

    /* Copie locale pour ne pas modifier l'original */
    Process *proc = malloc(count * sizeof(Process));
    memcpy(proc, processes, count * sizeof(Process));
    qsort(proc, count, sizeof(Process), cmp_arrival);

    /* Reinitialisation des champs dynamiques */
    for (int i = 0; i < count; i++) {
        proc[i].remaining_burst = proc[i].cpu_bursts[0];
        proc[i].current_burst_index = 0;
        proc[i].finish_time = 0;
        proc[i].total_wait_time = 0;
        proc[i].response_time = -1;
        proc[i].state = NEW;
    }

    /* Calcul de la duree maximale possible pour la timeline (majorant) */
    int max_time = 0;
    for (int i = 0; i < count; i++) {
        int total = proc[i].arrival_time;
        for (int j = 0; j < proc[i].num_bursts; j++) total += proc[i].cpu_bursts[j];
        for (int j = 0; j < proc[i].num_bursts - 1; j++) total += proc[i].io_bursts[j];
        if (total > max_time) max_time = total;
    }
    max_time += 10;   /* Marge de securite */

    /* Allocation de la timeline (etats pour chaque processus et chaque ms) */
    char **timeline = malloc(count * sizeof(char*));
    for (int i = 0; i < count; i++) {
        timeline[i] = malloc(max_time);
        memset(timeline[i], '.', max_time);   /* '.' = inactif par defaut */
    }

    /* File READY circulaire (capacite 100) */
    Process *ready[100];
    int ready_head = 0, ready_tail = 0;
    
    /* Liste des evenements d'E/S (finish_time croissant) */
    IOEvent *io_head = NULL;
    
    int current_time = 0;
    int next_arrival = 0;        /* Indice du prochain processus a arriver */
    Process *running = NULL;     /* Processus en cours d'execution */
    int remaining_quantum = 0;   /* Temps restant dans le quantum (pour RR) */

    /* Calcul du temps CPU total (pour l'utilisation CPU) */
    int total_cpu = 0;
    for (int i = 0; i < count; i++)
        for (int j = 0; j < proc[i].num_bursts; j++)
            total_cpu += proc[i].cpu_bursts[j];

    printf("\n=== Simulation %s ===\n", policy->name);
    if (policy->quantum > 0)
        printf("Quantum = %d ms\n", policy->quantum);

    /* Boucle principale : tant qu'il reste des evenements */
    while (next_arrival < count || ready_tail != ready_head || io_head || running) {
        /* 1. Determination du prochain evenement */
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
        if (next_event == INT_MAX) break;   /* Plus aucun evenement */

        int elapsed = next_event - current_time;

        /* 2. Mise a jour des temps d'attente pour les processus prêts */
        for (int i = ready_head; i != ready_tail; i = (i+1)%100)
            ready[i]->total_wait_time += elapsed;

        /* 3. Enregistrement de la timeline pour les ticks ecoules */
        for (int t = current_time; t < next_event; t++) {
            if (running)
                timeline[running->pid-1][t] = 'U';   /* CPU */
            for (int i = ready_head; i != ready_tail; i = (i+1)%100)
                timeline[ready[i]->pid-1][t] = 'W';   /* Attente */
            for (IOEvent *e = io_head; e; e = e->next)
                timeline[e->process->pid-1][t] = 'O'; /* E/S */
        }

        current_time = next_event;
        if (running) {
            running->remaining_burst -= elapsed;
            remaining_quantum -= elapsed;
        }

        /* 4. Traitement des arrivees */
        while (next_arrival < count && proc[next_arrival].arrival_time <= current_time) {
            ready[ready_tail] = &proc[next_arrival];
            ready_tail = (ready_tail+1)%100;
            next_arrival++;
        }

        /* 5. Traitement des fins d'E/S */
        while (io_head && io_head->finish_time <= current_time) {
            Process *p = io_head->process;
            /* Reinitialisation du remaining_burst pour le prochain burst CPU */
            p->remaining_burst = p->cpu_bursts[p->current_burst_index];
            ready[ready_tail] = p;
            ready_tail = (ready_tail+1)%100;
            IOEvent *tmp = io_head;
            io_head = io_head->next;
            free(tmp);
        }

        /* 6. Preemption specifique SRJF (verification a chaque evenement) */
        if (running != NULL && strcmp(policy->name, "SRJF") == 0) {
            int ready_count = (ready_tail - ready_head + 100) % 100;
            if (ready_count > 0) {
                int idx = policy->select_next(ready, ready_count, 0, current_time);
                if (idx >= 0) {
                    int real_idx = (ready_head + idx) % 100;
                    Process *shortest = ready[real_idx];
                    if (shortest->remaining_burst < running->remaining_burst) {
                        /* Preemption : remettre l'ancien processus dans la file */
                        ready[ready_tail] = running;
                        ready_tail = (ready_tail + 1) % 100;
                        printf("[T=%d] P%d preempte par P%d (burst restant %d < %d)\n",
                               current_time, running->pid, shortest->pid,
                               shortest->remaining_burst, running->remaining_burst);
                        running = NULL;
                        remaining_quantum = 0;
                    }
                }
            }
        }

        /* 7. Gestion de la fin du burst CPU ou de la terminaison */
        if (running && running->remaining_burst == 0) {
            running->current_burst_index++;
            if (running->current_burst_index == running->num_bursts) {
                running->finish_time = current_time;
                printf("[T=%d] P%d termine\n", current_time, running->pid);
                running = NULL;
            } else {
                int io_dur = running->io_bursts[running->current_burst_index-1];
                insert_io_event(&io_head, current_time + io_dur, running);
                printf("[T=%d] P%d -> E/S jusqu'a %d\n", current_time, running->pid, current_time+io_dur);
                running = NULL;
            }
        } 
        /* 8. Gestion de la fin de quantum (Round Robin) */
        else if (running && remaining_quantum == 0 && policy->quantum > 0) {
            ready[ready_tail] = running;
            ready_tail = (ready_tail+1)%100;
            printf("[T=%d] P%d quantum ecoule (reste %d), remis en file\n", current_time, running->pid, running->remaining_burst);
            running = NULL;
        }

        /* 9. Selection d'un nouveau processus si le CPU est libre */
        if (running == NULL && ready_tail != ready_head) {
            int idx = policy->select_next(ready, ready_tail - ready_head, policy->quantum, current_time);
            int real_idx = (ready_head + idx) % 100;
            running = ready[real_idx];
            /* Retrait de l'element choisi de la file circulaire */
            for (int i = real_idx; i != ready_tail-1; i = (i+1)%100)
                ready[i] = ready[(i+1)%100];
            ready_tail = (ready_tail - 1 + 100) % 100;
            if (ready_tail < ready_head) ready_head = 0;

            /* Enregistrement du temps de reponse (premier acces CPU) */
            if (running->response_time == -1)
                running->response_time = current_time - running->arrival_time;

            if (policy->quantum > 0) {
                remaining_quantum = (running->remaining_burst < policy->quantum) ? running->remaining_burst : policy->quantum;
                printf("[T=%d] P%d demarre (burst restant %d, quantum %d)\n",
                       current_time, running->pid, running->remaining_burst, remaining_quantum);
            } else {
                printf("[T=%d] P%d demarre (burst restant %d)\n",
                       current_time, running->pid, running->remaining_burst);
            }
        }
    }

    /* Affichage de la chronologie textuelle dans la console */
    printf("\nChronologie (U=CPU, O=E/S, W=Attente, .=inactif)\n");
    for (int i = 0; i < count; i++) {
        printf("P%d: ", proc[i].pid);
        for (int t = 0; t < max_time; t++) {
            if (t % 20 == 0 && t) printf(" ");
            printf("%c", timeline[i][t]);
        }
        printf("\n");
    }

    /* Affichage des resultats individuels (format copiable dans Excel) */
    printf("\n=== Resultats individuels (copiez-collez dans Excel) ===\n");
    printf("PID,Arrivee,Turnaround,Attente,Reponse\n");
    int total_wait = 0, total_turn = 0, total_resp = 0;
    for (int i = 0; i < count; i++) {
        int turn = proc[i].finish_time - proc[i].arrival_time;
        total_turn += turn;
        total_wait += proc[i].total_wait_time;
        total_resp += proc[i].response_time;
        printf("%d,%d,%d,%d,%d\n",
               proc[i].pid, proc[i].arrival_time,
               turn, proc[i].total_wait_time, proc[i].response_time);
    }

    /* Calcul des metriques globales */
    result->avg_wait_time = (float)total_wait / count;
    result->avg_turnaround_time = (float)total_turn / count;
    result->avg_response_time = (float)total_resp / count;
    result->cpu_utilization = (float)total_cpu / current_time * 100;

    /* Affichage synthetique */
    printf("\n=== Resultats synthetiques ===\n");
    if (policy->quantum > 0)
        printf("Quantum = %d ms\n", policy->quantum);
    printf("Temps d'attente moyen\t%.2f ms\n", result->avg_wait_time);
    printf("Turnaround moyen\t%.2f ms\n", result->avg_turnaround_time);
    printf("Temps de reponse moyen\t%.2f ms\n", result->avg_response_time);
    printf("Taux d'occupation CPU\t%.2f %%\n", result->cpu_utilization);

    /* Export des fichiers CSV (resultats et timeline) */
    char filename[256];
    snprintf(filename, sizeof(filename), "results/results_%s.csv", policy->name);
    export_to_csv(filename, result, proc, count);

    char filename2[256];
    snprintf(filename2, sizeof(filename2), "results/timeline_%s.csv", policy->name);
    export_timeline_csv(filename2, timeline, count, max_time);
    
    /* Nettoyage memoire */
    for (int i = 0; i < count; i++) free(timeline[i]);
    free(timeline);
    free(proc);
}
