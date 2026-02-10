# Scheduling_Simulator


**Simulateur d'ordonnancement de processus en C**  
*Projet L3 Informatique - Groupe TD C*

## Équipe
| Membre | Rôle principal |
|--------|----------------|
| Alissa Missaoui | I/O, documentation & rapports |
| Thomas Hornung | Algorithmes avancés |
| Kamelia Takka | Architecture & modules de base|

##  Diagramme de Gantt (Planification)

```mermaid

%%{init: {'themeVariables': {
  'sectionBkgColor': {
    'Équipe': '#E8F4FD',
    'Alissa': '#F0F8FF',
    'Thomas': '#F0FFF0',
    'Kamelia': '#FFF8F0',
    'Jalons': '#FFF0F0'
  },
  'taskBkgColor': {
    'Architecture commune': '#4B0082',
    'Tests d\'intégration': '#800080',
    'Extensions bonus': '#483D8B',
    'Préparation soutenance': '#2F4F4F',
    'Packaging final (.tgz)': '#191970'
  },
  'taskTextColor': '#FFFFFF',
  'taskTextOutsideColor': '#333333',
  'milestoneBkgColor': '#FF4500',
  'milestoneTextColor': '#FFFFFF'
}}}%%

gantt
    title Diagramme de Gantt - Projet OS-Scheduler-Sim
    dateFormat YYYY-MM-DD
    axisFormat %d/%m
    
    section Équipe
    Architecture commune      :2026-02-09, 7d
    Tests d'intégration       :2026-03-13, 7d
    Extensions bonus          :2026-03-20, 5d
    Préparation soutenance    :2026-03-25, 5d
    Packaging final           :2026-03-30, 3d
    
    section Alissa
    Lecture/Parsing           :2026-02-16, 5d
    Algorithme FIFO           :2026-02-21, 6d
    Architecture modulaire    :2026-02-27, 5d
    Module simulation         :2026-03-03, 4d
    Export CSV                :2026-03-07, 5d
    Tests unitaires           :2026-03-12, 10d
    
    section Thomas
    Étude SJF/SJRF            :2026-02-16, 4d
    Algorithme SJF            :2026-02-20, 7d
    Algorithme SJRF           :2026-02-27, 6d
    Algorithme Round Robin    :2026-03-04, 7d
    Optimisation              :2026-03-11, 4d
    Tests croisés             :2026-03-15, 7d
    
    section Kamelia
    Calcul indicateurs        :2026-02-16, 8d
    Guide utilisateur         :2026-02-24, 5d
    Sortie texte              :2026-02-29, 6d
    Graphiques                :2026-03-06, 7d
    Rapport développeur       :2026-03-13, 8d
    Documentation             :2026-03-21, 4d
    
    section 🎯 Jalons
    Rendu Gantt               :milestone, 2026-02-12, 0d
    FIFO fonctionnel          :milestone, 2026-02-16, 0d
    Architecture prête        :milestone, 2026-02-23, 0d
    4 algos implémentés       :milestone, 2026-03-02, 0d
    Tests finaux              :milestone, 2026-03-23, 0d
    RENDU FINAL               :milestone, 2026-04-03, 0d
    

```

##  Dépendances critiques
1. **Architecture → Tous modules**
2. **FIFO → Tests unitaires**
3. **SJF/SJRF/RR → Tests croisés**
4. **Tous tests → Intégration**
5. **Intégration → Packaging**

##  Dates clés
| Date | Événement | Détail |
|------|-----------|--------|
| 12/02/2026 | Rendu Gantt | Diagramme de planification |
| 16/02/2026 | FIFO fonctionnel | Premier algorithme opérationnel |
| 02/03/2026 | 4 algos prêts | Tous algorithmes implémentés |
| 23/03/2026 | Tests finaux | Validation complète |
| 03/04/2026 | **RENDU FINAL** | Archive .tgz sur Moodle |

##  Installation rapide
```bash
git clone https://github.com/votre-compte/Scheduling_Simulator.git
cd Scheduling_Simulator
make
./Scheduling_Simulator input.txt
```

##  Structure du projet
```
Scheduling_Simulator/
├── src/                    # Code source C
├── include/               # Headers
├── docs/                  # Documentation
├── tests/                 # Tests
├── examples/              # Fichiers d'exemple
├── Makefile
└── README.md              # Ce fichier
```


---

*Dernière mise à jour : Février 2026 | Projet académique - CYU*
