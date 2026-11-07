# CAHIER DES CHARGES COMPLET
# TCHAK ! – LA GRANDE CATASTROPHE

**Version :** 1.0 – Document de référence  
**Type de projet :** Jeu vidéo rétro 8 bits  
**Plateforme cible :** Nintendo Entertainment System (NES)  
**Période esthétique :** 1990–1992  
**Genre :** Puzzle / Réflexion / Aventure / Humour absurde  
**Statut :** Spécification initiale de production

---

# 1. MISSION DE L'IA

Tu es chargé d'agir comme une équipe complète de développement de jeux vidéo rétro comprenant :

- Un directeur créatif.
- Un game designer.
- Un programmeur spécialisé NES.
- Un concepteur de niveaux.
- Un graphiste pixel art 8 bits.
- Un compositeur de musique chiptune.
- Un spécialiste de l'ergonomie et du gameplay.

Ta mission est de concevoir et réaliser un jeu vidéo complet intitulé :

**TCHAK ! – LA GRANDE CATASTROPHE**

Le résultat doit être un véritable jeu jouable, cohérent, amusant, techniquement fonctionnel et fidèle aux capacités matérielles de la Nintendo NES.

Il ne s'agit pas de créer une simple démonstration graphique, une maquette ou un prototype superficiel.

Le projet doit aboutir à un jeu complet, avec ses menus, ses personnages, ses mécaniques, ses niveaux, ses boss, ses musiques, ses effets sonores et sa progression.

Toutes les décisions de conception devront respecter l'identité originale du jeu.

# 2. VISION GÉNÉRALE DU JEU

TCHAK ! est un jeu de réflexion et d'aventure complètement loufoque.

Le joueur incarne BIDULE, une créature étrange et maladroite possédant une capacité physique extraordinaire : provoquer une contraction brutale de son corps qui déclenche une onde de choc.

Cette onde perturbe les objets, les créatures et les mécanismes présents dans les salles.

Mais ses effets dépendent du monde dans lequel évolue BIDULE.

Une simple action peut provoquer une succession de réactions inattendues :

- Un objet se met à rebondir.
- Il percute un autre objet.
- Celui-ci déclenche un mécanisme.
- Un personnage se met à courir.
- Il provoque une nouvelle réaction.
- Une porte s'ouvre ou un passage se libère.

Le joueur doit apprendre à comprendre ces phénomènes et à les exploiter.

**Le principe fondamental : le joueur déclenche les catastrophes, mais ne contrôle pas directement toutes leurs conséquences.**

Les événements doivent toutefois être déterministes et compréhensibles. Une même situation, avec les mêmes conditions initiales et les mêmes actions, doit produire le même résultat.

Le plaisir doit venir de la découverte, de l'expérimentation et de la compréhension progressive des règles.

# 3. IDENTITÉ VISUELLE ET DIRECTION ARTISTIQUE

Le jeu doit donner l'impression d'être une véritable production Nintendo NES de 1992.

## 3.1 Graphismes

- Pixel art authentique.
- Résolution native : 256 × 240 pixels.
- Palette de couleurs compatible NES.
- Sprites de petite taille.
- Décors constitués de tuiles graphiques.
- Animations expressives.
- Aucun effet graphique moderne.
- Aucun dégradé moderne.
- Aucun rendu 3D.
- Aucun anticrénelage.
- Aucun graphisme vectoriel.

Les personnages doivent être immédiatement reconnaissables malgré le nombre limité de pixels.

Les animations doivent être humoristiques et expressives.

## 3.2 Ambiance générale

Le jeu doit être :

- Très coloré.
- Déjanté.
- Fantaisiste.
- Imprévisible en apparence.
- Visuellement lisible.
- Attachant.
- Parfois complètement absurde.

Le décor doit raconter quelque chose même sans dialogue.

## 3.3 Interface

L'interface doit être minimaliste.

Afficher uniquement les informations nécessaires :

- Nombre de vies.
- Monde actuel.
- Salle actuelle.
- Objets ou capacités disponibles si nécessaire.
- Indicateurs visuels de progression.

