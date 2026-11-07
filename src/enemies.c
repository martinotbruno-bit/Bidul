/* =====================================================================
 *  TCHAK ! - module 8 : gestion des ennemis
 *
 *  LE TROUILLARD (monde 1) : petite creature peureuse.
 *   - DORT tant que rien ne se passe ;
 *   - un TCHAK entendu (ou un choc) l'EFFRAIE : il sursaute ("!") ;
 *   - puis il COURT droit devant lui, a l'oppose du bruit, en poussant
 *     tout ce qu'il rencontre ;
 *   - arrete par un mur (ou bloque), il S'ESSOUFFLE puis se rendort.
 *  Comportement simple, lisible et donc exploitable par le joueur.
 * ===================================================================== */
#include "nes.h"
#include "game.h"

#define TROU_SPEED 20

void enemy_scare(u8 i, s8 dir)
{
    if (o_state[i] == TS_SLEEP || o_state[i] == TS_PANT) {
        o_state[i] = TS_ALERT;
        o_timer[i] = 18;
        o_dir[i] = dir;
        o_vy[i] = -32;
        sfx_play(SFX_PANIC);
    }
}

void enemies_wake(s8 dir)
{
    u8 i;
    for (i = 0; i < MAX_OBJ; ++i)
        if (o_type[i] == OT_TROU && !(o_flags[i] & (OF_DEAD | OF_HIDDEN))) enemy_scare(i, dir);
}

void enemy_update(u8 i)
{
    u8 px;
    switch (o_state[i]) {
    case TS_SLEEP:
        break;
    case TS_ALERT:
        o_vx[i] = 0;
        if (--o_timer[i] == 0) {
            o_state[i] = TS_RUN;
            o_timer[i] = 0;
            o_aux[i] = PIX(o_x[i]);
        }
        break;
    case TS_RUN:
        /* arrete par un mur, ou s'il n'avance plus depuis 12 trames
           (il pousse un objet bloque) : o_aux = position la plus avancee */
        px = PIX(o_x[i]);
        if (o_dir[i] < 0 ? (px < o_aux[i]) : (px > o_aux[i])) { o_aux[i] = px; o_timer[i] = 0; }
        else ++o_timer[i];
        if (o_wallhit[i] || o_timer[i] > 12) {
            o_state[i] = TS_PANT;
            o_timer[i] = 150;
            o_vx[i] = 0;
            break;
        }
        o_vx[i] = (o_dir[i] < 0) ? -TROU_SPEED : TROU_SPEED;
        break;
    case TS_PANT:
        o_vx[i] = 0;
        if (--o_timer[i] == 0) o_state[i] = TS_SLEEP;
        break;
    }
}
