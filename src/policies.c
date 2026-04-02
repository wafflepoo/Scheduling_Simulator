/**
 * @file policies.c
 * @brief Définition des politiques d'ordonnancement prédéfinies
 * @authors MISSAOUI Alissa (30%), TAKKA Kamelia (40%), HORNUNG Thomas (30%)
 * @date 2026-04-02
 * 
 * Ce fichier instancie les quatre structures SchedPolicy correspondant
 * aux algorithmes demandés : FIFO, SJF, SRJF et Round Robin.
 * Chaque politique associe un nom, une fonction de sélection et (pour RR)
 * un quantum par défaut.
 */

#include "scheduler.h"

/**
 * @brief Politique FIFO (First In, First Out) – non préemptive.
 * 
 * La fonction de sélection fifo_select() retourne toujours l'indice 0,
 * c'est-à-dire le premier processus de la file d'attente.
 * Quantum = 0 (non utilisé).
 */
SchedPolicy FIFO_POLICY = { "FIFO", fifo_select, 0 };

/**
 * @brief Politique SJF (Shortest Job First) – non préemptive.
 * 
 * La fonction sjf_select() choisit le processus ayant le prochain
 * burst CPU le plus court (basé sur cpu_bursts[current_burst_index]).
 * Quantum = 0 (non utilisé).
 */
SchedPolicy SJF_POLICY = { "SJF", sjf_select, 0 };

/**
 * @brief Politique SRJF (Shortest Remaining Job First) – préemptive.
 * 
 * La fonction sjrf_select() sélectionne le processus prêt ayant le
 * remaining_burst (temps restant du burst courant) le plus petit.
 * La préemption est gérée dans simulate().
 * Quantum = 0 (non utilisé).
 */
SchedPolicy SRJF_POLICY = { "SRJF", sjrf_select, 0 };

/**
 * @brief Politique Round Robin – préemptive avec quantum fixe.
 * 
 * La fonction rr_select() retourne toujours 0 (file FIFO), mais la
 * préemption est déclenchée par le quantum dans la boucle principale.
 * Le quantum par défaut est de 2 ms (peut être modifié par l'utilisateur
 * via une copie locale dans main.c).
 */
SchedPolicy RR_POLICY = { "Round Robin", rr_select, 2 };