# Guide d'installation et d'utilisation

Simulateur d'ordonnancement de processus — L3 Informatique  
CY Cergy Paris Université — Groupe C6 — 2025/2026  
Auteurs : MISSAOUI Alissa, TAKKA Kamelia, HORNUNG Thomas

---

## Prérequis

Avant de commencer, vérifiez que vous disposez de :

- Un système Linux, ou Windows avec WSL
- GCC (compilateur C)
- Make
- Python 3 avec les bibliothèques matplotlib et pandas :
```bash
pip install matplotlib pandas
```

- Doxygen (optionnel, pour la documentation)

Pour vérifier :
```bash
gcc --version
make --version
python3 --version
```

---

## Installation

### 1. Récupérer le projet
```bash
git clone https://github.com/wafflepoo/Scheduling_Simulator
cd Scheduling_Simulator
```

### 2. Compiler
```bash
make
```

Cela génère l'exécutable `simulateur` à la racine du projet.

### 3. Installer (optionnel)
```bash
make install
```

Installe l'exécutable dans `~/bin/` pour pouvoir le lancer depuis n'importe où.

---

## Format du fichier de processus

Le simulateur lit les processus depuis un fichier texte.  
Une ligne par processus, selon le format suivant :
```
PID  arrivee  cpu1  io1  cpu2  io2  cpu3  ...
```

- `PID` : identifiant du processus
- `arrivee` : instant d'arrivée en ms
- `cpu1, cpu2...` : durées des cycles CPU en ms
- `io1, io2...` : durées des cycles d'E/S en ms (entre deux cycles CPU)
- Les lignes commençant par `#` sont des commentaires et sont ignorées

### Exemple
```
# PID  arrivee  cpu1  io1  cpu2  io2  cpu3
1      0        5     2    3
2      2        4     1    2     3    1
3      4        3
```

---

## Utilisation

### Syntaxe
```bash
./simulateur <fichier> <algorithme> [fichier_csv]
```

| Paramètre | Obligatoire | Description |
|---|---|---|
| `<fichier>` | Oui | Chemin vers le fichier de processus |
| `<algorithme>` | Oui | `fifo`, `sjf`, `sjrf` ou `rr` |
| `[fichier_csv]` | Non | Nom du fichier CSV à générer |

### Lancer le programme
```bash
./simulateur
```

Un menu s'affiche avec 7 options :
```
MENU PRINCIPAL
1. Charger un fichier de processus
2. Saisie manuelle des processus
3. Choisir l'algorithme d'ordonnancement
4. Lancer la simulation
5. Exporter les résultats en CSV
6. Visualiser avec matplotlib (Python)
7. Quitter
```
```
## Méthode 1 — Charger un fichier

### Étape 1 — Charger le fichier
Tapez `1` puis appuyez sur Entrée.  
Entrez le chemin vers votre fichier de processus :
```
tests/exemple.txt
```
Le programme confirme le chargement et affiche la liste des processus.  
Appuyez sur Entrée pour revenir au menu.

### Étape 2 — Choisir l'algorithme
Tapez `3` puis appuyez sur Entrée.  
Choisissez parmi les 4 algorithmes disponibles :
- `fifo` — First In First Out (premier arrivé, premier servi)
- `sjf` — Shortest Job First (plus court burst en premier)
- `sjrf` — Shortest Job Remaining First (préemptif)
- `rr` — Round Robin (quantum de temps paramétrable)

### Étape 3 — Lancer la simulation
Tapez `4` puis appuyez sur Entrée.  
Le simulateur affiche :
- La chronologie d'exécution tick par tick :
  - `U` = processus en cours d'exécution CPU
  - `W` = processus en attente dans la file
  - `O` = processus en cours d'E/S
  - `.` = processus pas encore arrivé ou terminé
- Un tableau des métriques par processus (temps d'attente, de réponse, turnaround)
- Les moyennes globales et le taux d'occupation CPU

### Étape 4 — Exporter en CSV
Tapez `5` puis appuyez sur Entrée.  
Deux fichiers sont générés dans le dossier `results/` :
- `results_<algorithme>.csv` : indicateurs par processus et moyennes
- `timeline_<algorithme>.csv` : chronologie complète

Pour consulter le CSV dans le terminal :
```bash
cat results/results_FIFO.csv
```
Pour l'ouvrir dans Excel ou LibreOffice Calc :  
Naviguez vers le dossier `results/` depuis l'explorateur Windows et double-cliquez sur le fichier CSV. Choisissez la virgule comme séparateur.

### Étape 5 — Visualiser les graphiques
Tapez `6` puis appuyez sur Entrée.  
Quatre graphiques s'affichent automatiquement :
- **Diagramme de Gantt** : chronologie d'exécution de chaque processus (bleu = CPU, vert = E/S, orange = attente)
- **Graphique à barres** : comparaison des temps de réponse, d'attente et de restitution par processus
- **Radar** : vue globale normalisée des performances — plus la surface est petite, meilleur est l'algorithme
- **Occupation CPU** : zones bleues = CPU actif, zones blanches = CPU inactif

### Étape 6 — Quitter
Tapez `7` puis appuyez sur Entrée.

---

## Méthode 2 — Saisie manuelle

### Étape 1 — Lancer la saisie manuelle
Tapez `2` puis appuyez sur Entrée.  
Entrez le nombre de processus à saisir.

### Étape 2 — Saisir chaque processus
Pour chaque processus, entrez :
- Le temps d'arrivée en ms
- Le nombre de bursts CPU
- La durée de chaque burst CPU en ms
- La durée de l'E/S après chaque burst (sauf le dernier)

Exemple pour 2 processus :
```
Nombre de processus : 2

--- Processus 1 ---
Temps d'arrivée (ms) : 0
Nombre de bursts CPU : 2
  Durée du burst CPU 1 : 5
  Durée de l'E/S après ce burst : 3
  Durée du burst CPU 2 : 4

--- Processus 2 ---
Temps d'arrivée (ms) : 2
Nombre de bursts CPU : 2
  Durée du burst CPU 1 : 3
  Durée de l'E/S après ce burst : 2
  Durée du burst CPU 2 : 6
```

Le programme confirme la saisie et affiche la liste des processus.  
Appuyez sur Entrée pour revenir au menu.

### Étapes suivantes
Poursuivez de la même façon qu'avec la méthode 1 :  
Choix `3` → algorithme, `4` → simulation, `5` → CSV, `6` → graphiques, `7` → quitter.

---

## Générer la documentation
```bash
make doc
```

La documentation HTML est générée dans `doc/html/`.  
Ouvrez `doc/html/index.html` dans un navigateur pour la consulter.

---

## Nettoyage
```bash
make clean
```

Supprime l'exécutable et les fichiers objets compilés.

---


Pour plus de détails, consulter le rapport développeur (PDF).
