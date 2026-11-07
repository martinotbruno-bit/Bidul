/* =====================================================================
 *  TCHAK ! - MONDE 2 : LA MAISON QUI N'EN FAIT QU'A SA TETE
 *  (banque MMC1 du monde 2 : appele par la table de sauts bankhdr_w2.s)
 *
 *  Ici, le TCHAK ne pousse pas les meubles : il les fait REAGIR selon leur
 *  humeur. Regles (toutes deterministes) :
 *
 *   FAUTEUIL GROGNON (colerique) : entend le TCHAK (96 px) -> fonce VERS
 *     BIDULE jusqu'au premier obstacle, en poussant tout ; il aplatit BIDULE
 *     s'il le touche. Puis il ronchonne et se rendort.
 *   LAMPE CURIEUSE : entend le TCHAK (128 px) -> va se placer exactement la
 *     ou BIDULE a fait TCHAK, puis s'immobilise.
 *   COMMODE PARESSEUSE : 1 TCHAK = "?", 2 = "?!", il en faut 3 rapproches
 *     (l'agacement retombe d'un cran toutes les 1,5 s) -> elle s'enfuit a
 *     l'oppose de BIDULE jusqu'au premier obstacle.
 *   COFFRE GOURMAND : ignore le TCHAK ; marche vers le biscuit le plus proche
 *     a son etage et le mange.
 *   BISCUIT : objet leger, pousse par le TCHAK.
 *   MADAME ARMOIRE (boss) : avale tout objet qu'on lui LANCE (rapide) et le
 *     recrache par-dessus elle, vers l'arriere. Un TCHAK proche la retourne.
 * ===================================================================== */
#include "nes.h"
#include "game.h"

#define ST_IDLE   0
#define ST_GO     1
#define ST_REST   2

static u8 px, py, bx, by;
static u8 w_best[MAX_OBJ], w_stall[MAX_OBJ];     /* (RAM de banque) */

static void start_go(u8 i)
{
    o_state[i] = ST_GO;
    w_best[i] = PIX(o_x[i]);
    w_stall[i] = 0;
}

static void stop_if_blocked(u8 i)
{
    /* plus d'avancee depuis 12 trames ou mur : arret */
    if (o_dir[i] < 0 ? (px < w_best[i]) : (px > w_best[i])) { w_best[i] = px; w_stall[i] = 0; }
    else ++w_stall[i];
    if (o_wallhit[i] || w_stall[i] > 12) {
        o_state[i] = ST_REST;
        o_timer[i] = 90;
        o_vx[i] = 0;
        if (o_type[i] == OT_DRESSER) o_aux[i] = 0;
    }
}

static u8 touches_bidule(void)
{
    if ((u8)(px + 16) <= (u8)(bx + 3) || px >= (u8)(bx + 13)) return 0;
    if ((u8)(py + 16) <= (u8)(by + 2) || py >= (u8)(by + 24)) return 0;
    return 1;
}

static void go(u8 i, u8 speed)
{
    o_vx[i] = (o_dir[i] < 0) ? -(s8)speed : (s8)speed;
}

/* ------------------------------------------------------------ Madame Armoire */
static const u8 wr_speed[4] = { 8, 12, 16, 16 };

