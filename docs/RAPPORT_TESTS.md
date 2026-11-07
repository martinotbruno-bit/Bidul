# TCHAK ! – Rapport de tests (version 0.6 – Mondes 1 à 4, MMC1 256 Ko)

## Méthode

Les tests jouent réellement la ROM `build/tchak.nes` dans l'émulateur headless **cynes** : entrées manette scriptées, puis vérification de la RAM (adresses tirées de `build/tchak.labels`). Lancement : `make test` (ou `python3 tests/test_rom.py --shots` pour régénérer les captures de `docs/img/`).

## Résultats : 52 / 52 réussis (en 18 min d'émulation)

Détail brut : `docs/resultats_tests.txt`. Numérotation des salles : 1-1 à 4-8.

| Test | Vérifie |
|---|---|
| t01 | démarrage, logo animé, menu, présentation, entrée en salle 1-1 |
| t02, t03 | 1-1 : solution ; le TCHAK réflexe envoie la balle dans la gelée, elle revient ; hors de portée rien ne bouge |
| t20 | 1-2 : réflexe → gelée ; balle → puits → ressort → cibles → porte sur la corniche, atteinte en rebondissant (+ chaussette) |
| t21 | 1-3 : depuis le toit, le Trouillard fuit dans la gelée ; depuis sa gauche, il pousse la caisse au fond du tunnel |
| t04, t05, t06 | 1-4 : TCHAK au vol depuis le rebord ; TCHAK au mauvais moment → échec ; chaussette du ressort |
| t22 | 1-5 : BIDULE seul trop léger ; BIDULE + balle → trappe → caisse → porte |
| t07, t08, t09 | 1-6 : sans planche le Trouillard tombe dans la gelée ; solutions Trouillard et caisse |
| t23 | 1-7 : planche + balle/ressort/cibles/convoyeur + Trouillard effrayé du bon côté |
| t24 | 1-8 : le Roi tombe 3 fois (levier au bon moment, 3 phases), porte, écran de fin |
| t25 | 1-8 : sans levier, 30 s d'attente → il ne tombe jamais ; il aplatit BIDULE sans lui coûter de vie |
| t10, t11 | gelée : perte d'une vie ; plus de vies : fin de partie |
| t12 | pause / SELECT recommence |
| t13, t14 | mot de passe valide (salle 1-6) / invalide |
| t15 | déterminisme |
| t16 | écran de fin après le boss |
| t17 | aucune trame perdue dans la scène la plus chargée |
| t18 | musique et bruitages |
| t19 | **joueur qui ne comprend rien** : 6 parties de 20 s d'appuis aléatoires dans chacune des 8 salles → aucune salle résolue |
| t30–t33 | 2-1 à 2-4 : fauteuil (le mauvais côté → gelée), lampe (TCHAK sur la plaque), commode (1 TCHAK ne suffit pas, 3 du bon côté), coffre (dosage du biscuit) ; banque MMC1 du monde 2 sélectionnée |
| t34 | 2-5 : les trois serrures, une par humeur |
| t35 | 2-6 : deux solutions vérifiées (commode seule / fauteuil qui la pousse) |
| t36 | 2-7 : lampe + fauteuil d'un seul TCHAK, planche, commode |
| t37 | 2-8 : Madame Armoire ne se bat pas seule (30 s) ; solveur : 3 lancers visés → porte → écran de fin du monde 2 |
| t38 | Monde 2 : 4 parties de 20 s d'appuis aléatoires par salle → aucune salle résolue |
| t40 | 3-1 : réflexe → gelée ; balle → cible → piston → caisse → plaque ; banque MMC1 du monde 3 |
| t41 | 3-2 : sans miroir le Copieur tombe ; avec miroir il va sur la plaque |
| t42 | 3-3 : attendre le passage du Machinal ne permet pas de traverser ; le figer sur la plaque, oui |
| t43 | 3-4 : balle → inversion du convoyeur → caisse devant le piston → levier |
| t44 | 3-5 : pousser contre le mur ne fait pas avancer le Copieur ; le cliquet miroir ouvre la cage |
| t45 | 3-6 : deux solutions vérifiées (piston / TCHAK) |
| t46 | 3-7 : trois serrures |
| t47 | 3-8 : le Grand Dérangé ne tombe pas seul (20 s) ; vaincu par une stratégie automatique |
| t48 | Monde 3 : 4 parties de 20 s d'appuis aléatoires par salle → aucune salle résolue |
| t50 | 4-1 : inverser tout de suite colle la caisse au plafond à côté de la plaque ; la placer d'abord ; banque MMC1 du monde 4 |
| t51 | 4-2 : ballon poussé au sol (gravité inversée) sous la cheminée, puis retour à la normale |
| t52 | 4-3 : l'aimant seul ne suffit pas ; par le plafond, oui |
| t53 | 4-4 : trois serrures (plaque du sol, plaque du plafond, ballon) |
| t54 | 4-5 : zigzag (levier de l'aimant en haut de l'échelle + deux retournements) |
| t55 | 4-6 : l'aimant seul fait tomber la caisse dans la cuve ; par le plafond elle passe |
| t56 | 4-7 : le grand retournement |
| t57 | 4-8 : le Roi à l'Envers ne tombe pas seul (25 s) ; vaincu par une stratégie automatique |
| t58 | Monde 4 : 4 parties de 20 s d'appuis aléatoires par salle → aucune salle résolue |

