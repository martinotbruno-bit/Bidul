/* =====================================================================
 *  TCHAK ! - MONDE 3 : L'USINE DES CATASTROPHES
 *  (banque MMC1 du monde 3, appelee par la table de sauts bankhdr_w3.s)
 *
 *  Regle du monde : les reactions en chaine a plusieurs etages ; il faut
 *  comprendre L'ORDRE dans lequel les choses se produisent.
 *
 *   PISTON : fixe ; une action WORLD <n> (cible, plaque, levier...) le fait
 *     frapper : tout objet juste devant lui est projete.
 *   LE COPIEUR : fait exactement les memes pas que BIDULE. Un TCHAK
 *     (96 px) le fait passer en mode MIROIR (il fait l'inverse), et retour.
 *   LE MACHINAL : patrouille d'un obstacle a l'autre en appuyant sur tout
 *     ce qu'il croise ; un TCHAK (96 px) le fige 2,5 s, puis il repart
 *     dans l'autre sens.
 *   LE GRAND DERANGE (boss) : patrouille sans jamais tomber dans un trou
 *     et inverse les convoyeurs a intervalles reguliers. Un TCHAK proche
 *     l'etourdit : il ne marche plus... et le convoyeur l'emporte.
 * ===================================================================== */
#include "nes.h"
#include "game.h"

static u8 px, py;
static u8 w_best[MAX_OBJ], w_stall[MAX_OBJ];     /* (RAM de banque) */
static u8 der_clock;
static u8 prev_bx;              /* position de BIDULE a la trame precedente */

static const u8 der_speed[3]  = { 10, 14, 18 };
static const u8 der_period[3] = { 240, 180, 140 };
static const u8 der_stun[3]   = { 90, 70, 55 };

static void go(u8 i, u8 speed)
{
    o_vx[i] = (o_dir[i] < 0) ? -(s8)speed : (s8)speed;
}

/* bloque (mur, ou n'avance plus depuis 10 trames) ? */
static u8 blocked(u8 i)
{
    if (px == w_best[i]) ++w_stall[i]; else w_stall[i] = 0;
    w_best[i] = px;
    if (o_wallhit[i] || w_stall[i] > 10) { w_stall[i] = 0; return 1; }
    return 0;
}

static u8 touches_bidule(void)
{
    u8 bx = PIX(b_x), by = PIX(b_y);
    if ((u8)(px + 16) <= (u8)(bx + 3) || px >= (u8)(bx + 13)) return 0;
    if ((u8)(py + 16) <= (u8)(by + 2) || py >= (u8)(by + 24)) return 0;
    return 1;
}

/* ---------------------------------------------------------- action WORLD */
void __fastcall__ w3_action(u8 n)
{
    u8 j, ox, oy, lo, hi;
    s8 d;
    if (n >= MAX_OBJ || o_type[n] != OT_PISTON) return;
    o_timer[n] = 16;                        /* bras sorti */
    sfx_play(SFX_BUMP);
    px = PIX(o_x[n]); py = PIX(o_y[n]);
    if (o_flags[n] & OF_LEFT) { d = -1; lo = (u8)(px - 20); hi = (u8)(px + 4); }
    else                      { d = 1;  lo = (u8)(px + 12); hi = (u8)(px + 36); }
    for (j = 0; j < MAX_OBJ; ++j) {
        if (j == n || o_type[j] == OT_NONE || o_type[j] == OT_PISTON || o_type[j] == OT_SOCK) continue;
        if (o_flags[j] & (OF_HIDDEN | OF_DEAD)) continue;
        ox = PIX(o_x[j]); oy = PIX(o_y[j]);
        if (ox < lo || ox > hi) continue;
        if ((u8)(oy - py + 14) > 28) continue;
        o_vx[j] = (d < 0) ? -80 : 80;
        o_vy[j] = -24;
        o_flags[j] &= ~(OF_GROUND | OF_REST);
        o_anim[j] = 8;
        if (o_type[j] == OT_DERANGE) o_timer[j] = der_stun[o_state[j] > 2 ? 2 : o_state[j]];
    }
}

