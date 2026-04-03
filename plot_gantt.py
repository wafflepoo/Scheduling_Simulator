#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
@file plot_gantt.py
@brief Génération de graphiques à partir des résultats de simulation
@authors MISSAOUI Alissa (80%), TAKKA Kamelia (20%), HORNUNG Thomas (0%)
@date 2026-04-02
@version 1.0

Ce module lit les fichiers CSV produits par le simulateur C et génère quatre types
de graphiques :
    - Diagramme de Gantt (états CPU/E/S/Attente par processus)
    - Diagramme à barres comparatif (temps de réponse, attente, turnaround)
    - Diagramme radar (performances globales normalisées)
    - Occupation CPU dans le temps

Les graphiques sont sauvegardés dans le dossier `results/` sous forme d'images PNG.
"""

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
import sys
import os


# =============================================================================
# Constantes globales
# =============================================================================

# Couleurs adaptées aux daltoniens (palette ColorBrewer Set2)
COLORS = {
    'U': '#4c72b0',      # Bleu : processus en exécution CPU
    'W': '#dd8452',      # Orange : processus en attente
    'O': '#55a868',      # Vert : processus en entrée/sortie
    '.': '#e5e5e5',      # Gris clair : inactif (pas encore arrivé ou terminé)
    'resp': '#66c2a5',   # Turquoise : temps de réponse
    'wait': '#fc8d62',   # Orange clair : temps d'attente
    'turn': '#8da0cb',   # Bleu clair : turnaround
}


# =============================================================================
# Fonctions de tracé
# =============================================================================

def plot_gantt(timeline_file, output_png):
    """
    Génère un diagramme de Gantt à partir du fichier de timeline.

    Chaque ligne représente un processus. Pour chaque milliseconde,
    une couleur et une lettre (U, W, O, .) indiquent l'état.

    @param timeline_file (str) Chemin du fichier CSV contenant la timeline.
    @param output_png (str or None) Chemin du fichier PNG à sauvegarder.
                                      Si None, la figure n'est pas sauvegardée.
    @return matplotlib.figure.Figure ou None en cas d'échec.
    """
    # Vérification de l'existence du fichier
    if not os.path.exists(timeline_file):
        print(f"Fichier timeline introuvable : {timeline_file}")
        return None

    # Chargement des données
    df = pd.read_csv(timeline_file)
    time_cols = df.columns[1:]          # Colonnes des temps (0,1,2,...)
    max_time = len(time_cols)           # Durée totale simulée
    nb_proc = len(df)                   # Nombre de processus

    # Dimensions adaptatives de la figure
    fig_width = max(10, max_time * 0.2)
    fig_height = max(2, nb_proc * 0.6)
    fig, ax = plt.subplots(figsize=(fig_width, fig_height))

    # Parcours de chaque processus et de chaque instant
    for i, row in df.iterrows():
        pid = row['PID']
        for t, state in enumerate(row[1:]):
            # Barre horizontale de largeur 1 à la position t
            ax.barh(y=i, width=1, left=t,
                    color=COLORS.get(state, '#000000'),
                    edgecolor='black', linewidth=0.5)
            # Affichage de la lettre dans la barre
            ax.text(t + 0.5, i, state, ha='center', va='center',
                    color='white', fontsize=8, fontweight='bold')

    # Configuration des axes
    ax.set_yticks(range(nb_proc))
    ax.set_yticklabels(df['PID'])
    ax.set_xlim(0, max_time)
    ax.set_xticks(range(0, max_time+1, max(1, max_time//20)))
    ax.set_xlabel('Temps (ms)')
    ax.set_title(f'Diagramme de Gantt - {os.path.basename(timeline_file).replace("timeline_","").replace(".csv","")}')
    ax.invert_yaxis()                     # P1 en haut
    ax.grid(axis='x', linestyle='--', alpha=0.5)

    # Légende
    patches = [mpatches.Patch(color=COLORS[s], label=lab)
               for s, lab in zip(['U','W','O','.'],
                                 ['CPU (U)','Attente (W)','E/S (O)','Inactif'])]
    ax.legend(handles=patches, loc='upper right', bbox_to_anchor=(1.15, 1))

    plt.tight_layout()
    if output_png:
        plt.savefig(output_png, dpi=300)
        print(f"Diagramme de Gantt sauvegardé : {output_png}")
    return fig


def plot_bar_chart(results_csv, output_png):
    """
    Génère un diagramme à barres groupées comparant les métriques par processus.

    @param results_csv (str) Chemin du fichier CSV contenant les résultats.
    @param output_png (str or None) Chemin du fichier PNG de sortie.
    @return matplotlib.figure.Figure ou None.
    """
    if not os.path.exists(results_csv):
        print(f"Fichier résultats introuvable : {results_csv}")
        return None

    # Lecture du CSV (les lignes commençant par '#' sont des commentaires)
    df = pd.read_csv(results_csv, comment='#')
    if 'PID' not in df.columns:
        print("Fichier CSV invalide : colonne PID manquante.")
        return None

    pids = df['PID']
    resp = df['Response Time']
    wait = df['Wait Time']
    turn = df['Turnaround Time']

    x = np.arange(len(pids))          # positions des groupes
    width = 0.25                      # largeur de chaque barre

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.bar(x - width, resp, width, label='Temps de réponse', color=COLORS['resp'])
    ax.bar(x, wait, width, label="Temps d'attente", color=COLORS['wait'])
    ax.bar(x + width, turn, width, label='Turnaround', color=COLORS['turn'])

    ax.set_xlabel('Processus')
    ax.set_ylabel('Temps (ms)')
    ax.set_title('Comparaison des métriques par processus')
    ax.set_xticks(x)
    ax.set_xticklabels([f'P{pid}' for pid in pids])
    ax.legend()
    ax.grid(True, axis='y', linestyle='--', alpha=0.7)

    plt.tight_layout()
    if output_png:
        plt.savefig(output_png, dpi=150)
        print(f"Graphique à barres sauvegardé : {output_png}")
    return fig


def plot_radar(results_csv, output_png):
    """
    Génère un diagramme radar (toile d'araignée) des performances globales.

    Les valeurs sont normalisées : plus la valeur est proche de 0, meilleure
    est la performance (sauf pour l'occupation CPU où 100% est idéal).
    Le graphique présente : temps de réponse, temps d'attente,
    turnaround, et l'inverse de l'utilisation CPU.

    @param results_csv (str) Chemin du fichier CSV contenant les résultats.
    @param output_png (str or None) Chemin du fichier PNG de sortie.
    @return matplotlib.figure.Figure ou None.
    """
    if not os.path.exists(results_csv):
        print(f"Fichier résultats introuvable : {results_csv}")
        return None

    # Lecture des lignes pour extraire les moyennes et l'utilisation CPU
    with open(results_csv, 'r') as f:
        lines = f.readlines()

    avg_resp = avg_wait = avg_turn = cpu_util = 0.0
    for line in lines:
        if line.startswith('#Moyennes'):
            parts = line.strip().split(',')
            if len(parts) >= 4:
                avg_resp = float(parts[1])
                avg_wait = float(parts[2])
                avg_turn = float(parts[3])
        elif line.startswith('#CPU Utilization'):
            parts = line.strip().split(',')
            if len(parts) >= 2:
                cpu_util = float(parts[1])

    # Normalisation : on fixe des bornes supérieures pour éviter une trop grande dispersion
    max_resp = max(avg_resp, 10)
    max_wait = max(avg_wait, 10)
    max_turn = max(avg_turn, 50)
    resp_norm = avg_resp / max_resp
    wait_norm = avg_wait / max_wait
    turn_norm = avg_turn / max_turn
    cpu_norm = cpu_util / 100
    cpu_norm_inv = 1 - cpu_norm          # Inverser pour que 100% donne 0 (meilleur)

    values = [resp_norm, wait_norm, turn_norm, cpu_norm_inv]
    categories = ['Réponse', 'Attente', 'Turnaround', 'CPU %']

    # Angles pour chaque axe du radar
    angles = np.linspace(0, 2 * np.pi, len(categories), endpoint=False).tolist()
    # Fermeture du polygone
    values += values[:1]
    angles += angles[:1]

    fig, ax = plt.subplots(figsize=(8, 8), subplot_kw=dict(polar=True))
    ax.plot(angles, values, 'o-', linewidth=2, color=COLORS['turn'], markersize=6)
    ax.fill(angles, values, alpha=0.25, color=COLORS['turn'])
    ax.set_rgrids([0.2, 0.4, 0.6, 0.8, 1.0], angle=0, fontsize=8)

    # Affichage des valeurs réelles au-dessus de chaque axe
    for i, (cat, val) in enumerate(zip(categories, [avg_resp, avg_wait, avg_turn, cpu_util])):
        angle = angles[i]
        ax.text(angle, 0.1, f'{val:.1f}', ha='center', va='center', fontsize=9,
                bbox=dict(boxstyle="round,pad=0.2", facecolor="white", alpha=0.7))

    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(categories, fontsize=10)
    ax.set_ylim(0, 1)
    ax.set_title('Performances globales (normalisées, 0=meilleur, 1=pire sauf CPU)', pad=20)

    plt.tight_layout()
    if output_png:
        plt.savefig(output_png, dpi=150)
        print(f"Diagramme radar sauvegardé : {output_png}")
    return fig


def plot_cpu_usage(timeline_file, output_png):
    """
    Génère un graphique en escalier montrant l'occupation du CPU au cours du temps.

    @param timeline_file (str) Chemin du fichier CSV de timeline.
    @param output_png (str or None) Chemin du fichier PNG de sortie.
    @return matplotlib.figure.Figure ou None.
    """
    if not os.path.exists(timeline_file):
        print(f"Fichier timeline introuvable : {timeline_file}")
        return None

    df = pd.read_csv(timeline_file)
    time_cols = df.columns[1:]
    max_time = len(time_cols)

    # Tableau indiquant pour chaque instant si au moins un processus est en CPU
    cpu_active = np.zeros(max_time)
    for i, row in df.iterrows():
        for t, state in enumerate(row[1:]):
            if state == 'U':
                cpu_active[t] += 1
    cpu_active = (cpu_active > 0).astype(int)   # 1 si actif, 0 sinon

    fig, ax = plt.subplots(figsize=(12, 4))
    # step='mid' pour aligner les marches au milieu des intervalles
    ax.fill_between(range(max_time), 0, cpu_active, step='mid',
                    color=COLORS['U'], alpha=0.7, edgecolor='none')
    ax.set_ylim(0, 1.1)
    ax.set_xlim(0, max_time)
    ax.set_xlabel('Temps (ms)')
    ax.set_ylabel('CPU actif')
    ax.set_yticks([0, 1])
    ax.set_yticklabels(['Inactif', 'Actif'])
    ax.set_title('Occupation CPU dans le temps')
    ax.grid(axis='x', linestyle='--', alpha=0.5)

    plt.tight_layout()
    if output_png:
        plt.savefig(output_png, dpi=150)
        print(f"Graphique d'occupation CPU sauvegardé : {output_png}")
    return fig


# =============================================================================
# Fonctions d'affichage combiné (non utilisée par défaut, mais conservée)
# =============================================================================

def show_combined(policy, results_dir):
    """
    Affiche les quatre graphiques dans quatre fenêtres séparées.
    (Fonction optionnelle non appelée par le main standard)

    @param policy (str) Nom de la politique (FIFO, SJF, ...)
    @param results_dir (str) Dossier contenant les fichiers CSV.
    """
    timeline_file = os.path.join(results_dir, f"timeline_{policy}.csv")
    results_file = os.path.join(results_dir, f"results_{policy}.csv")

    # Génération sans sauvegarde (output_png=None)
    fig_gantt = plot_gantt(timeline_file, None)
    fig_bar = plot_bar_chart(results_file, None)
    fig_radar = plot_radar(results_file, None)
    fig_cpu = plot_cpu_usage(timeline_file, None)

    if any(f is None for f in [fig_gantt, fig_bar, fig_radar, fig_cpu]):
        print("Erreur : impossible de charger tous les graphiques.")
        return

    # Affichage séquentiel
    plt.figure(fig_gantt.number)
    plt.show()
    plt.figure(fig_bar.number)
    plt.show()
    plt.figure(fig_radar.number)
    plt.show()
    plt.figure(fig_cpu.number)
    plt.show()


# =============================================================================
# Point d'entrée principal
# =============================================================================

if __name__ == "__main__":
    """
    Usage : python plot_gantt.py <policy_name> [--show]
    Exemple: python plot_gantt.py FIFO --show
    """
    if len(sys.argv) < 2:
        print("Usage: python plot_gantt.py <policy_name> [--show]")
        print("Exemple: python plot_gantt.py FIFO --show")
        sys.exit(1)

    policy = sys.argv[1]
    show = "--show" in sys.argv or "show" in sys.argv
    results_dir = "results"
    os.makedirs(results_dir, exist_ok=True)

    timeline_file = os.path.join(results_dir, f"timeline_{policy}.csv")
    results_file = os.path.join(results_dir, f"results_{policy}.csv")

    # Génération avec sauvegarde des images
    fig_gantt = plot_gantt(timeline_file, os.path.join(results_dir, f"gantt_{policy}.png"))
    fig_bar = plot_bar_chart(results_file, os.path.join(results_dir, f"barchart_{policy}.png"))
    fig_radar = plot_radar(results_file, os.path.join(results_dir, f"radar_{policy}.png"))
    fig_cpu = plot_cpu_usage(timeline_file, os.path.join(results_dir, f"cpu_usage_{policy}.png"))

    # Affichage à l'écran si demandé
    if show:
        if fig_gantt:
            plt.figure(fig_gantt.number)
            plt.show()
        if fig_bar:
            plt.figure(fig_bar.number)
            plt.show()
        if fig_radar:
            plt.figure(fig_radar.number)
            plt.show()
        if fig_cpu:
            plt.figure(fig_cpu.number)
            plt.show()

    print("\nTous les graphiques ont été générés avec succès.")
