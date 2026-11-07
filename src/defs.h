/* =====================================================================
 *  TCHAK ! - definitions communes
 * ===================================================================== */
#ifndef DEFS_H
#define DEFS_H

typedef unsigned char u8;
typedef signed char   s8;
typedef unsigned int  u16;
typedef signed int    s16;

/* ---- types de collision des metatuiles -------------------------------- */
#define C_EMPTY   0
#define C_SOLID   1
#define C_SPRING  2
#define C_CONV    3
#define C_TARGET  4
#define C_PLATE   5
#define C_LADDER  6
#define C_HAZARD  7
#define C_EXIT    8
#define C_LEVER   9
#define IS_SOLID(c) ((u8)((c) - 1) < 5)

/* ---- manette (ordre de lecture NES) ----------------------------------- */
#define PAD_A      0x80
#define PAD_B      0x40
#define PAD_SELECT 0x20
#define PAD_START  0x10
#define PAD_UP     0x08
#define PAD_DOWN   0x04
#define PAD_LEFT   0x02
#define PAD_RIGHT  0x01

/* ---- geometrie : salles de 16 x 13 metatuiles sous un bandeau de 32 px -- */
#define MAP_W      16
#define MAP_H      13
#define MAP_SIZE   (MAP_W * MAP_H)
#define PLAY_TOP   32

/* positions en 1/16 de pixel (12.4) ; vitesses en 1/16 px/trame */
#define SUB(px)    ((u16)(px) << 4)
#define PIX(v)     ((u8)((v) >> 4))

/* ---- objets ----------------------------------------------------------- */
#define MAX_OBJ    8
#define OT_NONE    0
#define OT_BALL    1   /* balle molle : elastique, legere           */
#define OT_CRATE   2   /* caisse : lourde, glisse, se pose          */
#define OT_TROU    3   /* Trouillard : fuit le bruit, pousse        */
#define OT_SOCK    4   /* chaussette : bonus a ramasser             */
#define OT_KING    5   /* boss : le Roi du Rebond                  */
/* monde 2 : comportement programme dans la banque du monde (world2.c) */
#define OT_ARMCHAIR 6  /* fauteuil grognon : fonce vers le bruit    */
#define OT_LAMP    7   /* lampe curieuse : va voir d'ou vient le bruit */
#define OT_DRESSER 8   /* commode paresseuse : il faut insister     */
#define OT_CHEST   9   /* coffre gourmand : mange les biscuits      */
#define OT_COOKIE  10  /* biscuit                                   */
#define OT_WARDROBE 11 /* boss : Madame Armoire                     */
/* monde 3 : banque du monde 3 (world3.c) */
#define OT_PISTON  12  /* piston : projette ce qui est devant lui (action WORLD) */
#define OT_COPY    13  /* le Copieur : imite les pas de BIDULE (TCHAK : miroir) */
#define OT_ROBOT   14  /* le Machinal : patrouille ; TCHAK : pause puis demi-tour */
#define OT_DERANGE 15  /* boss : le Grand Derange                   */
/* monde 4 : banque du monde 4 (world4.c) */
#define OT_BALLOON 16  /* ballon : tombe vers le haut               */
#define OT_MAGNET  17  /* aimant : attire les caisses (action WORLD) */
#define OT_ENVERS  18  /* boss : le Roi a l'Envers                  */
#define OT_WORLD   6   /* types >= OT_WORLD : geres par la banque   */
#define OT_COUNT   19
#define OF_LEFT    0x10   /* (donnees de salle) objet tourne vers la gauche */
#define OF_FLOAT   0x20   /* objet qui tombe vers le haut (ballon) */

/* drapeaux d'objet */
#define OF_GROUND  0x01
#define OF_HIDDEN  0x02
#define OF_DEAD    0x04
#define OF_REST    0x08   /* au repos : physique allegee */

/* ---- declencheurs et actions ------------------------------------------ */
#define TK_TARGET  1   /* cible murale : touchee par un objet en mouvement */
#define TK_PLATE   2   /* plaque : enfoncee par un poids >= 2               */
#define TK_LEVER   3   /* levier : actionne par BIDULE (bouton A)           */
#define TK_BOSS    4   /* boss : ON a chaque coup recu, OFF a sa defaite    */

#define A_NONE        0
#define A_DOOR_OPEN   1
#define A_DOOR_CLOSE  2
#define A_CONV_RIGHT  3
#define A_CONV_LEFT   4
#define A_CONV_STOP   5
#define A_CONV_REV    6
#define A_HATCH_OPEN  7
#define A_HATCH_CLOSE 8
#define A_WAKE        9
#define A_REVEAL      10
#define A_WORLD       13   /* action propre au monde (banque) : parametre x */
#define A_LOCK        12   /* serrure : la case devient un bloc, compteur +1 */
#define A_HATCH_PULSE 11   /* trappe ouverte 1,3 s puis refermee */

/* ---- etats du jeu ----------------------------------------------------- */
#define GS_TITLE     0
#define GS_MENU      1
#define GS_PASSWORD  2
#define GS_INTRO     3
#define GS_PLAY      4
#define GS_CLEAR     5
#define GS_LOST      6
#define GS_GAMEOVER  7
#define GS_END       8

/* description d'une salle (donnees generees par tools/rooms.py) */
typedef struct {
    const u8  *map;      /* MAP_SIZE metatuiles                         */
    const char *name;    /* nom affiche                                 */
    u8  bx, by;          /* depart de BIDULE (pixels)                   */
    u8  nobj;
    const u8 *objs;      /* type, x, y, drapeaux                        */
    u8  ntrig;
    const u8 *trigs;     /* voir rooms.py : 14 octets par declencheur    */
    s8  conv;            /* etat initial des convoyeurs : -1, 0, +1     */
    u8  world, num;      /* monde 1-5, salle 1-8                        */
} Room;

#define TRIG_SIZE 14

#endif
