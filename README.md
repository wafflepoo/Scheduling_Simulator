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



%%{init: {'theme': 'default', 'themeVariables': {
  'primaryColor': '#ffffff',
  'primaryTextColor': '#333',
  'primaryBorderColor': '#ccc',
  'lineColor': '#666',
  'tertiaryColor': '#f0f0f0'
}}}%%

gantt
    title Diagramme de Gantt - Projet OS-Scheduler-Sim
    dateFormat  YYYY-MM-DD
    axisFormat %d/%m
    
    %% ========== ÉQUIPE ==========
    section Équipe (Tâches communes)
    Architecture commune      :a1, 2026-02-09, 7d
    Tests d'intégration       :a2, after t16, 7d
    Extensions bonus          :a3, after t17, 5d
    Préparation soutenance    :a4, after a3, 5d
    Packaging final (.tgz)    :a5, after a4, 3d
    
    %% ========== ALISSA ==========
    section Alissa (Architecture & Base)
    Lecture/Parsing fichiers  :t1, after a1, 5d
    Algorithme FIFO           :t2, after t1, 6d
    Architecture modulaire    :t3, after t2, 5d
    Module simulation         :t4, after t3, 4d
    Export CSV générique      :t5, after t4, 5d
    Tests unitaires           :t6, after t5, 10d
    
    %% ========== THOMAS ==========
    section Thomas (Algorithmes avancés)
    Étude SJF/SJRF            :t7, after a1, 4d
    Algorithme SJF            :t8, after t7, 7d
    Algorithme SJRF           :t9, after t8, 6d
    Algorithme Round Robin    :t10, after t9, 7d
    Optimisation files        :t11, after t10, 4d
    Tests croisés             :t12, after t11, 7d
    
    %% ========== KAMELIA ==========
    section Kamelia (I/O & Documentation)
    Calcul indicateurs        :t13, after a1, 8d
    Guide utilisateur         :t14, after t13, 5d
    Sortie texte formatée     :t15, after t14, 6d
    Graphiques résultats      :t16, after t15, 7d
    Rapport développeur       :t17, after t16, 8d
    Documentation finale      :t18, after t17, 4d
    
    %% ========== JALONS ==========
    section 🎯 JALONS IMPORTANTS
    Rendu Gantt (12/02)       :milestone, m1, 2026-02-12, 0d
    FIFO fonctionnel (16/02)  :milestone, m2, 2026-02-16, 0d
    Architecture prête (23/02):milestone, m3, 2026-02-23, 0d
    4 algos implémentés (02/03):milestone, m4, 2026-03-02, 0d
    Tests finaux (23/03)      :milestone, m5, 2026-03-23, 0d
    RENDU FINAL (03/04)       :milestone, m6, 2026-04-03, 0d
    
    %% ========== DÉPENDANCES ==========
    %% Dépendances critiques
    a1 --> t1
    a1 --> t7
    a1 --> t13
    t1 --> t2
    t2 --> t3
    t3 --> t4
    t4 --> t5
    t5 --> t6
    t7 --> t8
    t8 --> t9
    t9 --> t10
    t10 --> t11
    t11 --> t12
    t13 --> t14
    t14 --> t15
    t15 --> t16
    t16 --> t17
    t17 --> t18
    t6 --> a2
    t12 --> a2
    a2 --> a3
    a3 --> a4
    a4 --> a5

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
