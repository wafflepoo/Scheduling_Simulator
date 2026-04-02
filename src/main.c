/**
 * @file main.c
 * @brief Interface utilisateur interactive du simulateur d'ordonnancement
 * @authors MISSAOUI Alissa (100%), TAKKA Kamelia (0%), HORNUNG Thomas (0%)
 * @date 2026-04-02
 * 
 * Ce fichier contient le menu principal, la gestion des entrées utilisateur,
 * le chargement des processus (fichier ou saisie manuelle), la sélection
 * de l'algorithme, le lancement de la simulation, l'export CSV et l'appel
 * au script Python de visualisation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

/* Inclusion pour la création de répertoire (mkdir) */
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir(p, 0755)
#endif

/* Déclarations externes des politiques d'ordonnancement (définies dans policies.c) */
extern SchedPolicy FIFO_POLICY;
extern SchedPolicy SJF_POLICY;
extern SchedPolicy SRJF_POLICY;
extern SchedPolicy RR_POLICY;

/* Codes ANSI pour la coloration dans le terminal */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

/* Variables globales (état de la session) */
static Process *loaded_processes = NULL;   /**< Tableau des processus chargés */
static int process_count = 0;              /**< Nombre de processus chargés */
static SchedPolicy *current_policy = NULL; /**< Politique d'ordonnancement sélectionnée */

/* Prototypes des fonctions internes (définies plus bas) */
void clear_screen(void);
void print_header(const char *title);
void print_menu(void);
void load_processes_interactive(void);
void load_processes_manual(void);
void select_algorithm_interactive(void);
void run_simulation_interactive(void);
void export_results_interactive(void);
void visualize_with_matplotlib(void);
void wait_for_enter(void);
void flush_stdin(void);

/* ========================================================================
 * Point d'entrée principal
 * ======================================================================== */

/**
 * @brief Programme principal – boucle interactive du menu.
 * 
 * Initialise l'interface, puis répète l'affichage du menu et le traitement
 * du choix utilisateur jusqu'à la sélection de l'option Quitter (7).
 * 
 * @return int Code de retour (0 = succès).
 */
int main(void) {
    clear_screen();
    print_header("SIMULATEUR D'ORDONNANCEMENT DE PROCESSUS");

    int choice;
    do {
        print_menu();
        printf(BOLD "Votre choix : " RESET);
        if (scanf("%d", &choice) != 1) {
            flush_stdin();
            choice = 0;
        } else {
            flush_stdin();
        }

        switch (choice) {
            case 1: load_processes_interactive(); break;
            case 2: load_processes_manual(); break;
            case 3: select_algorithm_interactive(); break;
            case 4: run_simulation_interactive(); break;
            case 5: export_results_interactive(); break;
            case 6: visualize_with_matplotlib(); break;
            case 7: printf(GREEN "\nAu revoir !\n" RESET); break;
            default:
                printf(RED "Choix invalide.\n" RESET);
                wait_for_enter();
        }
    } while (choice != 7);

    /* Libération de la mémoire avant de quitter */
    if (loaded_processes != NULL) {
        free_processes(loaded_processes, process_count);
        loaded_processes = NULL;
    }
    return 0;
}

/* ========================================================================
 * Fonctions utilitaires (entrée/sortie, affichage)
 * ======================================================================== */

/**
 * @brief Vide le tampon d'entrée standard (supprime les caractères résiduels).
 * 
 * Utilisé après scanf pour éviter qu'un '\n' restant ne perturbe la saisie suivante.
 */
void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * @brief Efface l'écran du terminal (commande ANSI).
 */
void clear_screen(void) {
    printf("\033[2J\033[1;1H");
}

/**
 * @brief Affiche un en-tête encadré en couleur.
 * 
 * @param title Texte à afficher au centre de l'en-tête.
 */
void print_header(const char *title) {
    printf(BOLD CYAN "+------------------------------------------------------------+\n");
    printf("|  %-50s  |\n", title);
    printf("+------------------------------------------------------------+\n\n" RESET);
}

/**
 * @brief Affiche le menu principal avec les options colorées.
 */
