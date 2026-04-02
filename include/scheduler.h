/**
 * @file scheduler.h
 * @brief Définitions des structures, énumérations et prototypes du simulateur
 * @authors MISSAOUI Alissa (40%), TAKKA Kamelia (30%), HORNUNG Thomas (30%)
 * @date 2026-04-02
 * 
 * Ce fichier d'en-tête contient toutes les déclarations communes au simulateur
 * d'ordonnancement de processus. Il définit :
 *   - Les états d'un processus (enum ProcessState)
 *   - La structure Process (description d'un processus)
 *   - La structure ScheduleResult (résultats de simulation)
 *   - La structure SchedPolicy (politique d'ordonnancement)
 *   - Les prototypes des fonctions de sélection et du simulateur
 *   - Les prototypes des fonctions utilitaires
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <limits.h>

/* ========================================================================
 * Énumérations
 * ======================================================================== */

/**
 * @brief États possibles d'un processus au cours de la simulation.
 */
typedef enum {
    NEW,        /**< Processus nouvellement créé, pas encore dans la file READY */
    READY,      /**< Processus prêt à être exécuté (dans la file d'attente) */
    RUNNING,    /**< Processus en cours d'exécution sur le CPU */
    WAITING,    /**< Processus en attente d'une opération d'E/S */
    TERMINATED  /**< Processus terminé */
} ProcessState;

/* ========================================================================
 * Structures principales
 * ======================================================================== */

/**
 * @brief Structure représentant un processus.
 * 
 * Contient toutes les informations statiques (PID, arrivée, bursts)
 * et dynamiques (état, temps d'attente, temps restant, etc.).
 */
typedef struct {
    /* --- Données statiques (lues depuis le fichier ou saisie) --- */
    int pid;                /**< Identifiant unique du processus */
    int arrival_time;       /**< Temps d'arrivée dans le système (ms) */
    int *cpu_bursts;        /**< Tableau des durées des bursts CPU (ms) */
    int *io_bursts;         /**< Tableau des durées des E/S (ms), taille = num_bursts - 1 */
    int num_bursts;         /**< Nombre de bursts CPU (et donc nombre de phases CPU) */
    
    /* --- Données dynamiques (évoluent pendant la simulation) --- */
    int current_burst_index;    /**< Index du burst CPU en cours (0..num_bursts-1) */
    int start_time;              /**< Instant de premier démarrage (non utilisé) */
    int finish_time;             /**< Instant de terminaison du processus */
    int total_wait_time;         /**< Temps total passé dans la file READY (ms) */
    int response_time;           /**< Délai entre arrival_time et premier accès CPU (ms) */
    int remaining_burst;         /**< Temps restant à exécuter pour le burst CPU courant */
    int last_exec_time;          /**< Dernier instant d'exécution (non utilisé) */
    ProcessState state;          /**< État courant du processus (NEW, READY, etc.) */
} Process;

/**
 * @brief Structure regroupant les résultats agrégés de la simulation.
 */
typedef struct {
    float avg_wait_time;        /**< Temps d'attente moyen (ms) */
    float avg_turnaround_time;  /**< Turnaround moyen (ms) */
    float avg_response_time;    /**< Temps de réponse moyen (ms) */
    float cpu_utilization;      /**< Taux d'occupation du CPU (%) */
    int *wait_times;            /**< Tableau des temps d'attente par processus (non utilisé) */
    int *turnaround_times;      /**< Tableau des turnaround par processus (non utilisé) */
    int *response_times;        /**< Tableau des temps de réponse par processus (non utilisé) */
} ScheduleResult;

/* ========================================================================
 * Politiques d'ordonnancement
 * ======================================================================== */

/**
 * @brief Structure décrivant une politique d'ordonnancement.
 * 
 * Associe un nom, une fonction de sélection et éventuellement un quantum
 * (pour Round Robin). L'utilisation d'un pointeur de fonction permet
 * d'ajouter facilement de nouvelles politiques sans modifier le simulateur.
 */
typedef struct {
    const char *name;   /**< Nom de la politique (ex: "FIFO", "Round Robin") */
    /**
     * @brief Fonction de sélection du prochain processus à exécuter.
     * 
     * @param ready_queue   Tableau de pointeurs vers les processus prêts.
     * @param ready_count   Nombre de processus dans le tableau.
     * @param quantum       Quantum (pour RR, sinon 0).
     * @param current_time  Temps courant de la simulation.
     * @return int          Indice du processus sélectionné dans ready_queue.
     */
    int (*select_next)(Process **ready_queue, int ready_count, int quantum, int current_time);
    int quantum;        /**< Quantum de temps pour Round Robin (0 pour les autres) */
} SchedPolicy;

/* ========================================================================
 * Déclarations externes des politiques prédéfinies
 * ======================================================================== */

extern SchedPolicy FIFO_POLICY;   /**< Politique First In, First Out */
extern SchedPolicy SJF_POLICY;    /**< Politique Shortest Job First (non préemptif) */
extern SchedPolicy SRJF_POLICY;   /**< Politique Shortest Remaining Job First (préemptif) */
extern SchedPolicy RR_POLICY;     /**< Politique Round Robin (quantum = 2 par défaut) */

/* ========================================================================
 * Prototypes des fonctions de sélection (implémentées dans *_select.c)
 * ======================================================================== */

int fifo_select(Process **ready, int n, int quantum, int current_time);
int sjf_select(Process **ready, int n, int quantum, int current_time);
int sjrf_select(Process **ready, int n, int quantum, int current_time);
int rr_select(Process **ready, int n, int quantum, int current_time);

/* ========================================================================
 * Prototypes des fonctions principales
 * ======================================================================== */

/**
 * @brief Lance la simulation avec une politique donnée.
 * 
 * @param processes Tableau des processus (non modifié).
 * @param count     Nombre de processus.
 * @param policy    Politique d'ordonnancement à utiliser.
 * @param result    Structure qui recevra les métriques de performance.
 */
void simulate(Process *processes, int count, SchedPolicy *policy, ScheduleResult *result);

/* ========================================================================
 * Prototypes des fonctions utilitaires
 * ======================================================================== */

/**
 * @brief Lit un fichier texte et construit un tableau de processus.
 * 
 * @param filename Chemin du fichier.
 * @param count    Pointeur vers un entier qui recevra le nombre de processus.
 * @return Process* Tableau alloué dynamiquement (NULL en cas d'erreur).
 */
Process* read_processes_from_file(const char *filename, int *count);

/**
 * @brief Exporte les résultats dans un fichier CSV.
 * 
 * @param filename  Nom du fichier de sortie.
 * @param result    Structure contenant les moyennes et l'utilisation CPU.
 * @param processes Tableau des processus (après simulation).
 * @param count     Nombre de processus.
 */
void export_to_csv(const char *filename, ScheduleResult *result, Process *processes, int count);

/**
 * @brief Libère la mémoire allouée pour un tableau de processus.
 * 
 * @param processes Tableau de processus.
 * @param count     Nombre de processus.
 */
void free_processes(Process *processes, int count);

/**
 * @brief Exporte la timeline (états par processus et par ms) dans un fichier CSV.
 * 
 * @param filename  Nom du fichier de sortie.
 * @param timeline  Tableau 2D des états (count lignes, max_time colonnes).
 * @param count     Nombre de processus.
 * @param max_time  Nombre de colonnes (durée simulée).
 */
void export_timeline_csv(const char *filename, char **timeline, int count, int max_time);

#endif /* SCHEDULER_H */