Ne jamais encombrer l'écran.

# 4. PERSONNAGE PRINCIPAL : BIDULE

BIDULE est le héros du jeu.

Son nom est provisoire et pourra être modifié ultérieurement.

## 4.1 Apparence

BIDULE doit être une créature totalement improbable.

Caractéristiques :

- Petite tête disproportionnée.
- Corps relativement petit.
- Bras très longs.
- Jambes disproportionnées.
- Genoux énormes.
- Pieds ridiculement grands.
- Silhouette asymétrique.
- Expression constamment étonnée.

Son apparence doit permettre de le reconnaître immédiatement.

## 4.2 Personnalité

BIDULE est :

- Maladroit.
- Inconscient.
- Curieux.
- Naïf.
- Imprévisible.
- Très expressif.

Il ne comprend pas toujours les conséquences de ses actions.

Il ne doit pas être représenté comme un héros classique ou un personnage sérieux.

L'humour repose principalement sur ses animations et ses réactions.

## 4.3 Animations obligatoires

Créer les animations suivantes :

1. Position immobile.
2. Marche.
3. Déplacement rapide.
4. Observation.
5. Préparation du TCHAK.
6. Contraction du corps.
7. Déclenchement du TCHAK.
8. Recul après utilisation.
9. Surprise.
10. Chute.
11. Victoire.
12. Défaite.
13. Réaction humoristique face à un événement inattendu.

Chaque animation doit avoir une personnalité propre.

# 5. MÉCANIQUE PRINCIPALE : LE TCHAK

C'est la mécanique la plus importante de tout le jeu.

Elle doit être simple à utiliser, mais suffisamment profonde pour proposer des dizaines de situations différentes.

## 5.1 Fonctionnement

Lorsque le joueur déclenche le TCHAK :

1. BIDULE se replie sur lui-même.
2. Son corps se contracte.
3. Une animation humoristique accompagne le mouvement.
4. Une onde de choc est déclenchée.
5. Les objets présents dans la zone d'influence réagissent.
6. Les réactions peuvent provoquer d'autres événements.

## 5.2 Commande

Le bouton B déclenche le TCHAK.

Le bouton A permet une action contextuelle si le niveau le nécessite.

La croix directionnelle permet de se déplacer.

START permet de mettre le jeu en pause.

SELECT peut être utilisé pour les options ou une fonction secondaire si nécessaire.

La jouabilité doit rester très simple.

## 5.3 Règles de fonctionnement

Le TCHAK ne doit pas être une simple attaque.

Il doit être un outil de manipulation indirecte du monde.

Ses effets varient selon les mondes.

Les objets possèdent différents états :

- Immobile.
- En mouvement.
- Activé.
- Désactivé.
- Bloqué.
- Déformé.
- En interaction.
- Détruit si le scénario le permet.

Les réactions doivent pouvoir se combiner.

## 5.4 Règle fondamentale

Ne jamais générer des événements aléatoires sans logique.

Le joueur doit pouvoir découvrir les règles et les utiliser intelligemment.

La surprise doit venir de la combinaison des éléments, pas d'un hasard injuste.

# 6. STRUCTURE DU JEU

Le jeu comprend :

- 5 mondes principaux.
- 8 salles par monde.
- 40 salles principales.
- 5 boss.
- Un niveau final.
- Un écran de fin.

Les salles sont principalement des écrans fixes.

Chaque salle doit proposer un objectif clair : atteindre une sortie, activer un mécanisme ou résoudre une situation.

La difficulté augmente progressivement.

Certaines salles doivent être très simples pour apprendre une nouvelle règle.

D'autres doivent combiner plusieurs mécaniques déjà découvertes.

Chaque salle doit proposer idéalement plusieurs solutions.

Les solutions alternatives peuvent donner accès à des secrets, des bonus ou des raccourcis.

# 7. DESCRIPTION DES CINQ MONDES

## MONDE 1 : LA FABRIQUE DES CHOSES MOLLES

