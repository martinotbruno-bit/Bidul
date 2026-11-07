# TCHAK ! – La Grande Catastrophe

Jeu de réflexion et de réactions en chaîne pour **Nintendo NES** (8 bits, esthétique 1992).
Vous êtes BIDULE, une créature qui ne sait pas sauter mais qui sait faire **TCHAK !** : une contraction brutale qui secoue tout autour de lui… et déclenche des catastrophes en cascade.

![Titre](docs/img/02_titre.png) ![Boss](docs/img/21_boss.png)

## État : version 1.0 – le jeu complet : 5 mondes, 40 salles, 5 boss, cartouche MMC1 de 256 Ko

Le prototype de 3 salles a validé la mécanique (§22) ; le **Monde 1 – La Fabrique des choses molles** est maintenant complet : 8 salles de difficulté croissante et le boss, le Roi du Rebond.

Ce qui est programmé et testé :

- ROM iNES **MMC1** (256 Ko PRG + 32 Ko CHR) : `dist/tchak.nes`
- BIDULE avec ses 13 types d'animation, le TCHAK, échelles, ressorts, convoyeurs, cibles, plaques, leviers, cuves de gelée, portes
- Système d'objets générique (balle, caisse, chaussette) et un ennemi (le Trouillard)
- Réactions en chaîne avec délai visible et protection contre les boucles infinies
- Monde 1 complet : 8 salles de difficulté croissante (dans chacune, le geste évident échoue et la solution demande de comprendre une règle), 3 chaussettes secrètes, et le **boss : le Roi du Rebond** (3 phases)
- Écran titre avec logo animé, menu, mot de passe, présentation du monde, pause, fin de partie, écran de fin
- Moteur son APU : 6 musiques originales (dont le thème du boss), 13 bruitages
- 62 tests automatiques sous émulateur (tous réussis), dont un « joueur qui appuie au hasard » qui ne résout aucune des 40 salles

- **Monde 2 – La maison qui n'en fait qu'à sa tête** : les meubles ont des humeurs (fauteuil grognon, lampe curieuse, commode paresseuse, coffre gourmand), 8 salles, boss **Madame Armoire**

- **Monde 3 – L'usine des catastrophes** : pistons, le Copieur (qui imite BIDULE, ou fait l'inverse), le Machinal (qui déclenche tout sans le vouloir), 8 salles, boss **le Grand Dérangé**

- **Monde 4 – Le monde qui marche de travers** : la gravité des objets s'inverse (caisses au plafond, plaques et cibles au plafond), ballons qui tombent vers le haut, aimants ; 8 salles, boss **le Roi à l'Envers**

- **Monde 5 – Le Grand Bazar** : toutes les règles mélangées (meubles à humeurs et machines la tête en bas, Copieur au plafond, convoyeur + gravité), 8 salles, boss final **le Maître du Chaos** (il change les propriétés des objets : gravité, caisses qui flottent, convoyeurs). Écran « FIN DU JEU ».

La difficulté a été revue sur l'ensemble du jeu (voir [GAME_DESIGN §9](docs/GAME_DESIGN.md)) : chaque monde est un peu plus exigeant que le précédent, et un joueur qui appuie au hasard ne résout aucune des 40 salles.

## Jouer

Ouvrez `dist/tchak.nes` dans un émulateur NES. Commandes : croix = marcher / échelles, **B = TCHAK**, A = levier, START = pause, SELECT = recommencer la salle. Voir le [manuel](docs/MANUEL.md).

## Compiler

```sh
sudo apt-get install cc65        # ca65, cc65, ld65
pip install pillow cynes         # outils graphiques + émulateur des tests
make                             # -> build/tchak.nes
make test                        # 62 tests automatiques (~25 min)
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
