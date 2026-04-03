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
