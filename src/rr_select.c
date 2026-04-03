/**
 * @file rr_select.c
 * @brief Implémentation de la politique Round Robin (tourniquet)
 * @authors MISSAOUI Alissa (0%), TAKKA Kamelia (100%), HORNUNG Thomas (0%)
 * @date 2026-04-02
 * 
 * Cette politique utilise une file FIFO. Le quantum est géré dans le simulateur
 * (simulator.c) ; la fonction de sélection se contente de retourner le premier
 * processus de la file.
 */

#include "scheduler.h"

/**
 * @brief Sélectionne le prochain processus pour l'algorithme Round Robin.
 * 
 * Round Robin utilise un ordre circulaire : le processus suivant est toujours
 * le premier de la file d'attente. La préemption par quantum est gérée par
 * la boucle principale de simulation (simulator.c), pas par cette fonction.
 * 
 * @param ready         Tableau de pointeurs vers les processus prêts.
 * @param n             Nombre de processus dans la file (inutilisé).
 * @param quantum       Quantum de temps (inutilisé ici, géré ailleurs).
 * @param current_time  Temps courant (inutilisé).
 * @return int          Toujours 0 (indice du premier élément du tableau).
 */
int rr_select(Process **ready, int n, int quantum, int current_time) {
    (void)quantum;        /* Évite un avertissement de paramètre non utilisé */
    (void)current_time;   /* Évite un avertissement de paramètre non utilisé */
    (void)n;              /* Le paramètre n n'est pas nécessaire ici */
    
    /* Round Robin utilise l'ordre FIFO : premier de la file */
    return 0;
}