void print_menu(void) {
    printf(BOLD "MENU PRINCIPAL\n" RESET);
    printf("1. " GREEN "Charger un fichier de processus\n" RESET);
    printf("2. " GREEN "Saisie manuelle des processus\n" RESET);
    printf("3. " YELLOW "Choisir l'algorithme d'ordonnancement\n" RESET);
    printf("4. " BLUE "Lancer la simulation\n" RESET);
    printf("5. " MAGENTA "Exporter les résultats en CSV\n" RESET);
    printf("6. " CYAN "Visualiser avec matplotlib (Python)\n" RESET);
    printf("7. " RED "Quitter\n" RESET);
}

/* ========================================================================
 * Chargement des processus
 * ======================================================================== */

/**
 * @brief Charge des processus à partir d'un fichier texte.
 * 
 * Demande le chemin du fichier, appelle read_processes_from_file(),
 * puis affiche la liste des processus chargés.
 */
void load_processes_interactive(void) {
    clear_screen();
    print_header("CHARGEMENT DES PROCESSUS (FICHIER)");

    char filename[256];
    printf("Entrez le chemin du fichier de processus : ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf(RED "Erreur de lecture.\n" RESET);
        wait_for_enter();
        return;
    }
    filename[strcspn(filename, "\n")] = '\0';

    /* Libération de l'ancien jeu de processus s'il existe */
    if (loaded_processes != NULL) {
        free_processes(loaded_processes, process_count);
        loaded_processes = NULL;
    }

    loaded_processes = read_processes_from_file(filename, &process_count);
    if (loaded_processes == NULL) {
        printf(RED "Échec du chargement. Vérifiez le fichier.\n" RESET);
    } else {
        printf(GREEN "✓ %d processus chargés.\n" RESET, process_count);
        printf("\n" BOLD "Liste des processus :\n" RESET);
        printf("PID\tArrivée\tBursts CPU (IO)\n");
        for (int i = 0; i < process_count; i++) {
            Process *p = &loaded_processes[i];
            printf("%d\t%d\t", p->pid, p->arrival_time);
            for (int j = 0; j < p->num_bursts; j++) {
                printf("%d", p->cpu_bursts[j]);
                if (j < p->num_bursts - 1)
                    printf(" (IO:%d) ", p->io_bursts[j]);
            }
            printf("\n");
        }
    }
    wait_for_enter();
}

/**
 * @brief Saisie interactive manuelle des processus.
 * 
 * L'utilisateur entre le nombre de processus, puis pour chacun :
 *   - temps d'arrivée
 *   - nombre de bursts CPU
 *   - pour chaque burst : durée CPU, puis durée E/S (sauf après le dernier burst)
 * 
 * Les processus sont stockés dans la variable globale loaded_processes.
 */
