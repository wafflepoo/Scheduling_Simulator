/**
 * @file sjrf_select.c
 * @brief Implémentation de la politique SRJF (Shortest Remaining Job First) préemptive
 * @authors MISSAOUI Alissa (50%), TAKKA Kamelia (0%), HORNUNG Thomas 50%)
 * @date 2026-04-02
 * 
 * Cette politique sélectionne le processus ayant le temps restant (remaining_burst)
 * le plus court. Elle est préemptive : à chaque événement (arrivée, fin d'E/S,
 * fin de burst), le simulateur compare le processus courant avec le meilleur
 * candidat de la file READY et préempte si nécessaire.
 */

#include "scheduler.h"

/**
 * @brief Sélectionne le processus avec le plus petit temps restant (remaining_burst).
 * 
 * Parcourt la file des processus prêts et compare leur remaining_burst.
 * Cette fonction est appelée par le simulateur pour déterminer quel processus
 * devrait être exécuté en priorité. La préemption effective est gérée dans
 * la boucle principale de simulate().
 * 
 * @param ready         Tableau de pointeurs vers les processus prêts.
 * @param n             Nombre de processus dans la file.
 * @param quantum       Quantum de temps (inutilisé pour SRJF).
 * @param current_time  Temps courant (inutilisé).
 * @return int          Indice (dans ready) du processus ayant le plus petit remaining_burst.
 */
int sjrf_select(Process **ready, int n, int quantum, int current_time) {
    (void)quantum;        /* Évite un avertissement de paramètre non utilisé */
    (void)current_time;   /* Évite un avertissement de paramètre non utilisé */
    
    int idx = 0;
    int min_rem = ready[0]->remaining_burst;
    
    for (int i = 1; i < n; i++) {
        if (ready[i]->remaining_burst < min_rem) {
            min_rem = ready[i]->remaining_burst;
            idx = i;
        }
    }
    return idx;
}