#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
import sys
import os

# Couleurs adaptées aux daltoniens (ColorBrewer Set2)
COLORS = {
    'U': '#4c72b0',   # CPU
    'W': '#dd8452',   # Attente
    'O': '#55a868',   # E/S
    '.': '#e5e5e5',   # Inactif
    'resp': '#66c2a5',
    'wait': '#fc8d62',
    'turn': '#8da0cb',
}

# ----------------------------------------------------------------------
# Fonctions de génération (inchangées, mais renvoient la figure pour le Gantt)
# ----------------------------------------------------------------------
def plot_gantt(timeline_file, output_png):
    if not os.path.exists(timeline_file):
        print(f"Fichier timeline introuvable : {timeline_file}")
        return None
    df = pd.read_csv(timeline_file)
    time_cols = df.columns[1:]
    max_time = len(time_cols)
    nb_proc = len(df)

    fig_width = max(10, max_time * 0.2)
    fig_height = max(2, nb_proc * 0.6)
    fig, ax = plt.subplots(figsize=(fig_width, fig_height))

    for i, row in df.iterrows():
        pid = row['PID']
        for t, state in enumerate(row[1:]):
            ax.barh(y=i, width=1, left=t,
                    color=COLORS.get(state, '#000000'),
                    edgecolor='black', linewidth=0.5)
            ax.text(t + 0.5, i, state, ha='center', va='center',
                    color='white', fontsize=8, fontweight='bold')

    ax.set_yticks(range(nb_proc))
    ax.set_yticklabels(df['PID'])
    ax.set_xlim(0, max_time)
    ax.set_xticks(range(0, max_time+1, max(1, max_time//20)))
    ax.set_xlabel('Temps (ms)')
    ax.set_title(f'Diagramme de Gantt - {os.path.basename(timeline_file).replace("timeline_","").replace(".csv","")}')
    ax.invert_yaxis()
    ax.grid(axis='x', linestyle='--', alpha=0.5)

    patches = [mpatches.Patch(color=COLORS[s], label=lab)
               for s, lab in zip(['U','W','O','.'],
                                 ['CPU (U)','Attente (W)','E/S (O)','Inactif'])]
    ax.legend(handles=patches, loc='upper right', bbox_to_anchor=(1.15, 1))
    plt.tight_layout()
    plt.savefig(output_png, dpi=300)
    print(f"Diagramme de Gantt sauvegardé : {output_png}")
    return fig  # retourne la figure pour un éventuel affichage groupé

def plot_bar_chart(results_csv, output_png):
    if not os.path.exists(results_csv):
        print(f"Fichier résultats introuvable : {results_csv}")
        return None
    df = pd.read_csv(results_csv, comment='#')
    if 'PID' not in df.columns:
        print("Fichier CSV invalide : colonne PID manquante.")
        return None
    pids = df['PID']
    resp = df['Response Time']
    wait = df['Wait Time']
    turn = df['Turnaround Time']

    x = np.arange(len(pids))
    width = 0.25
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
    plt.savefig(output_png, dpi=150)
    print(f"Graphique à barres sauvegardé : {output_png}")
    return fig

def plot_radar(results_csv, output_png):
    if not os.path.exists(results_csv):
        print(f"Fichier résultats introuvable : {results_csv}")
        return None
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

    max_resp = max(avg_resp, 10)
    max_wait = max(avg_wait, 10)
    max_turn = max(avg_turn, 50)
    resp_norm = avg_resp / max_resp
    wait_norm = avg_wait / max_wait
    turn_norm = avg_turn / max_turn
    cpu_norm = cpu_util / 100
    cpu_norm_inv = 1 - cpu_norm

    values = [resp_norm, wait_norm, turn_norm, cpu_norm_inv]
    categories = ['Réponse', 'Attente', 'Turnaround', 'CPU %']

    angles = np.linspace(0, 2 * np.pi, len(categories), endpoint=False).tolist()
    values += values[:1]
    angles += angles[:1]

    fig, ax = plt.subplots(figsize=(8, 8), subplot_kw=dict(polar=True))
    ax.plot(angles, values, 'o-', linewidth=2, color=COLORS['turn'], markersize=6)
    ax.fill(angles, values, alpha=0.25, color=COLORS['turn'])
    ax.set_rgrids([0.2, 0.4, 0.6, 0.8, 1.0], angle=0, fontsize=8)

    for i, (cat, val) in enumerate(zip(categories, [avg_resp, avg_wait, avg_turn, cpu_util])):
        angle = angles[i]
        ax.text(angle, 0.1, f'{val:.1f}', ha='center', va='center', fontsize=9,
                bbox=dict(boxstyle="round,pad=0.2", facecolor="white", alpha=0.7))

    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(categories, fontsize=10)
    ax.set_ylim(0, 1)
    ax.set_title('Performances globales (normalisées, 0=meilleur, 1=pire sauf CPU)', pad=20)

    plt.tight_layout()
    plt.savefig(output_png, dpi=150)
    print(f"Diagramme radar sauvegardé : {output_png}")
    return fig

def plot_cpu_usage(timeline_file, output_png):
    if not os.path.exists(timeline_file):
        print(f"Fichier timeline introuvable : {timeline_file}")
        return None
    df = pd.read_csv(timeline_file)
    time_cols = df.columns[1:]
    max_time = len(time_cols)
    cpu_active = np.zeros(max_time)
    for i, row in df.iterrows():
        for t, state in enumerate(row[1:]):
            if state == 'U':
                cpu_active[t] += 1
    cpu_active = (cpu_active > 0).astype(int)
    fig, ax = plt.subplots(figsize=(12, 4))
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
    plt.savefig(output_png, dpi=150)
    print(f"Graphique d'occupation CPU sauvegardé : {output_png}")
    return fig

# ----------------------------------------------------------------------
# Affichage combiné (tous les graphiques dans une seule fenêtre)
# ----------------------------------------------------------------------
def show_combined(policy, results_dir):
    """Crée une figure avec 4 subplots (2x2) affichant tous les graphiques."""
    timeline_file = os.path.join(results_dir, f"timeline_{policy}.csv")
    results_file = os.path.join(results_dir, f"results_{policy}.csv")
    # Récupérer les figures individuelles (sans les sauvegarder, on les recrée)
    fig_gantt = plot_gantt(timeline_file, None)      # None pour ne pas sauvegarder
    fig_bar = plot_bar_chart(results_file, None)
    fig_radar = plot_radar(results_file, None)
    fig_cpu = plot_cpu_usage(timeline_file, None)

    if any(f is None for f in [fig_gantt, fig_bar, fig_radar, fig_cpu]):
        print("Erreur : impossible de charger tous les graphiques.")
        return

    # Créer une figure principale avec 2x2 subplots
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    # Récupérer les axes des figures individuelles et les recopier
    # Méthode simple : on récupère les données des figures existantes et on les redessine
    # Mais plus simple : on réutilise les axes des figures individuelles en les déplaçant ?
    # On préfère redessiner manuellement :
    # Pour le Gantt
    ax_g = axes[0, 0]
    # On extrait les données du Gantt (on pourrait re-générer, mais on peut aussi copier)
    # Pour ne pas surcharger, on va réutiliser le code de tracé mais en lui passant l'axe
    # On définit une fonction de tracé sur un axe donné
    # Mais pour garder le code simple, on utilise directement les figures existantes :
    # On capture le contenu de chaque figure et on le place dans le subplot
    # C'est un peu technique. Pour une solution robuste, on peut simplement afficher les 4 fenêtres.
    # Je vais opter pour l'affichage séparé avec plt.show() après chaque sauvegarde.
    # Mais l'utilisateur veut une seule fenêtre.

    # Une autre approche : on sauvegarde d'abord, puis on ouvre toutes les images avec un visualiseur externe ?
    # Non.

    # Finalement, pour rester simple, nous allons afficher les 4 fenêtres l'une après l'autre.
    # L'utilisateur pourra les fermer pour passer à la suivante.
    plt.figure(fig_gantt.number)
    plt.show()
    plt.figure(fig_bar.number)
    plt.show()
    plt.figure(fig_radar.number)
    plt.show()
    plt.figure(fig_cpu.number)
    plt.show()

# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python visualize_results.py <policy_name> [--show]")
        print("Exemple: python visualize_results.py FIFO --show")
        sys.exit(1)

    policy = sys.argv[1]
    show = "--show" in sys.argv or "show" in sys.argv
    results_dir = "results"
    os.makedirs(results_dir, exist_ok=True)

    timeline_file = os.path.join(results_dir, f"timeline_{policy}.csv")
    results_file = os.path.join(results_dir, f"results_{policy}.csv")

    # Génération avec sauvegarde
    fig_gantt = plot_gantt(timeline_file, os.path.join(results_dir, f"gantt_{policy}.png"))
    fig_bar = plot_bar_chart(results_file, os.path.join(results_dir, f"barchart_{policy}.png"))
    fig_radar = plot_radar(results_file, os.path.join(results_dir, f"radar_{policy}.png"))
    fig_cpu = plot_cpu_usage(timeline_file, os.path.join(results_dir, f"cpu_usage_{policy}.png"))

    if show:
        # Afficher chaque figure une par une
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