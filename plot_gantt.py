#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
import sys
import os

# === COULEURS adaptées aux daltoniens ===
COLORS = {
    'U': "#0d00ff",   # CPU → bleu
    'W': "#fa9e00",   # Attente → orange
    'O': "#168400",   # E/S → vert
    '.': "#ccc9c9"    # Inactif → gris clair
}
LEGEND_LABELS = {
    'U': 'CPU (U)',
    'W': 'Attente (W)',
    'O': 'E/S (O)',
    '.': 'Inactif'
}

def plot_gantt(timeline_file, output_file=None):
    """
    Lit le fichier timeline CSV et génère un diagramme de Gantt.
    """
    # Vérifier l'existence du fichier
    if not os.path.exists(timeline_file):
        print(f"Erreur : fichier {timeline_file} introuvable.")
        return

    # Lire le CSV
    df = pd.read_csv(timeline_file)
    # La première colonne est 'PID', les suivantes sont les temps
    time_columns = df.columns[1:]
    max_time = len(time_columns)
    nb_proc = len(df)

    # Définir la taille de la figure (ajustable)
    fig_width = max(10, max_time * 0.2)
    fig_height = max(2, nb_proc * 0.6)
    fig, ax = plt.subplots(figsize=(fig_width, fig_height))

    # Parcourir chaque processus
    for i, row in df.iterrows():
        pid = row['PID']
        for t, state in enumerate(row[1:]):
            # Dessiner la barre
            ax.barh(
                y=i,
                width=1,
                left=t,
                color=COLORS.get(state, '#000000'),
                edgecolor='black',
                linewidth=0.5
            )
            # Ajouter la lettre au centre de la case
            ax.text(t + 0.5, i, state, ha='center', va='center',
                    color='white', fontsize=8, fontweight='bold')

    # Personnalisation des axes
    ax.set_yticks(range(nb_proc))
    ax.set_yticklabels(df['PID'])
    ax.set_xlim(0, max_time)
    ax.set_xticks(range(0, max_time+1, max(1, max_time//20)))
    ax.set_xlabel('Temps (ms)')
    ax.set_title(f'Diagramme de Gantt - {os.path.basename(timeline_file).replace("timeline_","").replace(".csv","")}')
    ax.invert_yaxis()  # P1 en haut
    ax.grid(axis='x', linestyle='--', alpha=0.5)

    # Légende
    patches = [mpatches.Patch(color=COLORS[state], label=LEGEND_LABELS[state])
               for state in ['U', 'W', 'O', '.']]
    ax.legend(handles=patches, loc='upper right', bbox_to_anchor=(1.15, 1))

    plt.tight_layout()
    if output_file is None:
        output_file = timeline_file.replace(".csv", ".png").replace("timeline_", "gantt_")
    plt.savefig(output_file, dpi=300)
    plt.close()
    print(f"Diagramme de Gantt sauvegardé : {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot_gantt.py <timeline_file.csv> [output.png]")
        print("Exemple : python plot_gantt.py results/timeline_FIFO.csv")
        sys.exit(1)
    timeline_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    plot_gantt(timeline_file, output_file)