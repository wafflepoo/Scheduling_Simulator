/**
 * @file fifo_select.c
 * @brief Implémentation de la politique d'ordonnancement FIFO (First In, First Out)
 * @authors MISSAOUI Alissa (100%), TAKKA Kamelia (0%), HORNUNG Thomas (0%)
 * @date 2026-04-02
 * 
 * Cette politique sélectionne toujours le premier processus de la file d'attente.
 * Elle est non préemptive : un processus s'exécute jusqu'à son achèvement ou
 * jusqu'à une entrée/sortie.
 */

#include "scheduler.h"

/**
 * @brief Sélectionne le prochain processus à exécuter selon l'algorithme FIFO.
 * 
 * @param ready         Tableau de pointeurs vers les processus prêts.
 * @param n             Nombre de processus dans la file (inutilisé).
 * @param quantum       Quantum de temps (inutilisé pour FIFO).
 * @param current_time  Temps courant (inutilisé).
 * @return int          Toujours 0 (indice du premier élément du tableau).
 * 
 * L'algorithme FIFO se contente de prendre le processus arrivé le plus tôt,
 * c'est-à-dire le premier élément du tableau `ready` (qui est maintenu dans
 * l'ordre d'arrivée par le simulateur).
 */
int fifo_select(Process **ready, int n, int quantum, int current_time) {
    (void)quantum;        /* Évite un avertissement de paramètre non utilisé */
    (void)current_time;   /* Évite un avertissement de paramètre non utilisé */
    (void)n;              /* Le paramètre n n'est pas nécessaire ici */
    
    /* Retourne l'indice du premier processus de la file (ordre FIFO) */
    return 0;
}