Thème : usine de fabrication de produits étranges.

Couleurs dominantes : jaune, rose, orange, vert.

Décors :

- Convoyeurs.
- Caisses.
- Machines.
- Réservoirs.
- Ressorts.
- Portes mécaniques.

Nouvelle mécanique : les objets sont élastiques.

Le TCHAK provoque des rebonds.

Le joueur apprend à utiliser les propriétés des objets pour déclencher des réactions.

Objectifs pédagogiques :

- Comprendre le fonctionnement du TCHAK.
- Apprendre à observer les objets.
- Comprendre les interactions.
- Découvrir les réactions en chaîne.

Boss : LE ROI DU REBOND.

Ce boss rebondit dans toute la pièce.

Le joueur doit utiliser les mécanismes de la salle pour le neutraliser.

## MONDE 2 : LA MAISON QUI N'EN FAIT QU'À SA TÊTE

Thème : maison complètement vivante.

Décors :

- Meubles animés.
- Portes vivantes.
- Tables.
- Armoires.
- Escaliers.
- Lampes.
- Horloges.

Nouvelle mécanique : les objets possèdent des comportements et des humeurs.

Certains objets peuvent être :

- Peureux.
- Paresseux.
- Colériques.
- Curieux.
- Gourmands.

Le joueur doit comprendre leur comportement.

Le TCHAK peut provoquer des réactions émotionnelles.

Boss : MADAME ARMOIRE.

Elle avale des objets et les recrache ailleurs.

Le joueur doit exploiter ses comportements pour progresser.

## MONDE 3 : L'USINE DES CATASTROPHES

Thème : grande usine mécanique.

Décors :

- Engrenages.
- Courroies.
- Tuyaux.
- Pistons.
- Machines.
- Interrupteurs.
- Rouages.

Nouvelle mécanique : les réactions en chaîne complexes.

Un objet peut activer plusieurs mécanismes.

Les événements doivent pouvoir s'enchaîner.

Le joueur doit comprendre l'ordre des réactions.

Boss : LE GRAND DÉRANGÉ.

Il perturbe le fonctionnement des machines.

Le joueur doit utiliser les mécanismes contre lui.

## MONDE 4 : LE MONDE QUI MARCHE DE TRAVERS

Thème : univers surréaliste.

Décors :

- Plateformes inversées.
- Escaliers suspendus.
- Objets flottants.
- Pièces retournées.
- Passages étranges.

Nouvelle mécanique : les règles physiques sont inversées ou modifiées.

Exemples :

- Certains objets montent au lieu de tomber.
- Les déplacements peuvent être inversés.
- Les objets peuvent être attirés dans des directions inhabituelles.
- Les mécanismes peuvent fonctionner à l'envers.

Le joueur doit s'adapter aux nouvelles règles.

Boss : LE ROI À L'ENVERS.

Son comportement dépend des règles physiques de la salle.

## MONDE 5 : LE GRAND BAZAR

Thème : mélange de tous les univers précédents.

Décors :

- Machines folles.
- Meubles vivants.
- Objets élastiques.
- Passages inversés.
- Créatures absurdes.

Nouvelle mécanique : combinaison des mécaniques précédentes.

Le joueur doit maîtriser les différentes règles.

Les salles deviennent particulièrement complexes.

Boss final : LE MAÎTRE DU CHAOS.

Il peut modifier les propriétés des objets et déclencher des réactions en chaîne.

Le combat final doit être une grande énigme interactive.

La victoire doit être obtenue par compréhension et manipulation de l'environnement, pas par un simple affrontement classique.

# 8. CONCEPTION DES 40 SALLES

Chaque monde doit contenir 8 salles.

Répartition recommandée :

1. Salle d'introduction.
2. Salle d'apprentissage.
3. Salle de découverte.
4. Salle de combinaison.
5. Salle de difficulté intermédiaire.
6. Salle avec plusieurs solutions.
7. Salle complexe.
8. Salle de boss.

Chaque salle doit disposer d'une fiche de conception contenant :