static void wardrobe_update(u8 i)
{
    u8 j, ox, oy;
    if (lock_count >= 3) {              /* vaincue : elle reste bouche bee */
        o_vx[i] = 0;
        return;
    }
    if (o_aux[i]) {
        /* elle digere : bouche ouverte, puis recrache par-dessus, vers l'arriere */
        o_vx[i] = 0;
        if (--o_timer[i] == 0) {
            j = o_aux[i] - 1;
            o_aux[i] = 0;
            o_flags[j] &= ~OF_HIDDEN;
            o_x[j] = o_x[i];
            o_y[j] = o_y[i] - SUB(28);
            o_vx[j] = (o_dir[i] < 0) ? 30 : -30;
            o_vy[j] = -100;
            o_flags[j] &= ~(OF_GROUND | OF_REST);
            sfx_play(SFX_BOING);
        }
        return;
    }
    /* demi-tour contre un mur, ou si elle est bloquee (elle pousse un objet coince) */
    if (px == w_best[i]) ++w_stall[i]; else w_stall[i] = 0;
    w_best[i] = px;
    if (o_wallhit[i] || w_stall[i] > 8) { o_dir[i] = -o_dir[i]; w_stall[i] = 0; }
    go(i, wr_speed[lock_count & 3]);
    /* avale ce qui touche sa facade */
    for (j = 0; j < MAX_OBJ; ++j) {
        if (j == i || o_type[j] == OT_NONE || o_type[j] == OT_SOCK) continue;
        if (o_flags[j] & (OF_HIDDEN | OF_DEAD)) continue;
        /* elle n'avale que ce qu'on lui LANCE (objet rapide) */
        if (o_vx[j] < 16 && o_vx[j] > -16) continue;
        ox = PIX(o_x[j]); oy = PIX(o_y[j]);
        if ((u8)(ox + 18) <= px || ox >= (u8)(px + 18)) continue;
        if ((u8)(oy + 16) <= (u8)(py - 16) || oy >= (u8)(py + 16)) continue;
        o_flags[j] |= OF_HIDDEN;
        o_aux[i] = j + 1;
        o_timer[i] = 40;
        sfx_play(SFX_SQUASH);
        return;
    }
    if (touches_bidule() && b_emote != EM_SQUASH) {
        bidule_emote(EM_SQUASH, 45);
        sfx_play(SFX_SQUASH);
    }
}

/* -------------------------------------------------------------- mise a jour */
void __fastcall__ w2_update(u8 i)
{
    u8 j, best, d, ox;
    px = PIX(o_x[i]); py = PIX(o_y[i]);
    bx = PIX(b_x); by = PIX(b_y);
    switch (o_type[i]) {
    case OT_ARMCHAIR:
        if (o_state[i] == ST_GO) {
            go(i, 32);
            stop_if_blocked(i);
            if (touches_bidule() && b_emote != EM_SQUASH) {
                bidule_emote(EM_SQUASH, 45);
                sfx_play(SFX_SQUASH);
            }
        } else if (o_state[i] == ST_REST && --o_timer[i] == 0) o_state[i] = ST_IDLE;
        break;
    case OT_LAMP:
        if (o_state[i] == ST_REST) o_state[i] = ST_IDLE;
        if (o_state[i] == ST_GO) {
            /* o_aux = abscisse visee */
            if ((u8)(px - o_aux[i] + 1) <= 2) {
                o_x[i] = SUB(o_aux[i]);
                o_vx[i] = 0;
                o_state[i] = ST_IDLE;
            } else {
                o_dir[i] = (o_aux[i] > px) ? 1 : -1;
                go(i, 12);
                stop_if_blocked(i);
            }
        }
        break;
    case OT_DRESSER:
        if (o_state[i] == ST_GO) {
            go(i, 16);
            stop_if_blocked(i);
        } else if (o_timer[i] && --o_timer[i] == 0) {
            if (o_state[i] == ST_REST) o_state[i] = ST_IDLE;
            else if (o_aux[i] && --o_aux[i]) o_timer[i] = 90;   /* l'agacement retombe */
        }
        break;
    case OT_CHEST:
        if (o_state[i] == ST_REST) {          /* il mache */
            o_vx[i] = 0;
            if (--o_timer[i] == 0) o_state[i] = ST_IDLE;
            break;
        }
        best = 0xFF; d = 255;
        for (j = 0; j < MAX_OBJ; ++j) {
            if (o_type[j] != OT_COOKIE || (o_flags[j] & (OF_HIDDEN | OF_DEAD))) continue;
            if ((u8)(PIX(o_y[j]) - py + 24) > 48) continue;      /* pas au meme etage */
            ox = PIX(o_x[j]);
            ox = (ox > px) ? ox - px : px - ox;
            if (ox < d) { d = ox; best = j; }
        }
        if (best == 0xFF) { o_vx[i] = 0; o_state[i] = ST_IDLE; break; }
        if (d <= 2) {
            o_flags[best] |= OF_DEAD;         /* miam ! (un autre biscuit reapparaitra) */
            o_timer[best] = 120;
            o_state[i] = ST_REST;
            o_timer[i] = 60;
            o_vx[i] = 0;
            sfx_play(SFX_SOCK);
        } else {
            o_dir[i] = (PIX(o_x[best]) > px) ? 1 : -1;
            o_state[i] = ST_GO;
            go(i, 10);
        }
        break;
    case OT_WARDROBE:
        wardrobe_update(i);
        break;
    }
}

