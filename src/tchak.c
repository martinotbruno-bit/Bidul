/* =====================================================================
 *  TCHAK ! - module 4 : gestion du TCHAK
 *
 *  Sequence (24 trames, voir bidule.c) :
 *    0-3  preparation (BIDULE se replie)
 *    4-8  contraction (il se met en boule)
 *    9    DECLENCHEMENT : onde de choc
 *    9-13 bras et jambes ecartes
 *    14-23 recul
 *
 *  Effets de l'onde (depuis le centre de BIDULE) :
 *   - zone de choc  : |dx| < 40 et |dy| < 28 px -> impulsion
 *       vx = sens * (tchak_vx[type] + (40 - |dx|))  (plus fort de pres)
 *       vy = -tchak_vy[type]                        (le monde 1 est elastique)
 *   - zone de bruit : |dx| < 80 et |dy| < 96 px -> le Trouillard s'enfuit
 * ===================================================================== */
#include "nes.h"
#include "game.h"

u8 tchak_fx;
s8 tk_dir;
u8 tk_adx, tk_ady;
u8 tchak_count;

#define SHOCK_X 40
#define SHOCK_Y 28
#define HEAR_X  80
#define HEAR_Y  96

void tchak_fire(void)
{
    u8 i, t, cx, cy, ox, oy, adx, ady;
    s8 dir;
    cx = PIX(b_x) + 8;
    cy = PIX(b_y) + 12;
    sfx_play(SFX_TCHAK);
    shake = 10;
    tchak_fx = 1;
    if (tchak_count < 99) ++tchak_count;

    for (i = 0; i < MAX_OBJ; ++i) {
        t = o_type[i];
        if (t == OT_NONE || t == OT_SOCK || (o_flags[i] & (OF_DEAD | OF_HIDDEN))) continue;
        ox = PIX(o_x[i]) + 8;
        oy = PIX(o_y[i]) + 8;
        if (ox > cx)      { adx = ox - cx; dir = 1; }
        else if (ox < cx) { adx = cx - ox; dir = -1; }
        else              { adx = 0; dir = b_face ? -1 : 1; }
        ady = (oy > cy) ? oy - cy : cy - oy;

        tk_dir = dir; tk_adx = adx; tk_ady = ady;
        if (t >= OT_WORLD) {            /* reaction programmee par le monde */
            bk_wtchak(i);
            continue;
        }
        if (t == OT_TROU && adx < HEAR_X && ady < HEAR_Y) enemy_scare(i, dir);
        tchak_impulse(i);
    }
}

/* impulsion de l'onde de choc sur l'objet i (tk_dir/tk_adx/tk_ady remplis) */
void tchak_impulse(u8 i)
{
    u8 v, t = o_type[i];
    if (tk_adx >= SHOCK_X || tk_ady >= SHOCK_Y) return;
    v = p_tchak_vx[t] + (SHOCK_X - tk_adx);
    if (v > 96) v = 96;
    o_vx[i] = (tk_dir < 0) ? -(s8)v : (s8)v;
    o_vy[i] = -(s8)p_tchak_vy[t];
    o_flags[i] &= ~OF_GROUND;
    o_anim[i] = 8;
    o_dir[i] = tk_dir;
}

/* onde de choc : 4 quarts de cercle qui s'eloignent */
void tchak_draw(void)
{
    u8 x, y, r, tile;
    if (!tchak_fx) return;
    r = tchak_fx * 3;
    tile = (tchak_fx < 5) ? FX_WAVE1 : FX_WAVE2;
    x = PIX(b_x) + 8;
    y = PIX(b_y) + 12;
    oam_spr(x - r - 8, y - r - 8, tile, 0x03);
    oam_spr(x + r,     y - r - 8, tile, 0x43);
    oam_spr(x - r - 8, y + r,     tile, 0x83);
    oam_spr(x + r,     y + r,     tile, 0xC3);
    if (++tchak_fx > 12) tchak_fx = 0;
}