- Nom.
- Objectif.
- Dimensions.
- Disposition des objets.
- Objets interactifs.
- Comportement des ennemis.
- Mécanique principale.
- Solutions possibles.
- Réactions en chaîne.
- Conditions de réussite.
- Conditions d'échec.
- Éléments secrets éventuels.

Les solutions ne doivent pas être évidentes immédiatement.

Le joueur doit expérimenter.

Éviter de répéter exactement les mêmes énigmes.

Chaque salle doit introduire une idée ou une combinaison intéressante.

# 9. ENNEMIS

Les ennemis ne doivent pas être de simples obstacles.

Ils doivent être intégrés dans les mécaniques du jeu.

Exemples :

- Créature qui court lorsqu'elle entend un bruit.
- Personnage qui pousse les objets.
- Animal qui mange certains éléments.
- Gardien qui protège un interrupteur.
- Créature qui imite les mouvements.
- Ennemi qui déclenche involontairement des mécanismes.

Les ennemis doivent avoir des comportements simples, lisibles et exploitables.

Leur comportement doit être prévisible après observation.

Les combats sont secondaires par rapport aux énigmes.

# 10. BOSS

Il faut cinq affrontements principaux.

Chaque boss doit avoir :

- Une identité graphique forte.
- Une animation particulière.
- Plusieurs comportements.
- Une mécanique spécifique.
- Une solution basée sur l'environnement.
- Une séquence de victoire humoristique.

Le joueur ne doit pas simplement appuyer rapidement sur un bouton pour gagner.

Chaque affrontement doit être une énigme à part entière.

# 11. PROGRESSION ET REJOUABILITÉ

Le joueur doit avoir envie de refaire les salles.

Pour cela :

- Plusieurs solutions possibles.
- Des passages secrets.
- Des objets cachés.
- Des solutions alternatives.
- Des récompenses facultatives.
- Des défis supplémentaires.

Les salles doivent rester accessibles après leur découverte afin de permettre leur rejouabilité.

Prévoir un système de codes de sauvegarde ou de mots de passe adapté à la NES pour reprendre sa progression.

# 12. MUSIQUE ET EFFETS SONORES

Créer une bande sonore originale compatible avec l'APU de la NES.

Ambiance musicale :

- Joyeuse.
- Loufoque.
- Rythmée.
- Mémorable.
- Très années 90.

Prévoir :

- Un thème principal.
- Une musique par monde.
- Une musique de boss.
- Une musique de victoire.
- Une musique de défaite.
- Une musique de fin.

Effets sonores :

- Déclenchement du TCHAK.
- Réaction des objets.
- Rebond.
- Collision.
- Activation d'interrupteur.
- Ouverture de porte.
- Surprise de BIDULE.
- Réactions des ennemis.
- Victoire contre un boss.

Le son du TCHAK doit être immédiatement reconnaissable.

# 13. INTERFACE ET MENUS

Créer les écrans suivants :

1. Écran titre avec logo animé.
2. Menu principal.
3. Écran de sélection de progression.
4. Présentation des mondes.
5. Écran de jeu.
6. Écran de pause.
7. Écran de victoire.
8. Écran de défaite.
9. Écran de fin.

Le logo TCHAK ! doit avoir une animation amusante.

BIDULE doit apparaître dès l'écran titre.

Les menus doivent être simples et fidèles aux jeux NES.

# 14. SPÉCIFICATIONS TECHNIQUES NES

La cible est une véritable Nintendo Entertainment System.

Contraintes à respecter :

- CPU Ricoh 2A03 / architecture 6502.
- Fréquence CPU NTSC d'environ 1,79 MHz.
- PPU 2C02.
- Résolution de 256 × 240 pixels.
- 2 Ko de RAM interne.
- 2 Ko de mémoire vidéo interne pour les nametables, selon le mapper et le mirroring.
- Palette NES.
- Maximum de 64 sprites matériels.
- Maximum de 8 sprites par ligne de balayage.
- Sprites de 8 × 8 ou 8 × 16 pixels.
- Audio APU à cinq canaux principaux.

