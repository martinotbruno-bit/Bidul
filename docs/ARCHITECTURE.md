# TCHAK ! – Document technique (architecture)

Version 0.3 (Monde 1 complet). Ce document présente les choix techniques (§14, §15 et §22 du cahier des charges) et l'organisation du code.

## 1. Choix techniques

| Sujet | Choix du prototype | Évolution prévue pour le jeu complet |
|---|---|---|
| Langage | **C (cc65 2.19)** pour la logique, **assembleur 6502 (ca65)** pour le démarrage, le NMI, le son et les routines critiques | Passer d'autres boucles critiques en assembleur si un monde dépasse le budget CPU |
| Mapper | **NROM-256** (mapper 0) : 32 Ko PRG + 8 Ko CHR-ROM | **MMC1 (SNROM/SUROM)** : 128–256 Ko PRG en banques de 16 Ko (une banque par monde : salles + logique propre), CHR en banques de 4 Ko (un jeu de décors par monde) |
| Format | iNES 1.0, miroir vertical | iNES/NES 2.0 avec MMC1 |
| Graphismes | CHR-ROM générée à partir d'art en texte (`tools/gfx.py`) | Une table de décors par monde, logo et police dans une banque commune |
| Sauvegarde | **Mot de passe** de 4 lettres (pas de pile) | Identique, avec 2 lettres de plus pour les chaussettes par monde |
| Son | Moteur maison, 6 flux sur les 4 canaux APU (le DMC n'est pas utilisé) | Ajout d'échantillons DMC pour le « TCHAK » si la place le permet |

Pourquoi C + assembleur : la logique de jeu (règles, réactions, états) change beaucoup pendant la conception ; le C la rend lisible et modifiable. Tout ce qui tourne en VBlank ou à chaque sprite est en assembleur.

### Utilisation des ressources (prototype, mesurée)

| Ressource | Utilisé | Disponible |
|---|---|---|
| PRG (code + données) | ≈ 28,1 Ko | 32 Ko |
| RAM | 56 o page zéro, 1 043 o BSS/DATA, 256 o OAM, pile C de 128 o | 2 Ko |
| Tuiles de décor | 186 | 256 |
| Tuiles de sprites | 164 | 256 |
| Sprites matériels | 6 (BIDULE) + 4 par objet + 4 (onde) + 2 (émotions) ≤ 44 | 64 |
| CPU par trame | salle au repos : ≈ 14 000 cycles libres ; pic pendant une réaction en chaîne : ≈ 3 000 cycles libres | 29 780 cycles (NTSC) |

Le temps libre est mesuré par la ROM elle-même (compteur `idle_cnt` dans `ppu_wait_nmi`) et une trame en retard est comptée dans `lag_frames` ; le test `t17_performances` vérifie qu'il n'y en a aucune pendant la réaction en chaîne de la salle 1-2.

### Respect des contraintes NES

- 256 × 240, palette NES, décors en tuiles 8×8 groupées en **métatuiles 16×16** (une métatuile = un quadrant d'attributs, donc pas de conflit de couleur).
- Sprites 8×8 uniquement ; BIDULE fait 2×3 sprites, les objets 2×2.
- Plus de 8 sprites sur une ligne : l'ordre de dessin des objets **tourne à chaque trame** (scintillement à la manière des jeux d'époque) ; BIDULE est toujours dessiné en premier et ne scintille jamais.
- Aucune virgule flottante : positions en 1/16 de pixel (12.4), vitesses en 1/16 px/trame, gravité = 3/16 px/trame².
- Les modifications de décor (porte qui s'ouvre, plaque enfoncée…) passent par une **file VRAM** vidée pendant la VBlank (≤ 124 octets par trame).
- Effets « modernes » interdits : pas de dégradé, le tremblement d'écran utilise le registre de défilement, le mouvement des convoyeurs est un **cycle de palette**.

## 2. Carte mémoire

```
$0000-$00FF  page zéro : registres cc65, variables NMI/manette, paramètres métasprites, son
$0100-$01FF  pile 6502
$0200-$02FF  copie OAM (DMA $4014 à chaque NMI)
$0300-$077F  variables C (BSS/DATA), file VRAM (128 o), palette (32 o), carte de salle (208 o)
$0780-$07FF  pile logicielle cc65 (les variables locales sont statiques : option -Cl)
$8000-$FFF9  code et données (NROM-256)
$FFFA-$FFFF  vecteurs NMI / RESET / IRQ
PPU $0000    tuiles de décor (police ASCII en $20-$5F)
PPU $1000    tuiles de sprites
```

## 3. Modules (§15 du cahier des charges)

| # | Module | Fichier(s) |
|---|---|---|
| 1 | Processeur et boucle principale | `src/crt0.s` (reset, NMI), `src/main.c` (machine à états, boucle de salle) |
| 2 | Commandes | `src/crt0.s` (`pad_poll`), `src/bidule.c` (`input_normal`) |
| 3 | BIDULE | `src/bidule.c` |
| 4 | TCHAK | `src/tchak.c` |
| 5 | Objets | `src/objects.c` (tables de propriétés, physique) |
| 6 | Interactions | `src/objects.c` (chocs objet/objet, objet/BIDULE) |
| 7 | Réactions en chaîne | `src/events.c` (déclencheurs, file d'actions, garde-fous) |
| 8 | Ennemis | `src/enemies.c` (le Trouillard) |
| 9 | Boss | `src/enemies.c` (`boss_update`, `boss_hit`) + déclencheur `BOSS` dans les données de salle |
| 10 | Salles | `src/room.c`, données `rooms/world1.txt` → `tools/rooms.py` |
| 11 | Mondes | `src/room.c` (palettes, musique), `src/menus.c` (présentation) |
| 12 | Collisions | `src/fast.s` (`coll_at`), `src/objects.c`, `src/bidule.c` |
| 13 | Graphismes | `src/video.c`, `src/fast.s` (OAM, métasprites), `tools/gfx.py`, `tools/art_bidule.py` |
| 14 | Son | `src/sound.s`, `tools/music.py` |
| 15 | Progression | `src/progress.c` (vies, chaussettes, mot de passe) |
| 16 | Menus | `src/menus.c` |

### Machine à états du jeu

```
TITRE ──START──> MENU ──COMMENCER──> PRÉSENTATION ──> SALLE ──gagnée──> PRÉSENTATION (salle suivante)
                   └──MOT DE PASSE──> SAISIE ──code valide──┘     │            └─ dernière salle ──> FIN
                                                                  └─perdue─> vie -1 ─> SALLE (ou FIN DE PARTIE)
```

Boucle d'une trame de jeu : `bidule_update` → `objects_update` (IA ennemis + physique + chocs) → `events_update` (plaques, file d'actions) → `room_update` (animations de tuiles, convoyeurs, tremblement) → construction de l'OAM → attente VBlank.

## 4. Système d'objets générique

Chaque objet est une ligne de tableaux (`o_type`, `o_x`, `o_y`, `o_vx`, `o_vy`, `o_state`, `o_flags`…, 8 objets maximum par salle). Son comportement vient de **tables de propriétés par type** (`objects.c`) :

| Propriété | Balle | Caisse | Trouillard | Chaussette |
|---|---|---|---|---|
| masse (chocs) | 1 | 4 | 2 | – |
| élasticité /8 | 6 | 0 | 0 | – |
| frottement | 1 | 4 | 4 | – |
| poids (plaques) | 1 | 3 | 2 | – |
| réaction au TCHAK (vx, vy) | 40, 56 | 30, 24 | 24, 32 + fuite | – |

États possibles d'un objet : immobile (`OF_REST`), en mouvement, au sol (`OF_GROUND`), déformé (animation d'écrasement `o_anim`), caché (`OF_HIDDEN`), détruit (`OF_DEAD` : tombé dans la gelée, il réapparaît à sa place 1,5 s plus tard — aucune salle ne peut devenir insoluble).

Les chocs entre objets utilisent une conservation de la quantité de mouvement en entiers (masses, coefficient de restitution 6/8 si un objet est élastique, sinon 2/8). Un objet qui tombe sur un autre s'y pose ; un objet qui monte cogne celui du dessus.

Optimisation : un objet immobile au sol est « endormi » ; il ne refait sa physique complète que s'il reçoit une vitesse, si son convoyeur tourne, ou une trame sur huit (pour remarquer qu'on lui a retiré le sol).