void load_processes_manual(void) {
    clear_screen();
    print_header("SAISIE MANUELLE DES PROCESSUS");

    int n;
    printf("Nombre de processus à saisir : ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        flush_stdin();
        printf(RED "Nombre invalide.\n" RESET);
        wait_for_enter();
        return;
    }
    flush_stdin();

    Process *proc = malloc(n * sizeof(Process));
    if (!proc) {
        printf(RED "Erreur d'allocation mémoire.\n" RESET);
        wait_for_enter();
        return;
    }

    for (int i = 0; i < n; i++) {
        printf("\n--- Processus %d ---\n", i+1);
        proc[i].pid = i+1;
        
        printf("Temps d'arrivée (ms) : ");
        if (scanf("%d", &proc[i].arrival_time) != 1) {
            flush_stdin();
            printf(RED "Erreur de saisie.\n" RESET);
            free(proc);
            wait_for_enter();
            return;
        }
        flush_stdin();

        printf("Nombre de bursts CPU : ");
        int nb;
        if (scanf("%d", &nb) != 1 || nb <= 0) {
            flush_stdin();
            printf(RED "Nombre de bursts invalide.\n" RESET);
            free(proc);
            wait_for_enter();
            return;
        }
        flush_stdin();
        proc[i].num_bursts = nb;

        /* Allocation des tableaux CPU bursts et IO bursts */
        proc[i].cpu_bursts = malloc(nb * sizeof(int));
        proc[i].io_bursts = malloc((nb - 1) * sizeof(int));
        if (!proc[i].cpu_bursts || (nb > 1 && !proc[i].io_bursts)) {
            printf(RED "Erreur d'allocation.\n" RESET);
            free(proc[i].cpu_bursts);
            free(proc[i].io_bursts);
            free(proc);
            wait_for_enter();
            return;
        }

        for (int j = 0; j < nb; j++) {
            printf("  Durée du burst CPU %d : ", j+1);
            if (scanf("%d", &proc[i].cpu_bursts[j]) != 1 || proc[i].cpu_bursts[j] <= 0) {
                flush_stdin();
                printf(RED "Durée invalide.\n" RESET);
                free(proc[i].cpu_bursts);
                free(proc[i].io_bursts);
                free(proc);
                wait_for_enter();
                return;
            }
            flush_stdin();
            
            if (j < nb - 1) {
                printf("  Durée de l'E/S après ce burst : ");
                if (scanf("%d", &proc[i].io_bursts[j]) != 1 || proc[i].io_bursts[j] <= 0) {
                    flush_stdin();
                    printf(RED "Durée d'E/S invalide.\n" RESET);
                    free(proc[i].cpu_bursts);
                    free(proc[i].io_bursts);
                    free(proc);
                    wait_for_enter();
                    return;
                }
                flush_stdin();
            }
        }

        /* Initialisation des champs dynamiques (état, temps restant, ...) */
        proc[i].current_burst_index = 0;
        proc[i].remaining_burst = proc[i].cpu_bursts[0];
        proc[i].total_wait_time = 0;
        proc[i].response_time = -1;
        proc[i].finish_time = 0;
        proc[i].state = NEW;
    }

    /* Remplacement de l'ancien jeu de processus */
    if (loaded_processes != NULL) {
        free_processes(loaded_processes, process_count);
        loaded_processes = NULL;
    }

    loaded_processes = proc;
    process_count = n;

    printf(GREEN "\n✓ %d processus saisis avec succès.\n" RESET, n);
    printf("\n" BOLD "Liste des processus :\n" RESET);
    printf("PID\tArrivée\tBursts CPU (IO)\n");
    for (int i = 0; i < process_count; i++) {
        Process *p = &loaded_processes[i];
        printf("%d\t%d\t", p->pid, p->arrival_time);
        for (int j = 0; j < p->num_bursts; j++) {
            printf("%d", p->cpu_bursts[j]);
            if (j < p->num_bursts - 1)
                printf(" (IO:%d) ", p->io_bursts[j]);
        }
        printf("\n");
    }

    wait_for_enter();
}

/* ========================================================================
 * Sélection de l'algorithme
 * ======================================================================== */

/**
 * @brief Menu interactif pour choisir l'algorithme d'ordonnancement.
 * 
 * Propose FIFO, SJF, SRJF et Round Robin (avec quantum personnalisable).
 * Met à jour la variable globale current_policy.
 */
void select_algorithm_interactive(void) {
    clear_screen();
    print_header("CHOIX DE L'ALGORITHME");

    printf("1. FIFO (First In, First Out)\n");
    printf("2. SJF (Shortest Job First) - non préemptif\n");
    printf("3. SRJF (Shortest Remaining Job First) - préemptif\n");
    printf("4. Round Robin (avec quantum modifiable)\n");
    printf("Choisissez (1-4) : ");

    int algo;
    if (scanf("%d", &algo) != 1) {
        flush_stdin();
        printf(RED "Entrée invalide.\n" RESET);
        wait_for_enter();
        return;
    }
    flush_stdin();

    switch (algo) {
        case 1:
            current_policy = &FIFO_POLICY;
            printf(GREEN "FIFO sélectionné.\n" RESET);
            break;
        case 2:
            current_policy = &SJF_POLICY;
            printf(GREEN "SJF sélectionné.\n" RESET);
            break;
        case 3:
            current_policy = &SRJF_POLICY;
            printf(GREEN "SRJF sélectionné.\n" RESET);
            break;
        case 4: {
            int quantum;
            printf("Entrez le quantum (ms) : ");
            if (scanf("%d", &quantum) != 1 || quantum <= 0) {
                flush_stdin();
                quantum = 2;
                printf(RED "Quantum invalide, 2 ms par défaut.\n" RESET);
            }
            flush_stdin();
            /* Variable statique pour conserver la politique personnalisée */
            static SchedPolicy rr_custom;
            rr_custom = RR_POLICY;
            rr_custom.quantum = quantum;
            current_policy = &rr_custom;
            printf(GREEN "Round Robin avec quantum = %d ms.\n" RESET, quantum);
            break;
        }
        default:
            printf(RED "Choix invalide.\n" RESET);
            wait_for_enter();
            return;
    }
    wait_for_enter();
}