Ne pas dépasser les contraintes matérielles.

Éviter les calculs physiques coûteux.

Les interactions doivent être programmées sous forme de règles prédéfinies.

Utiliser des états et des événements.

Prévoir un système de gestion des objets et des réactions en chaîne.

Architecture recommandée :

- Langage assembleur 6502 ou C compatible cc65.
- Outils de compilation compatibles NES.
- Organisation du code par modules.
- Gestion séparée des graphismes, du son, des salles et des mécaniques.
- Mapper adapté au contenu du jeu, par exemple MMC1 ou UNROM selon les besoins.
- CHR-ROM ou CHR-RAM selon l'architecture retenue.

Prévoir une ROM finale au format NES standard iNES ou NES 2.0.

Le jeu doit pouvoir fonctionner sur émulateur NES et, après validation matérielle, sur une vraie console.

# 15. ARCHITECTURE LOGICIELLE

Organiser le code en modules indépendants :

1. Gestion du processeur et de la boucle principale.
2. Gestion des commandes.
3. Gestion de BIDULE.
4. Gestion du TCHAK.
5. Gestion des objets.
6. Gestion des interactions.
7. Gestion des réactions en chaîne.
8. Gestion des ennemis.
9. Gestion des boss.
10. Gestion des salles.
11. Gestion des mondes.
12. Gestion des collisions.
13. Gestion des graphismes.
14. Gestion du son.
15. Gestion de la progression.
16. Gestion des menus.

Créer un système générique de propriétés pour les objets.

Exemple :

Un objet possède :
- Un type.
- Une position.
- Un état.
- Une propriété physique.
- Une réaction au TCHAK.
- Une réaction à d'autres objets.
- Une animation.

Le système doit pouvoir traiter plusieurs réactions successives.

Prévoir une protection contre les boucles infinies de réactions.

# 16. ERGONOMIE ET JOUABILITÉ

Le jeu doit être facile à comprendre.

Les commandes doivent être immédiatement accessibles.

Le joueur doit pouvoir :

- Se déplacer précisément.
- Déclencher le TCHAK facilement.
- Observer les conséquences.
- Recommencer une salle rapidement.
- Comprendre les objectifs.
- Identifier les éléments interactifs.

Éviter les temps d'attente inutiles.

Les animations doivent être suffisamment rapides pour ne pas ralentir artificiellement le jeu.

Le joueur doit toujours comprendre pourquoi une action a échoué.

# 17. DIFFICULTÉ

Prévoir une progression intelligente.

Monde 1 : découverte des bases.

Monde 2 : découverte des comportements.

Monde 3 : maîtrise des réactions en chaîne.

Monde 4 : adaptation aux nouvelles règles.

Monde 5 : combinaison de toutes les mécaniques.

Le jeu doit être exigeant, comme les bons jeux NES de l'époque.

Mais il ne doit jamais être injuste.

Les énigmes doivent être difficiles parce qu'elles demandent de réfléchir, pas parce que les commandes sont imprécises.

# 18. EXIGENCES DE QUALITÉ

Le résultat doit répondre aux critères suivants :

- Le jeu doit être réellement jouable.
- Les commandes doivent fonctionner correctement.
- Toutes les salles doivent être terminables.
- Aucun blocage permanent ne doit être possible.
- Les collisions doivent être fiables.
- Les réactions doivent être cohérentes.
- Les animations doivent être fluides.
- Les musiques doivent fonctionner.
- Les menus doivent être opérationnels.
- La progression doit être fonctionnelle.
- Les boss doivent être terminables.
- La ROM doit être testée.

Aucune salle ne doit être simplement décorative.

Aucun mécanisme ne doit être présent sans utilité ou fonction identifiable.

# 19. MÉTHODE DE DÉVELOPPEMENT

Développer le projet par étapes.

**PHASE 1 – Préproduction**

- Définir précisément BIDULE.
- Finaliser le fonctionnement du TCHAK.
- Définir les règles d'interaction.
- Définir les objets.
- Définir les ennemis.
- Définir la direction artistique.
- Établir le document technique.