## 5. Réactions en chaîne et garde-fous

- **Déclencheurs** (par salle) : cible murale (touchée par un objet à ≥ 0,75 px/trame), plaque (poids ≥ 2, relâchée après 6 trames sans poids), levier (bouton A). Chacun a jusqu'à 2 actions « ON » et 1 action « OFF ».
- **Actions** : ouvrir/fermer une porte, démarrer/arrêter/inverser les convoyeurs, ouvrir/fermer une rangée de trappes (ou l'ouvrir 0,8 s : `HATCH_PULSE`), réveiller les ennemis, faire apparaître un objet. Un déclencheur `BOSS` lance ses actions ON à chaque coup reçu par le boss et OFF à sa défaite.
- Les actions passent par une **file avec 8 trames de délai** : le joueur voit l'ordre des conséquences.
- **Protection contre les boucles infinies** : recharge de 40 trames par déclencheur, file limitée à 8 actions, budget global de 16 actions régénéré d'une unité toutes les 8 trames. Une machine qui s'auto-entretient (la balle sur le ressort de la salle 1-2 tape la cible indéfiniment) ne peut donc jamais saturer le jeu.

## 6. Données de salle

Une salle est décrite en texte (`rooms/world1.txt`) : carte 16 × 13 en caractères, position de départ, objets, déclencheurs. `tools/rooms.py` vérifie la cohérence (dimensions, caractères connus, une sortie au moins, actions qui visent bien une porte ou une trappe) et produit `src/gen/rooms.c` (208 octets de carte par salle). 40 salles ≈ 9 Ko : elles tiendront en banques MMC1 (8 salles par banque de 16 Ko avec leur logique).

## 7. Moteur son

6 flux : musique sur carré 1, carré 2, triangle, bruit ; bruitages sur carré 2 et bruit (ils masquent la musique de ce canal le temps de l'effet). Format compact (note+durée, instrument, enveloppe de décroissance, glissement de fréquence, boucle). `tools/music.py` compile une notation texte (`C5.2 E5.2 r.1 …`). Le moteur est appelé dans le NMI : tempo stable même si la logique déborde.

Morceaux : thème principal (titre), Monde 1, boss, victoire, défaite, fin. Bruitages : TCHAK, ressort, choc, déclic, porte, surprise, plouf, chaussette, menu, validation, écrasement, panique, levier.

## 8. Compilation

```
sudo apt-get install cc65 python3-pil   # ou : pip install pillow
make            # -> build/tchak.nes
make test       # tests automatiques (pip install cynes pillow)
make dist       # copie la ROM dans dist/
```

Chaîne : `tools/gfx.py` → CHR + `src/gen/gfx.*` ; `tools/rooms.py` → `src/gen/rooms.*` ; `tools/music.py` → `src/gen/music.*` ; `cc65 -Oirs -Cl` → `ca65` → `ld65 -C cfg/nrom.cfg`. Le fichier `build/tchak.labels` sert aux tests (adresses des variables) et aux débogueurs (Mesen, FCEUX).
