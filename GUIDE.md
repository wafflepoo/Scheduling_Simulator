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
- Doxygen (optionnel, pour la documentation)

Pour vérifier :
```bash
gcc --version
make --version
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

### Exemples
```bash
# Simulation FIFO
./simulateur tests/exemple.txt fifo

# Simulation Round Robin avec export CSV
./simulateur tests/exemple.txt rr resultats.csv

# Simulation SJF
./simulateur tests/exemple.txt sjf

# Simulation SJRF
./simulateur tests/exemple.txt sjrf
```

---

## Résultats

Le programme affiche dans le terminal :

- Le nombre de processus chargés et leurs caractéristiques
- La chronologie d'exécution tick par tick :
  - `U` = processus en cours d'exécution CPU
  - `W` = processus en attente dans la file
  - `O` = processus en cours d'E/S
  - `.` = processus pas encore arrivé ou terminé
- Un tableau des métriques par processus
- Les moyennes globales et le taux d'occupation CPU

Si un fichier CSV est spécifié, les résultats sont également sauvegardés  
et peuvent être ouverts dans Excel ou LibreOffice Calc.

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

## Ajouter un nouvel algorithme

1. Créer `src/nom_select.c` avec la fonction de sélection
2. Déclarer la politique dans `src/policies.c`
3. Ajouter le cas dans `src/main.c`
4. Relancer `make`

Pour plus de détails, consulter le rapport développeur (PDF).