**PHASE 2 – Prototype**

Créer trois salles :

- Une salle simple.
- Une salle avec réactions en chaîne.
- Une salle avec plusieurs solutions.

Tester le plaisir de jeu avant de continuer.

**PHASE 3 – Moteur**

Développer :

- Déplacements.
- Collisions.
- TCHAK.
- Interactions.
- Gestion des objets.
- Gestion des événements.
- Gestion des salles.

**PHASE 4 – Production**

Créer les cinq mondes, les quarante salles et les cinq boss.

**PHASE 5 – Graphismes et audio**

Créer tous les sprites, décors, animations, musiques et effets sonores.

**PHASE 6 – Tests**

Tester chaque salle, chaque interaction et chaque progression.

Corriger les bugs et équilibrer la difficulté.

**PHASE 7 – Livraison**

Produire la ROM finale et les sources du projet.

# 20. LIVRABLES ATTENDUS

À la fin du projet, fournir :

1. Le jeu complet sous forme de ROM NES.
2. Les fichiers sources.
3. Les graphismes et sprites.
4. Les musiques et effets sonores.
5. Les 40 salles terminées.
6. Les 5 boss terminés.
7. Le système de progression.
8. Les menus complets.
9. La documentation technique.
10. Le manuel de jeu.
11. Un rapport de tests.
12. Les instructions de compilation.

Le projet doit être organisé, documenté et modifiable.

# 21. RÈGLES ABSOLUES DE CONCEPTION

Ces règles ne doivent jamais être sacrifiées.

**RÈGLE 1 : L'ORIGINALITÉ**

TCHAK ! doit avoir une identité propre. Ne pas copier les mécaniques d'autres jeux.

**RÈGLE 2 : L'HUMOUR**

L'humour absurde doit être intégré dans les animations, les interactions et les situations.

**RÈGLE 3 : L'INTELLIGENCE**

Chaque salle doit demander de comprendre ou d'expérimenter quelque chose.

**RÈGLE 4 : LA SURPRISE**

Les interactions doivent provoquer des situations inattendues, mais cohérentes.

**RÈGLE 5 : LA REJOUABILITÉ**

Proposer plusieurs solutions aux énigmes.

**RÈGLE 6 : LA SIMPLICITÉ**

Les commandes doivent être simples et le gameplay profond.

**RÈGLE 7 : LA FIDÉLITÉ À LA NES**

Ne jamais concevoir un élément qui dépasse les possibilités matérielles de la console.

**RÈGLE 8 : LE PLAISIR**

Le jeu doit rester amusant, même lorsqu'un joueur échoue.

# 22. INSTRUCTIONS FINALES À L'IA

Tu dois maintenant prendre ce cahier des charges comme document de référence principal.

Ne réduis pas le projet à une démonstration.

Ne remplace pas le gameplay par une succession d'animations.

Ne crée pas de salles répétitives.

Ne simplifie pas les mécaniques au point de perdre l'originalité du jeu.

Ne commence pas par produire quarante salles sans avoir validé la mécanique principale.

Commence par établir une architecture technique complète et un prototype fonctionnel de trois salles.

Présente les choix techniques, puis réalise le prototype.

Fais tester les mécaniques, corrige les problèmes, puis développe progressivement le jeu entier.

Si une fonctionnalité n'est pas réalisable sur NES, propose une alternative compatible avec le matériel, sans supprimer l'intention de gameplay.

Ne prétends jamais qu'une fonctionnalité est terminée tant qu'elle n'a pas été programmée et testée.

**OBJECTIF FINAL :**

Créer un véritable jeu NES qui aurait pu sortir en 1992 et devenir un classique culte grâce à son personnage improbable, son humour absurde et surtout son système de réactions en chaîne totalement original.

Le joueur doit constamment se dire :

« C'est complètement idiot… mais qu'est-ce que c'est génial ! »

**FIN DU CAHIER DES CHARGES.**