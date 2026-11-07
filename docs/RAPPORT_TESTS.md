# TCHAK ! – Rapport de tests (prototype 0.2)

## Méthode

Les tests jouent réellement la ROM `build/tchak.nes` dans l'émulateur headless **cynes** : entrées manette scriptées, puis vérification de la RAM (adresses tirées de `build/tchak.labels`). Lancement : `make test` (ou `python3 tests/test_rom.py --shots` pour régénérer les captures de `docs/img/`).

## Résultats : 19 / 19 réussis

| Test | Vérifie | Résultat |
|---|---|---|
| t01 | démarrage, logo animé, menu, présentation, entrée en salle 1-1 | OK |
| t02 | salle 1-1 résolue depuis le bon côté de la balle, échelle, sortie | OK |
| t03 | salle 1-1 : le TCHAK réflexe envoie la balle dans la gelée, elle revient ; hors de portée rien ne bouge | OK |
| t04 | salle 1-2 : caisse sur le ressort, TCHAK au vol depuis le rebord, étagère, plaque, porte, sortie | OK |
| t05 | salle 1-2 : TCHAK au mauvais moment → la caisse continue de rebondir, porte fermée | OK |
| t06 | salle 1-2 : chaussette secrète en se laissant projeter par le ressort, et BIDULE peut en sortir | OK |
| t07 | salle 1-3 : sans planche, le Trouillard effrayé tombe dans la gelée et revient endormi | OK |
| t08 | salle 1-3 : levier puis Trouillard → plaque → porte → sortie | OK |
| t09 | salle 1-3 : levier puis TCHAK sur la caisse depuis la plate-forme → porte → sortie | OK |
| t10 | gelée : perte d'une vie et retour au départ | OK |
| t11 | plus de vies : fin de partie puis retour au titre | OK |
| t12 | pause qui fige le jeu ; SELECT recommence sans perte de vie | OK |
| t13 | mot de passe valide → reprise en salle 1-3 avec la chaussette | OK |
| t14 | mot de passe invalide refusé | OK |
| t15 | déterminisme : deux parties identiques → même état de jeu trame par trame | OK |
| t16 | écran de fin après la dernière salle | OK |
| t17 | aucune trame perdue dans la scène la plus chargée (salle 1-3 : caisse + Trouillard + planche) | OK |
| t18 | musique active sur 4 canaux, bruitage TCHAK lancé puis terminé | OK |
| t19 | **joueur qui ne comprend rien** : 8 parties de 20 s d'appuis aléatoires par salle, aucune salle résolue | OK |

Essai complémentaire (hors suite, plus long) : 12 parties de 30 s d'appuis aléatoires par salle → **0 victoire** dans chacune des 3 salles. Avant la version 0.2, appuyer une seule fois sur B au départ suffisait à résoudre la salle 1-2.

## Problèmes trouvés et corrigés pendant les tests

| Problème | Correction |
|---|---|
| Salle 1-1 : la balle frappait le mur au-dessus de la cible | cible agrandie à 2 cases ; une cible touchée par le bas de l'objet compte |
| Salle 1-2 : la balle ressortait du puits avec sa vitesse horizontale | règle « le ressort projette à la verticale » |
| Salles 1-2 et 1-3 : une caisse poussée fort atterrissait sur le haut de la porte | pilier au-dessus de la porte ; la caisse rebondit dans le trou |
| Salle 1-3 : BIDULE coincé sur la caisse contre le pilier | pilier raccourci d'une case |
| Le TCHAK était ignoré pendant l'atterrissage après une grosse chute | B interrompt l'atterrissage |
| SELECT en pause relançait la salle en boucle | entrée manette consommée après le redémarrage |
| Trames perdues (jusqu'à 12 pendant la chaîne de la salle 1-2) | sprites et métasprites en assembleur, collisions sur positions pré-calculées, objets au repos « endormis » → 0 trame perdue |
| Mémoire : BSS débordait de 11 octets | pile C réduite à 128 octets (variables locales statiques) |
| Le « Ï » de « AÏE » s'affichait « À » | glyphe Ï ajouté |
| **v0.2 – salles trop faciles** (retour joueur : « on résout sans rien comprendre ») | les 3 salles refaites : le geste évident échoue, la solution demande une règle (sens de la poussée, ressort vertical + timing, ordre levier → Trouillard) ; test t19 ajouté |
| Un objet poussé passait par une porte ouverte et s'incrustait dans le mur | une porte ouverte reste un obstacle pour les objets |
| Le Trouillard qui pousse une caisse bloquée courait sans fin (et ralentissait le jeu) | il s'essouffle s'il n'avance plus depuis 12 trames |

## Limites connues (honnêtement)

- **Testé uniquement sur émulateur** (cynes ; captures visuellement contrôlées). Pas encore de test sur Mesen/FCEUX ni sur vraie console.
- Le **son** est vérifié par l'état du moteur (flux actifs, lecture qui avance), pas à l'oreille : l'émulateur de test ne restitue pas l'audio.
- Le test de déterminisme compare l'état de jeu, pas les minuteries musicales (l'alignement CPU/PPU à l'allumage peut les décaler d'une trame d'une instance d'émulateur à l'autre).
- Pas encore de boss, ni de musique de boss : ils viendront avec la production des mondes.
- Le pic de calcul dans la scène la plus chargée laisse peu de cycles libres sur 29 780 : les salles plus chargées (8 objets en mouvement) demanderont d'autres optimisations (physique en assembleur).
