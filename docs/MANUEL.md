# TCHAK ! – La Grande Catastrophe · Manuel de jeu (prototype)

![titre](img/02_titre.png)

## L'histoire

Dans la Fabrique des Choses Molles, tout rebondit, tout glisse, tout tombe. BIDULE, qui ne sait même pas sauter, possède un seul talent : se contracter d'un coup — **TCHAK !** — et secouer tout ce qui l'entoure. Il ne contrôle pas vraiment ce qui se passe ensuite… mais avec un peu d'observation, la catastrophe peut devenir un plan.

## Les commandes

| Bouton | Action |
|---|---|
| Croix GAUCHE/DROITE | marcher (maintenir pour courir) |
| Croix HAUT/BAS | monter / descendre une échelle |
| **B** | **TCHAK !** |
| **A** | actionner un levier (se placer devant) |
| START | pause |
| SELECT | recommencer la salle (sans perdre de vie) |

Au menu : HAUT/BAS pour choisir, START ou A pour valider.

## L'écran

```
♥05      MONDE 1-2               ♪00
        (messages)
```
À gauche vos vies, au centre le monde et la salle, à droite les chaussettes trouvées.

## Comment jouer

- Le TCHAK pousse les objets proches **à l'opposé de BIDULE**, et vers le haut. Plus l'objet est près, plus il part vite. Trop loin, rien ne bouge.
- Les **cibles** vertes s'allument quand un objet les percute. Les **plaques** s'enfoncent sous un poids suffisant — BIDULE est trop maigre pour en enfoncer une seul !
- Les **ressorts** projettent tout ce qui tombe dessus tout droit vers le haut, BIDULE compris. Dirigez-vous pendant le vol.
- Le **Trouillard** dort. Faites du bruit et il s'enfuit droit devant, en poussant tout ce qu'il rencontre.
- Les **cuves de gelée** rose sont dangereuses pour BIDULE. Un objet qui y tombe réapparaît à sa place.
- Chaque salle a plusieurs solutions. Les **chaussettes** récompensent les plus curieux.

## Le mot de passe

Il est affiché sur l'écran de présentation de chaque salle (« CODE : »). Au menu, choisissez MOT DE PASSE, puis GAUCHE/DROITE pour changer de case, HAUT/BAS pour changer de lettre, START pour valider.

Codes du prototype : salle 1-1 `BNFR`, salle 1-2 `FNFS`, salle 1-3 `DNFT` (avec la chaussette de la salle 1-2 : `VNKT`).

## Conseils

- Si quelque chose tombe dans la gelée, ce n'est pas grave : observez, il reviendra.
- Le TCHAK part une fraction de seconde après l'appui : anticipez ce qui bouge.
- Avant d'effrayer quelqu'un, regardez où il va courir.

## Où jouer

Ouvrez `dist/tchak_prototype.nes` avec un émulateur NES (Mesen, FCEUX, Nestopia…). La ROM utilise le mapper 0 (NROM-256) et pourra être gravée sur une cartouche NROM ; elle n'a pas encore été essayée sur une vraie console.