/* ---------------------------------------------------------- reaction au TCHAK */
void __fastcall__ w2_tchak(u8 i)
{
    switch (o_type[i]) {
    case OT_ARMCHAIR:
        if (tk_adx < 112 && tk_ady < 96 && o_state[i] != ST_GO) {
            o_dir[i] = -tk_dir;               /* VERS BIDULE */
            start_go(i);
            o_vy[i] = -24;
            sfx_play(SFX_PANIC);
        }
        break;
    case OT_LAMP:
        if (tk_adx < 128 && tk_ady < 96) {
            o_aux[i] = PIX(b_x);             /* va voir la ou ca a fait TCHAK */
            o_dir[i] = (o_aux[i] > PIX(o_x[i])) ? 1 : -1;
            start_go(i);
            sfx_play(SFX_SURPRISE);
        }
        break;
    case OT_DRESSER:
        if (tk_adx < 64 && tk_ady < 48 && o_state[i] != ST_GO) {
            ++o_aux[i];
            o_timer[i] = 90;
            if (o_aux[i] >= 3) {
                o_aux[i] = 0;
                o_dir[i] = tk_dir;            /* a l'oppose de BIDULE */
                start_go(i);
                sfx_play(SFX_PANIC);
            } else sfx_play(SFX_BUMP);
        }
        break;
    case OT_COOKIE:
        tchak_impulse(i);
        break;
    case OT_WARDROBE:
        if (tk_adx < 64 && tk_ady < 48 && !o_aux[i]) o_dir[i] = -o_dir[i];
        break;
    }
}

/* -------------------------------------------------------------------- dessin */
static void meta(u8 f, u8 x, u8 y, u8 a)
{
    m_ptr = obj_frames[f];
    m_x = x; m_y = y; m_attr = a; m_w = 2; m_h = 2;
    oam_meta();
}

void __fastcall__ w2_draw(u8 i)
{
    u8 x, y, a, t;
    t = o_type[i];
    x = PIX(o_x[i]);
    y = PIX(o_y[i]);
    a = (o_dir[i] < 0) ? 0x40 : 0;
    switch (t) {
    case OT_ARMCHAIR:
        a |= 1;
        meta(o_state[i] == ST_GO ? OF_ARMCHAIR_MAD : OF_ARMCHAIR, x, y, a);
        if (o_state[i] == ST_IDLE && (room_timer & 32)) oam_spr(x + 12, y - 6, FX_ZZZ, 3);
        if (o_state[i] == ST_GO) oam_spr(x + 4, y - 9, FX_BANG, 3);
        break;
    case OT_LAMP:
        meta((o_state[i] == ST_GO && (room_timer & 8)) ? OF_LAMP_WALK : OF_LAMP, x, y, a | 3);
        if (o_state[i] == ST_GO) oam_spr(x + 4, y - 9, FX_BANG, 3);
        break;
    case OT_DRESSER:
        meta(o_state[i] == ST_GO ? OF_DRESSER_MAD : OF_DRESSER, x, y, 2);
        if (o_state[i] != ST_GO) {
            if (o_aux[i] == 0) { if (room_timer & 32) oam_spr(x + 12, y - 6, FX_ZZZ, 3); }
            else {
                oam_spr(x + 2, y - 9, FX_STAR, 3);
                if (o_aux[i] > 1) oam_spr(x + 9, y - 9, FX_STAR, 3);
            }
        }
        break;
    case OT_CHEST:
        meta((o_state[i] == ST_REST && (room_timer & 4)) ? OF_CHEST_OPEN : OF_CHEST, x, y, a | 2);
        break;
    case OT_COOKIE:
        meta(OF_COOKIE, x, y, 2);
        break;
    case OT_WARDROBE:
        meta(OF_WARDROBE_TOP, x, (u8)(y - 16), a | 1);
        meta((o_aux[i] || lock_count >= 3) ? OF_WARDROBE_OPEN : OF_WARDROBE_BOT, x, y, a | 1);
        if (lock_count >= 3 && (room_timer & 16)) oam_spr(x + 4, y - 24, FX_STAR, 3);
        break;
    }
}
