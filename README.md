# OS-Scheduler-Sim

![GitHub](https://img.shields.io/badge/license-MIT-blue)
![GitHub](https://img.shields.io/badge/langage-C-orange)
![GitHub](https://img.shields.io/badge/version-1.0-green)

**Simulateur d'ordonnancement de processus en C**  
*Projet L3 Informatique - Groupe TD C*

## 👥 Équipe
| Membre | Rôle principal |
|--------|----------------|
| Alissa Missaoui | Architecture & modules de base |
| Thomas Hornung | Algorithmes avancés |
| Kamelia Takka | I/O, documentation & rapports |

## 📊 Diagramme de Gantt (Planification)

```mermaid
gantt
    title OS-Scheduler-Sim - Planification du projet
    dateFormat  YYYY-MM-DD
    axisFormat %d/%m
    
    %% PHASE 1: CONCEPTION
    section Phase 1: Conception
    Architecture commune          :a1, 2026-02-09, 7d
    Étude détaillée des besoins   :2026-02-09, 5d
    
    %% ALISSA
    section Alissa (Architecture)
    Lecture/Parsing fichiers      :a2, after a1, 5d
    Implémentation FIFO           :a3, after a2, 6d
    Architecture modulaire        :a4, after a3, 5d
    Module simulation principale  :a5, after a4, 4d
    Export CSV générique          :a6, after a5, 5d
    Tests unitaires (P1)          :a7, after a6, 10d
    
    %% THOMAS
    section Thomas (Algorithmes)
    Étude SJF/SJRF                :b1, after a1, 4d
    Implémentation SJF            :b2, after b1, 7d
    Implémentation SJRF           :b3, after b2, 6d
    Implémentation Round Robin    :b4, after b3, 7d
    Optimisation files d'attente  :b5, after b4, 4d
    Tests croisés (P2)            :b6, after b5, 7d
    
    %% KAMELIA
    section Kamelia (I/O & Docs)
    Calcul indicateurs            :c1, after a1, 8d
    Guide utilisation Markdown    :c2, after c1, 5d
    Sortie texte formatée         :c3, after c2, 6d
    Graphiques résultats          :c4, after c3, 7d
    Rapport développeur           :c5, after c4, 8d
    Documentation finale          :c6, after c5, 4d
    
    %% ÉQUIPE
    section Tâches communes
    Tests intégration             :d1, after a7, 7d
    Extensions bonus              :d2, after d1, 5d
    Préparation soutenance        :d3, after d2, 5d
    Packaging .tgz final          :d4, after d3, 3d
    
    %% JALONS
    section 🎯 Jalons
    Rendu Gantt                   :milestone, m1, 2026-02-12, 0d
    FIFO fonctionnel              :milestone, m2, 2026-02-16, 0d
    Architecture prête            :milestone, m3, 2026-02-23, 0d
    4 algos implémentés           :milestone, m4, 2026-03-02, 0d
    Tests finaux                  :milestone, m5, 2026-03-23, 0d
    RENDU FINAL                   :milestone, m6, 2026-04-03, 0d
```

## 🔗 Dépendances critiques
1. **Architecture → Tous modules**
2. **FIFO → Tests unitaires**
3. **SJF/SJRF/RR → Tests croisés**
4. **Tous tests → Intégration**
5. **Intégration → Packaging**

## 📅 Dates clés
| Date | Événement | Détail |
|------|-----------|--------|
| 12/02/2026 | Rendu Gantt | Diagramme de planification |
| 16/02/2026 | FIFO fonctionnel | Premier algorithme opérationnel |
| 02/03/2026 | 4 algos prêts | Tous algorithmes implémentés |
| 23/03/2026 | Tests finaux | Validation complète |
| 03/04/2026 | **RENDU FINAL** | Archive .tgz sur Moodle |

## 🛠️ Installation rapide
```bash
git clone https://github.com/votre-compte/OS-Scheduler-Sim.git
cd OS-Scheduler-Sim
make
./scheduler-sim input.txt
```

## 📁 Structure du projet
```
OS-Scheduler-Sim/
├── src/                    # Code source C
├── include/               # Headers
├── docs/                  # Documentation
├── tests/                 # Tests
├── examples/              # Fichiers d'exemple
├── Makefile
└── README.md              # Ce fichier
```

## 📞 Contact
- **Email projet** : [à définir]
- **Repository** : https://github.com/votre-compte/OS-Scheduler-Sim
- **Rendu** : Moodle L3 Informatique

---

*Dernière mise à jour : Février 2026 | Projet académique - CYU*
