/**
 * @file parser.c
 * @brief Lecture et parsing des fichiers d'entrée contenant les processus
 * @authors MISSAOUI Alissa (50%), TAKKA Kamelia (25%), HORNUNG Thomas (25%)
 * @date 2026-04-02
 * 
 * Ce module lit un fichier texte décrivant un ensemble de processus,
 * construit un tableau de structures Process, et trie par temps d'arrivée.
 * 
 * Format du fichier (une ligne par processus, champs séparés par espaces/tabulations) :
 *   PID temps_arrivee cpu1 io1 cpu2 io2 ... cpuN
 * Les lignes commençant par '#' sont ignorées (commentaires).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

/* ========================================================================
 * Fonctions statiques internes
 * ======================================================================== */

/**
 * @brief Comparateur pour qsort : tri par temps d'arrivée croissant.
 * 
 * @param a Pointeur vers le premier Process.
 * @param b Pointeur vers le second Process.
 * @return int Négatif si a arrive avant b, positif sinon.
 */
static int compare_arrival(const void *a, const void *b) {
    Process *pa = (Process *)a;
    Process *pb = (Process *)b;
    return pa->arrival_time - pb->arrival_time;
}

/* ========================================================================
 * Fonction publique
 * ======================================================================== */

/**
 * @brief Lit un fichier texte et construit un tableau de processus.
 * 
 * Le fichier doit contenir une ligne par processus. Les champs sont :
 *   PID, temps_arrivee, puis une alternance de durées CPU et de durées E/S.
 * Exemple : 1 0 5 2 3    (PID=1, arrive à 0, CPU=5, E/S=2, CPU=3)
 * 
 * @param filename Chemin du fichier à lire.
 * @param count    Pointeur vers un entier qui recevra le nombre de processus lus.
 * @return Process* Tableau alloué dynamiquement (à libérer avec free_processes()),
 *         ou NULL en cas d'erreur.
 * 
 * @note Les processus sont triés par temps d'arrivée après lecture.
 * @note Les champs internes (remaining_burst, response_time, etc.) sont initialisés.
 */
Process* read_processes_from_file(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur d'ouverture");
        return NULL;
    }
    
    /* ---------- Première passe : compter les lignes de données ---------- */
    int lines = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        /* Ignorer les lignes vides et les commentaires */
        if (buffer[0] != '#' && strlen(buffer) > 1) {
            lines++;
        }
    }
    
    *count = lines;
    if (lines == 0) {
        fclose(file);
        return NULL;
    }
    
    /* Allocation du tableau de processus */
    Process *processes = malloc(lines * sizeof(Process));
    if (!processes) {
        fclose(file);
        return NULL;
    }
    
    /* ---------- Seconde passe : lecture des données ---------- */
    rewind(file);
    int index = 0;
    while (fgets(buffer, sizeof(buffer), file) && index < lines) {
        /* Ignorer les commentaires et lignes vides */
        if (buffer[0] == '#' || strlen(buffer) <= 1) continue;
        
        /* Découpage de la ligne en entiers */
        int values[100];
        int val_count = 0;
        char *token = strtok(buffer, " \t\n");
        while (token && val_count < 100) {
            values[val_count++] = atoi(token);
            token = strtok(NULL, " \t\n");
        }
        
        /* Sécurité : au moins PID, arrival_time et un burst CPU */
        if (val_count < 3) continue;
        
        Process *p = &processes[index];
        p->pid = values[0];
        p->arrival_time = values[1];
        
        /* Calcul du nombre de bursts CPU : (nb_total_champs - 2 + 1)/2
         * car la séquence est : cpu1, io1, cpu2, io2, ..., cpuN
         * Exemple avec 3 bursts : cpu1 io1 cpu2 io2 cpu3 (5 champs après PID+arrival)
         */
        p->num_bursts = (val_count - 2 + 1) / 2;
        
        /* Allocation des tableaux CPU bursts et IO bursts */
        p->cpu_bursts = malloc(p->num_bursts * sizeof(int));
        p->io_bursts = malloc((p->num_bursts - 1) * sizeof(int));
        
        /* Remplissage des bursts à partir de l'indice 2 (après PID et arrival) */
        int idx = 2;
        for (int i = 0; i < p->num_bursts; i++) {
            p->cpu_bursts[i] = values[idx++];
            if (i < p->num_bursts - 1) {
                p->io_bursts[i] = values[idx++];
            }
        }
        
        /* Initialisation des champs d'état pour la simulation */
        p->current_burst_index = 0;
        p->remaining_burst = p->cpu_bursts[0];
        p->total_wait_time = 0;
        p->response_time = -1;   /* -1 signifie "pas encore commencé" */
        p->finish_time = 0;
        p->state = NEW;
        
        index++;
    }
    
    fclose(file);
    
    /* Tri final par temps d'arrivée (nécessaire pour la simulation événementielle) */
    qsort(processes, *count, sizeof(Process), compare_arrival);
    
    return processes;
}
