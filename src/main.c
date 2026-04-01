#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

// Pour mkdir
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir(p, 0755)
#endif

// Déclarations externes
extern SchedPolicy FIFO_POLICY;
extern SchedPolicy SJF_POLICY;
extern SchedPolicy SRJF_POLICY;
extern SchedPolicy RR_POLICY;

// Couleurs ANSI
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// Variables globales
static Process *loaded_processes = NULL;
static int process_count = 0;
static SchedPolicy *current_policy = NULL;

// Prototypes
void clear_screen(void);
void print_header(const char *title);
void print_menu(void);
void load_processes_interactive(void);
void load_processes_manual(void);          // nouvelle fonction
void select_algorithm_interactive(void);
void run_simulation_interactive(void);
void export_results_interactive(void);
void visualize_with_matplotlib(void);
void wait_for_enter(void);
void flush_stdin(void);

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
            case 2: load_processes_manual(); break;            // saisie manuelle
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

    if (loaded_processes != NULL) {
        free_processes(loaded_processes, process_count);
        loaded_processes = NULL;
    }
    return 0;
}

// --- Fonctions utilitaires ---
void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void clear_screen(void) {
    printf("\033[2J\033[1;1H");
}

void print_header(const char *title) {
    printf(BOLD CYAN "╔══════════════════════════════════════════════════════════╗\n");
    printf("║  %-50s  ║\n", title);
    printf("╚══════════════════════════════════════════════════════════╝\n\n" RESET);
}

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

// --- Chargement depuis fichier ---
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

// --- Saisie manuelle des processus ---
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

        // Initialisation des champs internes
        proc[i].current_burst_index = 0;
        proc[i].remaining_burst = proc[i].cpu_bursts[0];
        proc[i].total_wait_time = 0;
        proc[i].response_time = -1;
        proc[i].finish_time = 0;
        proc[i].state = NEW;
    }

    // Libération de l'ancien jeu de processus
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

// --- Les autres fonctions (choix algorithme, simulation, export, visualisation) ---
// (Elles restent inchangées, je les recopie pour mémoire)

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
        case 1: current_policy = &FIFO_POLICY; printf(GREEN "FIFO sélectionné.\n" RESET); break;
        case 2: current_policy = &SJF_POLICY; printf(GREEN "SJF sélectionné.\n" RESET); break;
        case 3: current_policy = &SRJF_POLICY; printf(GREEN "SRJF sélectionné.\n" RESET); break;
        case 4: {
            int quantum;
            printf("Entrez le quantum (ms) : ");
            if (scanf("%d", &quantum) != 1 || quantum <= 0) {
                flush_stdin();
                quantum = 2;
                printf(RED "Quantum invalide, 2 ms par défaut.\n" RESET);
            }
            flush_stdin();
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

void visualize_with_matplotlib(void) {
    if (loaded_processes == NULL || current_policy == NULL) {
        printf(RED "Veuillez d'abord charger des processus et choisir un algorithme.\n" RESET);
        wait_for_enter();
        return;
    }

    // Exécuter la simulation pour générer les fichiers CSV (timeline)
    ScheduleResult result;
    simulate(loaded_processes, process_count, current_policy, &result);

    char timeline_csv[256];
    snprintf(timeline_csv, sizeof(timeline_csv), "results/timeline_%s.csv", current_policy->name);

    // Nom du fichier de sortie PNG
    char output_png[256];
    snprintf(output_png, sizeof(output_png), "results/gantt_%s.png", current_policy->name);

    // Chemin du script Python (supposé dans le répertoire courant)
    const char *script_path = "plot_gantt.py";

    // Construction de la commande : python script.py timeline.csv output.png
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "python \"%s\" \"%s\" \"%s\"", script_path, timeline_csv, output_png);

    printf("\nExécution de : %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        printf(RED "Erreur lors de l'exécution du script Python.\n" RESET);
        printf(YELLOW "Vérifiez que Python et les bibliothèques (matplotlib, pandas) sont installés.\n" RESET);
        printf(YELLOW "   pip install matplotlib pandas\n" RESET);
    } else {
        printf(GREEN "Diagramme de Gantt généré : %s\n" RESET, output_png);
    }
    wait_for_enter();
}

void wait_for_enter(void) {
    printf("\n" BOLD "Appuyez sur Entrée pour continuer..." RESET);
    flush_stdin();
}