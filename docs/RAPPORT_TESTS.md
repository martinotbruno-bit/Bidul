# TCHAK ! – Rapport de tests (prototype 0.1)

## Méthode

Les tests jouent réellement la ROM `build/tchak.nes` dans l'émulateur headless **cynes** : entrées manette scriptées, puis vérification de la RAM (adresses tirées de `build/tchak.labels`). Lancement : `make test` (ou `python3 tests/test_rom.py --shots` pour régénérer les captures de `docs/img/`).

## Résultats : 18 / 18 réussis

| Test | Vérifie | Résultat |
|---|---|---|
| t01 | démarrage, logo animé, menu, présentation, entrée en salle 1-1 | OK |
| t02 | salle 1-1 résolue (balle → cible → porte → sortie) | OK |
| t03 | TCHAK hors de portée : rien ne bouge | OK |
| t04 | salle 1-2, chaîne complète balle → ressort → cibles → convoyeur → caisse → plaque → porte | OK |
| t05 | salle 1-2, solution alternative (échelle + TCHAK sur la caisse) | OK |
| t06 | salle 1-2, chaussette secrète du puits, et BIDULE peut ressortir du puits | OK |
| t07 | salle 1-3, solution Trouillard | OK |
| t08 | salle 1-3, solution levier (le Trouillard reste endormi) | OK |
| t09 | salle 1-3, solution ressort + TCHAK sur la caisse (+ chaussette) | OK |
| t10 | gelée : perte d'une vie et retour au départ | OK |
| t11 | plus de vies : fin de partie puis retour au titre | OK |
| t12 | pause qui fige le jeu ; SELECT recommence sans perte de vie | OK |
| t13 | mot de passe valide → reprise en salle 1-3 avec la chaussette | OK |
| t14 | mot de passe invalide refusé | OK |
| t15 | déterminisme : deux parties identiques → même état de jeu trame par trame | OK |
| t16 | écran de fin après la dernière salle | OK |
| t17 | aucune trame perdue pendant une réaction en chaîne | OK |
| t18 | musique active sur 4 canaux, bruitage TCHAK lancé puis terminé | OK |

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

## Limites connues (honnêtement)

- **Testé uniquement sur émulateur** (cynes ; captures visuellement contrôlées). Pas encore de test sur Mesen/FCEUX ni sur vraie console.
- Le **son** est vérifié par l'état du moteur (flux actifs, lecture qui avance), pas à l'oreille : l'émulateur de test ne restitue pas l'audio.
- Le test de déterminisme compare l'état de jeu, pas les minuteries musicales (l'alignement CPU/PPU à l'allumage peut les décaler d'une trame d'une instance d'émulateur à l'autre).
- Pas encore de boss, ni de musique de boss : ils viendront avec la production des mondes.
- Le pic de calcul pendant une chaîne laisse environ 3 000 cycles libres sur 29 780 : les salles plus chargées (8 objets en mouvement) demanderont d'autres optimisations (physique en assembleur).
