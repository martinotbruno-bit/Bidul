# TCHAK ! – La Grande Catastrophe

Jeu de réflexion et de réactions en chaîne pour **Nintendo NES** (8 bits, esthétique 1992).
Vous êtes BIDULE, une créature qui ne sait pas sauter mais qui sait faire **TCHAK !** : une contraction brutale qui secoue tout autour de lui… et déclenche des catastrophes en cascade.

![Titre](docs/img/02_titre.png) ![Salle 1-2](docs/img/07_chaine_convoyeur.png)

## État : prototype 0.1 (phases 1 à 3 du cahier des charges)

Conformément au §22 du cahier des charges, le projet commence par l'architecture technique et un **prototype jouable de 3 salles** pour valider la mécanique principale avant de produire les 40 salles.

Ce qui est programmé et testé :

- ROM iNES (mapper 0, 40 Ko) : `dist/tchak_prototype.nes`
- BIDULE avec ses 13 types d'animation, le TCHAK, échelles, ressorts, convoyeurs, cibles, plaques, leviers, cuves de gelée, portes
- Système d'objets générique (balle, caisse, chaussette) et un ennemi (le Trouillard)
- Réactions en chaîne avec délai visible et protection contre les boucles infinies
- 3 salles du Monde 1 : une simple, une à réaction en chaîne, une à 3 solutions (+ 2 chaussettes secrètes)
- Écran titre avec logo animé, menu, mot de passe, présentation du monde, pause, fin de partie, écran de fin
- Moteur son APU : 5 musiques originales, 13 bruitages
- 18 tests automatiques sous émulateur (tous réussis)

Pas encore fait : les 37 autres salles, les 5 boss, les mondes 2 à 5, le passage au mapper MMC1 (voir `docs/ARCHITECTURE.md`).

## Jouer

Ouvrez `dist/tchak_prototype.nes` dans un émulateur NES. Commandes : croix = marcher / échelles, **B = TCHAK**, A = levier, START = pause, SELECT = recommencer la salle. Voir le [manuel](docs/MANUEL.md).

## Compiler

```sh
sudo apt-get install cc65        # ca65, cc65, ld65
pip install pillow cynes         # outils graphiques + émulateur des tests
make                             # -> build/tchak.nes
make test                        # 18 tests automatiques
make dist                        # copie la ROM dans dist/
```

## Organisation

| Dossier | Contenu |
|---|---|
| `src/` | code du jeu : C (cc65) + assembleur 6502 (ca65) |
| `rooms/` | salles en texte (carte, objets, déclencheurs) |
| `tools/` | générateurs : graphismes (`gfx.py`, `art_bidule.py`), salles (`rooms.py`), musique (`music.py`) |
| `tests/` | banc de test sous émulateur |
| `cfg/` | carte mémoire de l'éditeur de liens |
| `docs/` | cahier des charges, [architecture](docs/ARCHITECTURE.md), [conception](docs/GAME_DESIGN.md), [manuel](docs/MANUEL.md), [rapport de tests](docs/RAPPORT_TESTS.md) |