/* ========================================================================
 * Simulation et export
 * ======================================================================== */

/**
 * @brief Lance la simulation avec les processus et l'algorithme courants.
 * 
 * Vérifie qu'un jeu de processus et une politique sont sélectionnés,
 * appelle simulate(), puis affiche un résumé des métriques.
 */
void run_simulation_interactive(void) {
    clear_screen();
    print_header("SIMULATION");

    if (loaded_processes == NULL) {
        printf(RED "Aucun processus chargé.\n" RESET);
        wait_for_enter();
        return;
    }
    if (current_policy == NULL) {
        printf(RED "Aucun algorithme sélectionné.\n" RESET);
        wait_for_enter();
        return;
    }

    printf(BOLD "Lancement de la simulation avec %s...\n\n" RESET, current_policy->name);
    ScheduleResult result;
    simulate(loaded_processes, process_count, current_policy, &result);

    printf("\n" BOLD CYAN "Résumé :\n" RESET);
    printf("Temps d'attente moyen : %.2f ms\n", result.avg_wait_time);
    printf("Turnaround moyen      : %.2f ms\n", result.avg_turnaround_time);
    printf("Temps de réponse moyen: %.2f ms\n", result.avg_response_time);
    printf("Utilisation CPU       : %.2f %%\n", result.cpu_utilization);
    wait_for_enter();
}

/**
 * @brief Exporte les résultats de la simulation dans des fichiers CSV.
 * 
 * Crée le dossier results/ (si nécessaire), lance la simulation,
 * puis exporte les métriques (results_*.csv) et la timeline (timeline_*.csv).
 */
void export_results_interactive(void) {
    clear_screen();
    print_header("EXPORT CSV");

    if (loaded_processes == NULL || current_policy == NULL) {
        printf(RED "Chargez d'abord des processus et choisissez un algorithme.\n" RESET);
        wait_for_enter();
        return;
    }

    MKDIR("results");
    ScheduleResult result;
    simulate(loaded_processes, process_count, current_policy, &result);

    printf(GREEN "\nFichiers exportés :\n");
    printf("  results/results_%s.csv\n", current_policy->name);
    printf("  results/timeline_%s.csv\n" RESET, current_policy->name);
    wait_for_enter();
}

/**
 * @brief Génère et affiche les graphiques via le script Python matplotlib.
 * 
 * Lance la simulation pour produire les CSV, puis appelle plot_gantt.py
 * avec l'option --show pour afficher les quatre graphiques.
 */
void visualize_with_matplotlib(void) {
    if (loaded_processes == NULL || current_policy == NULL) {
        printf(RED "Veuillez d'abord charger des processus et choisir un algorithme.\n" RESET);
        wait_for_enter();
        return;
    }

    /* Génération des CSV par la simulation */
    ScheduleResult result;
    simulate(loaded_processes, process_count, current_policy, &result);

    /* Appel du script Python */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "python plot_gantt.py \"%s\" --show", current_policy->name);
    printf("\nExécution de : %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        printf(RED "Erreur lors de l'exécution du script Python.\n" RESET);
        printf(YELLOW "Vérifiez que Python et les bibliothèques (matplotlib, pandas) sont installés.\n" RESET);
        printf(YELLOW "   pip install matplotlib pandas\n" RESET);
    } else {
        printf(GREEN "Tous les graphiques ont été générés dans le dossier results/\n" RESET);
    }
    wait_for_enter();
}

/**
 * @brief Attend que l'utilisateur appuie sur Entrée.
 * 
 * Affiche un message puis vide le tampon d'entrée.
 */
void wait_for_enter(void) {
    printf("\n" BOLD "Appuyez sur Entrée pour continuer..." RESET);
    flush_stdin();
}