/* -------------------------------------------------------------- mise a jour */
void __fastcall__ w3_update(u8 i)
{
    u8 ph, fx;
    px = PIX(o_x[i]); py = PIX(o_y[i]);
    switch (o_type[i]) {
    case OT_PISTON:
        /* fixe : il reste a sa place (les objets butent contre lui) */
        if (o_timer[i]) --o_timer[i];
        o_dir[i] = (o_flags[i] & OF_LEFT) ? -1 : 1;
        o_x[i] = SUB(o_home_x[i]);
        o_y[i] = SUB(o_home_y[i]);
        o_vx[i] = o_vy[i] = 0;
        break;
    case OT_COPY:
        /* memes pas que BIDULE : son deplacement REEL depuis la trame
           precedente (pousser contre un mur ne fait pas avancer le Copieur) */
        if (b_state == BS_NORMAL && b_ground) {
            fx = PIX(b_x) - prev_bx;
            o_vx[i] = (s8)fx << 4;
            if (o_state[i]) o_vx[i] = -o_vx[i];
            if (o_vx[i]) o_dir[i] = (o_vx[i] < 0) ? -1 : 1;
        } else o_vx[i] = 0;
        break;
    case OT_ROBOT:
        if (o_state[i]) {                  /* fige */
            o_vx[i] = 0;
            if (--o_timer[i] == 0) { o_state[i] = 0; o_dir[i] = -o_dir[i]; }
            break;
        }
        fx = (o_dir[i] < 0) ? (u8)(px - 1) : (u8)(px + 16);
        if (blocked(i) || !IS_SOLID(coll_at(fx, (u8)(py + 17)))) o_dir[i] = -o_dir[i];
        go(i, 12);
        break;
    case OT_DERANGE:
        if (o_aux[i]) { o_vx[i] = 0; break; }   /* vaincu (gere par le moteur) */
        ph = o_state[i] > 2 ? 2 : o_state[i];
        /* il deregle les machines */
        if (++der_clock >= der_period[ph]) {
            der_clock = 0;
            conv_dir = conv_dir ? -conv_dir : 1;
            sfx_play(SFX_CLICK);
            o_anim[i] = 12;
        }
        if (o_timer[i]) {                  /* etourdi : le convoyeur l'emporte */
            --o_timer[i];
            o_vx[i] = 0;
            break;
        }
        /* demi-tour au bord d'un trou ou contre un mur : il ne tombe jamais seul */
        fx = (o_dir[i] < 0) ? (u8)(px - 1) : (u8)(px + 16);
        if (blocked(i) || !IS_SOLID(coll_at(fx, (u8)(py + 17)))) o_dir[i] = -o_dir[i];
        go(i, der_speed[ph]);
        if (touches_bidule() && b_emote != EM_SQUASH) {
            bidule_emote(EM_SQUASH, 45);
            sfx_play(SFX_SQUASH);
        }
        break;
    }
}

/* ---------------------------------------------------------- reaction au TCHAK */
void __fastcall__ w3_tchak(u8 i)
{
    switch (o_type[i]) {
    case OT_COPY:
        if (tk_adx < 96 && tk_ady < 96) {
            o_state[i] ^= 1;
            sfx_play(SFX_SURPRISE);
        }
        break;
    case OT_ROBOT:
        if (tk_adx < 96 && tk_ady < 48 && !o_state[i]) {
            o_state[i] = 1;
            o_timer[i] = 150;
            o_vx[i] = 0;
            sfx_play(SFX_CLICK);
        }
        break;
    case OT_DERANGE:
        if (tk_adx < 64 && tk_ady < 48 && !o_aux[i]) {
            o_timer[i] = der_stun[o_state[i] > 2 ? 2 : o_state[i]];
            sfx_play(SFX_SQUASH);
        }
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

void __fastcall__ w3_draw(u8 i)
{
    u8 x, y, a;
    prev_bx = PIX(b_x);
    x = PIX(o_x[i]);
    y = PIX(o_y[i]);
    a = (o_dir[i] < 0) ? 0x40 : 0;
    switch (o_type[i]) {
    case OT_PISTON:
        meta(o_timer[i] ? OF_PISTON_OUT : OF_PISTON, x, y, a | 2);
        break;
    case OT_COPY:
        meta(o_state[i] ? OF_COPY_MIR : OF_COPY, x, y, a | 3);
        if (o_state[i] && (room_timer & 16)) oam_spr(x + 4, y - 9, FX_STAR, 3);
        break;
    case OT_ROBOT:
        meta(o_state[i] ? OF_ROBOT_PAUSE : OF_ROBOT, x, y, a | 3);
        if (o_state[i] && (room_timer & 8)) oam_spr(x + 4, y - 9, FX_BANG, 3);
        break;
    case OT_DERANGE:
        meta((o_timer[i] || o_aux[i]) ? OF_DERANGE_DIZZY : OF_DERANGE, x, y, a | 1);
        if (o_anim[i] && (room_timer & 2)) oam_spr(x + 4, y - 9, FX_BANG, 3);
        if (o_timer[i] && (room_timer & 8)) oam_spr(x + 2, y - 8, FX_STAR, 3);
        break;
    }
}
