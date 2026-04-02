/**
 * @file utils.c
 * @brief Fonctions utilitaires : libération mémoire et export CSV
 * @authors MISSAOUI Alissa (20%), TAKKA Kamelia (40%), HORNUNG Thomas (40%)
 * @date 2026-04-02
 * 
 * Ce module fournit deux fonctions essentielles :
 *   - free_processes() : libère la mémoire allouée pour un tableau de processus
 *   - export_to_csv() : sauvegarde les résultats de simulation dans un fichier CSV
 */

#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

/**
 * @brief Libère la mémoire allouée dynamiquement pour un tableau de processus.
 * 
 * Pour chaque processus, libère les tableaux cpu_bursts et io_bursts,
 * puis libère le tableau de processus lui-même.
 * 
 * @param processes Tableau de processus à libérer.
 * @param count     Nombre de processus dans le tableau.
 */
void free_processes(Process *processes, int count) {
    for (int i = 0; i < count; i++) {
        free(processes[i].cpu_bursts);
        free(processes[i].io_bursts);
    }
    free(processes);
}

/**
 * @brief Exporte les résultats de la simulation dans un fichier CSV.
 * 
 * Le fichier généré contient :
 *   - Une ligne d'en-tête : PID, Response Time, Wait Time, Turnaround Time
 *   - Une ligne par processus avec ses métriques individuelles
 *   - Une ligne commentée (#Moyennes) avec les moyennes des métriques
 *   - Une ligne commentée (#CPU Utilization) avec le taux d'occupation CPU
 * 
 * @param filename  Chemin du fichier CSV à créer.
 * @param result    Structure contenant les moyennes et l'utilisation CPU.
 * @param processes Tableau des processus (après simulation).
 * @param count     Nombre de processus.
 */
void export_to_csv(const char *filename, ScheduleResult *result, Process *processes, int count) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erreur création fichier CSV");
        return;
    }
    
    /* En-tête des colonnes */
    fprintf(file, "PID,Response Time,Wait Time,Turnaround Time\n");
    
    /* Données par processus */
    for (int i = 0; i < count; i++) {
        Process *p = &processes[i];
        fprintf(file, "%d,%d,%d,%d\n",
                p->pid,
                p->response_time,
                p->total_wait_time,
                p->finish_time - p->arrival_time);
    }
    
    /* Lignes de synthèse (commentées pour faciliter l'import dans un tableur) */
    fprintf(file, "#Moyennes,%.2f,%.2f,%.2f\n",
            result->avg_response_time,
            result->avg_wait_time,
            result->avg_turnaround_time);
    fprintf(file, "#CPU Utilization,%.2f\n", result->cpu_utilization);
    
    fclose(file);
    printf("Résultats exportés dans %s\n", filename);
}