# Simulateur d’Ordonnancement de Processus

##  Description

Ce projet implémente un simulateur d’ordonnancement de processus en langage C. Il permet d’exécuter plusieurs algorithmes d’ordonnancement (FIFO, SJF, SRJF, Round Robin) sur un jeu de processus définis par des cycles CPU et des entrées/sorties (E/S) parallélisables.

Le simulateur produit :

- Une **sortie console** détaillée (chronologie, indicateurs individuels et globaux)
- Un **export CSV** des résultats (indicateurs par processus et synthèse)
- Un **export CSV de la timeline** pour une visualisation externe
- Des **graphiques automatiques** (diagramme de Gantt, barres comparatives, radar, occupation CPU) via un script Python

---

##  Fonctionnalités

### Algorithmes implémentés
- **FIFO** (First In, First Out) – non préemptif
- **SJF** (Shortest Job First) – non préemptif
- **SRJF** (Shortest Remaining Job First) – préemptif
- **Round Robin** (quantum paramétrable) – préemptif

### Gestion des processus
- Chargement depuis un fichier texte
- Saisie manuelle interactive

### Indicateurs calculés
- Temps d’attente (moyen et par processus)
- Temps de restitution (turnaround)
- Temps de réponse
- Taux d’occupation CPU

### Sorties
- Chronologie textuelle (`U` = CPU, `O` = E/S, `W` = Attente, `.` = inactif)
- Tableau tabulé copiable dans un tableur
- Export CSV (`results/results_*.csv`) et timeline (`results/timeline_*.csv`)
- Génération de graphiques (Python + Matplotlib)

---

##  Installation


### 1. Récupérer le projet
```bash
git clone https://github.com/wafflepoo/Scheduling_Simulator
cd Scheduling_Simulator
```


### Prérequis
- **Compilateur C** (GCC recommandé)
- **make** (GNU make)
- **Python 3** et les bibliothèques suivantes : 
```bash
pip install matplotlib pandas
```
- **Doxygen**  (optionnel, pour la documentation)


##  Compilation
Placez-vous dans le répertoire racine du projet et lancez :
```bash
make
```
L’exécutable scheduler (ou scheduler.exe sous Windows) sera créé.

## Génération de la documentation

```bash
make doc
```
La documentation HTML sera générée dans le dossier doc/.

## Utilisation

Lancez le programme :

```bash
./simulateur
```


Un menu interactif vous guide :

- **Charger un fichier de processus (ou saisie manuelle)**
**Choisir l’algorithme (FIFO, SJF, SRJF, RR – avec quantum modifiable)**
- **Lancer la simulation**
- **Exporter les résultats en CSV (fichiers dans results/)**
- **Visualiser avec matplotlib (génère et affiche les graphiques)**

Vous pouvez également exécuter directement en ligne de commande :


```bash
./simulateur <fichier> [algorithme] [fichier_csv] 
```
### Exemple

```bash
./simulateur tests/exemple.txt fifo resultats.csv
```


## Format du fichier d’entrée
Le fichier texte doit contenir une ligne par processus, au format :


```bash
PID arrival cpu1 io1 cpu2 io2 ... cpuN
```

- **PID : identifiant (entier)**
- **arrival : temps d’arrivée (ms)**
- **cpuX : durée du X-ième burst CPU (ms)**
- **ioX : durée de l’E/S après le X-ième burst (sauf après le dernier burst)**

### Exemple

```bash
1 0 3 2 2 2 2
2 1 2 2 3 3 2
```

Les lignes commençant par # sont ignorées (commentaires).

## Sorties

### Console
- **Chronologie de l’exécution**
- **Tableau des résultats individuels (PID, arrivée, turnaround, attente, réponse) en format CSV**
- **Résumé synthétique**



### Fichiers CSV 
- **results/results_<algorithme>.csv : indicateurs par processus + moyennes**
- **results/timeline_<algorithme>.csv : état de chaque processus pour chaque unité de temps (U, O, W, .)**

### Graphiques (Python)
Exécutez l’option correspondante du menu pour générer automatiquement :

- **gantt_<algo>.png : diagramme de Gantt**
- **barchart_<algo>.png : barres comparatives**
- **radar_<algo>.png : diagramme radar**
- **cpu_usage_<algo>.png : occupation CPU**



## Structure du projet
```bash
Scheduling_Simulator/
.
├── Makefile
├── Doxyfile
├── README.md
├── include/
│   └── scheduler.h
├── src/
│   ├── main.c
│   ├── simulator.c
│   ├── fifo_select.c
│   ├── sjf_select.c
│   ├── sjrf_select.c
│   ├── rr_select.c
│   ├── policies.c
│   ├── parser.c
│   └── utils.c
├── results/
├── tests/
└── plot_gantt.py
```

## Dépendances

- **GCC (ou tout compilateur C compatible)**
- **make**
- **Python 3 avec matplotlib et pandas**
- **Doxygen**

## Auteurs
Ce projet a été réalisé dans le cadre du cours d’OS (L3 Informatique).

Membres du groupe :
MISSAOUI Alissa
TAKKA Kamelia
HORNUNG Thomas

## Licence
Ce projet est fourni à des fins pédagogiques. Toute réutilisation est soumise à l’accord des auteurs.