### Équilibrage du boss (mesuré)

Fenêtres de réussite depuis le levier de droite : on part d'un instant donné, on actionne le levier après un délai variable, et on regarde si le Roi tombe.

| Phase | Délais qui réussissent (pas de 6 trames, sur 10 s) |
|---|---|
| 1 | 216–246 (une fenêtre d'environ 0,5 s) |
| 2 | 276–312 (environ 0,6 s) |
| 3 | 114–120, 234–240, 354–360, 480–486 (4 fenêtres d'environ 0,1 s, une toutes les 2 s : il faut viser) |

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
| v0.3 – salle 1-2 : la balle passait au-dessus d'un puits de 2 cases | puits de 3 cases contre le mur |
| v0.3 – salle 1-5 : impossible de passer de l'autre côté de la cuvette | deuxième échelle |
| v0.3 – boss : la trappe restait ouverte, le Roi finissait par tomber tout seul | trappe ouverte 0,8 s seulement ; levier ajouté à droite pour la phase 3 (il vise BIDULE) |
| v0.4 – passage au MMC1 : une banque tirait des routines cc65 absentes du moteur commun | détection automatique (make_rom.py) + `src/libforce.s` |
| v0.4 – écran noir en entrant dans le monde 2 | index de salle non ramené à 0-7 dans l'écran de présentation |
| v0.4 – une serrure se comptait plusieurs fois (la plaque verrouillée était « relâchée » puis redessinée) | plaque verrouillée ignorée ; animation de tuile annulée |
| v0.4 – le fauteuil poussait la commode contre la porte sans fin | son compteur de blocage partageait un champ avec l'animation d'écrasement : variables propres en RAM de banque |
| v0.4 – Madame Armoire se battait toute seule en 20 s (balle élastique, objets avalés en marchant) | elle n'avale que ce qu'on lui lance ; biscuits au lieu de la balle ; demi-tour quand elle est bloquée |
| v0.5 – l'écran de fin du Monde 2 affichait n'importe quoi (texte manquant, bogue de la v0.4) | textes de fin des mondes 2 et 3 |
| v0.5 – 3-3 : attendre devant la porte suffisait (le robot passe tout seul sur la plaque) | la plaque pose une passerelle temporaire sur un trou de 3 cases |
| v0.5 – le Copieur avançait quand BIDULE poussait contre un mur | il copie le déplacement réel, pas la commande |
| v0.5 – une cible de 2 cases comptait 2 serrures | cibles-serrures d'une seule case |
| v0.6 – moteur commun plein | sprites d'objets, logo et thèmes musicaux déplacés dans les banques ; ROM 256 Ko |
| v0.6 – 4-5 : résolue par le joueur au hasard (leviers collés au départ) | levier de l'aimant en haut d'une échelle (0 victoire sur 10 parties au hasard ensuite) |
| v0.6 – 4-5 : la caisse passait sous l'obstacle du plafond en montant | obstacles en colonnes |
| v0.6 – 4-4 : plaque du plafond trop étroite pour la précision du TCHAK (bonds de ~45 px) | plaque de 2 cases |
| v0.6 – boss 4-8 : BIDULE devait traverser un trou pour atteindre un levier | un seul levier, trous du sol et du plafond décalés |

## Limites connues (honnêtement)

- **Testé uniquement sur émulateur** (cynes ; captures visuellement contrôlées). Pas encore de test sur Mesen/FCEUX ni sur vraie console.
- Le **son** est vérifié par l'état du moteur (flux actifs, lecture qui avance), pas à l'oreille : l'émulateur de test ne restitue pas l'audio.
- Le test de déterminisme compare l'état de jeu, pas les minuteries musicales (l'alignement CPU/PPU à l'allumage peut les décaler d'une trame d'une instance d'émulateur à l'autre).
- Monde 5 non commencé (la ROM a déjà la place : banque 5 libre).
- Le message de victoire du boss du Monde 3 est devenu un « BOSS VAINCU ! » générique (place libérée dans le moteur).
- Le boss 3-8 est exigeant : rester immobile fait perdre toutes ses vies en environ 25 s (le convoyeur emporte BIDULE).
- Le pic de calcul dans la scène la plus chargée laisse peu de cycles libres sur 29 780 : les salles plus chargées (8 objets en mouvement) demanderont d'autres optimisations (physique en assembleur).
