/**
 * @file sjf_select.c
 * @brief Implémentation de la politique SJF (Shortest Job First) non préemptive
 * @authors MISSAOUI Alissa (100%), TAKKA Kamelia (0%), HORNUNG Thomas (100%)
 * @date 2026-04-02
 * 
 * Cette politique sélectionne le processus ayant le prochain burst CPU
 * le plus court. Elle est non préemptive : une fois qu'un processus commence
 * son burst, il l'exécute jusqu'à son terme ou jusqu'à une entrée/sortie.
 */

#include "scheduler.h"

/**
 * @brief Sélectionne le processus avec le plus petit prochain burst CPU.
 * 
 * Parcourt la file des processus prêts et compare la durée du burst CPU
 * courant (cpu_bursts[current_burst_index]) pour chaque processus.
 * 
 * @param ready         Tableau de pointeurs vers les processus prêts.
 * @param n             Nombre de processus dans la file.
 * @param quantum       Quantum de temps (inutilisé pour SJF).
 * @param current_time  Temps courant (inutilisé).
 * @return int          Indice (dans ready) du processus ayant le plus petit burst.
 */
int sjf_select(Process **ready, int n, int quantum, int current_time) {
    (void)quantum;        /* Évite un avertissement de paramètre non utilisé */
    (void)current_time;   /* Évite un avertissement de paramètre non utilisé */
    
    int idx = 0;
    int min_burst = ready[0]->cpu_bursts[ready[0]->current_burst_index];
    
    for (int i = 1; i < n; i++) {
        int b = ready[i]->cpu_bursts[ready[i]->current_burst_index];
        if (b < min_burst) {
            min_burst = b;
            idx = i;
        }
    }
    return idx;
}
