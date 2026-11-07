/* =====================================================================
 *  TCHAK ! - MONDE 4 : LE MONDE QUI MARCHE DE TRAVERS
 *  (banque MMC1 du monde 4, appelee par la table de sauts bankhdr_w4.s)
 *
 *  Regle du monde : la gravite des OBJETS peut s'inverser (BIDULE, lui,
 *  garde les pieds sur terre). Un objet qui "tombe vers le haut" se pose
 *  au plafond, et peut y enfoncer des plaques ou y toucher des cibles.
 *
 *   ACTION WORLD n : n = numero d'un AIMANT -> on l'allume / l'eteint ;
 *                    n >= 8                -> la gravite s'inverse.
 *   BALLON : tombe toujours a l'envers du reste du monde.
 *   AIMANT : fixe ; allume, il attire horizontalement les caisses de toute
 *     la salle (meme collees au plafond) vers sa colonne.
 *   LE ROI A L'ENVERS (boss) : marche vers BIDULE sur le sol ou sur le
 *     plafond selon la gravite, sans jamais marcher dans un trou ; au bout
 *     d'un moment, c'est lui qui retourne la gravite. Il ne tombe dans une
 *     cuve (du sol ou du plafond) que si la gravite s'inverse au bon moment.
 * ===================================================================== */
#include "nes.h"
#include "game.h"

static u8 px, py;
static u8 w_best[MAX_OBJ], w_stall[MAX_OBJ];
static u8 env_clock;

static const u8 env_speed[3]  = { 12, 16, 20 };
static const u8 env_period[3] = { 255, 220, 160 };   /* il retourne la gravite */

static void flip_gravity(void)
{
    grav_up ^= 1;
    shake = 16;
    sfx_play(SFX_SURPRISE);
    hud_message(grav_up ? "TOUT TOMBE VERS LE HAUT !" : "RETOUR ] LA NORMALE ?");
}

void __fastcall__ w4_action(u8 n)
{
    if (n >= 8) { flip_gravity(); return; }
    if (o_type[n] == OT_MAGNET) {
        o_state[n] ^= 1;
        sfx_play(SFX_CLICK);
    }
}

static void stay_home(u8 i)
{
    o_x[i] = SUB(o_home_x[i]);
    o_y[i] = SUB(o_home_y[i]);
    o_vx[i] = o_vy[i] = 0;
}

void __fastcall__ w4_update(u8 i)
{
    u8 j, ph, fx, fy, bx;
    px = PIX(o_x[i]); py = PIX(o_y[i]);
    switch (o_type[i]) {
    case OT_BALLOON:
        o_flags[i] |= OF_FLOAT;
        break;
    case OT_MAGNET:
        stay_home(i);
        if (!o_state[i]) break;
        for (j = 0; j < MAX_OBJ; ++j) {
            if (o_type[j] != OT_CRATE || (o_flags[j] & (OF_DEAD | OF_HIDDEN))) continue;
            fx = PIX(o_x[j]);
            if (fx + 16 < px) { if (o_vx[j] < 40) o_vx[j] += 6; }
            else if (fx > px + 16) { if (o_vx[j] > -40) o_vx[j] -= 6; }
            o_flags[j] &= ~OF_REST;
        }
        break;
    case OT_ENVERS:
        if (o_aux[i]) { o_vx[i] = 0; break; }
        ph = o_state[i] > 2 ? 2 : o_state[i];
        if (ph && ++env_clock >= env_period[ph]) { env_clock = 0; flip_gravity(); }
        if (!(o_flags[i] & OF_GROUND)) { o_vx[i] = 0; break; }   /* en chute : il ne dirige rien */
        bx = PIX(b_x);
        o_dir[i] = (bx > px) ? 1 : -1;            /* il va vers BIDULE... */
        fx = (o_dir[i] < 0) ? (u8)(px - 1) : (u8)(px + 16);
        fy = grav_up ? (u8)(py - 1) : (u8)(py + 17);
        if (!IS_SOLID(coll_at(fx, fy))) { o_vx[i] = 0; break; }   /* ...sans tomber */
        if (px == w_best[i]) ++w_stall[i]; else w_stall[i] = 0;
        w_best[i] = px;
        o_vx[i] = (o_dir[i] < 0) ? -(s8)env_speed[ph] : (s8)env_speed[ph];
        if ((u8)(px + 16) > (u8)(bx + 3) && px < (u8)(bx + 13) &&
            (u8)(py + 16) > (u8)(PIX(b_y) + 2) && py < (u8)(PIX(b_y) + 24) && b_emote != EM_SQUASH) {
            bidule_emote(EM_SQUASH, 45);
            sfx_play(SFX_SQUASH);
        }
        break;
    }
}

void __fastcall__ w4_tchak(u8 i)
{
    if (o_type[i] == OT_BALLOON) tchak_impulse(i);
}

static void meta(u8 f, u8 x, u8 y, u8 a)
{
    m_ptr = obj_frames[f];
    m_w = 2;
    if (a & 0x80) {                      /* miroir vertical : lignes echangees */
        m_x = x; m_y = y + 8; m_attr = a; m_h = 1;
        oam_meta();
        m_ptr = obj_frames[f] + 2;
        m_x = x; m_y = y; m_attr = a; m_h = 1;
        oam_meta();
        return;
    }
    m_x = x; m_y = y; m_attr = a; m_h = 2;
    oam_meta();
}

void __fastcall__ w4_draw(u8 i)
{
    u8 x, y, a;
    x = PIX(o_x[i]);
    y = PIX(o_y[i]);
    a = (o_dir[i] < 0) ? 0x40 : 0;
    switch (o_type[i]) {
    case OT_BALLOON:
        /* au plafond, la ficelle pend vers le bas ; au sol, vers le haut */
        meta(OF_BALLOON, x, y, ((o_flags[i] & OF_FLOAT) && !grav_up) ? 1 : 0x81);
        break;
    case OT_MAGNET:
        meta(o_state[i] && (room_timer & 4) ? OF_MAGNET_ON : OF_MAGNET, x, y, 1);
        break;
    case OT_ENVERS:
        /* au plafond, il est dessine tete en bas (miroir vertical) */
        meta(o_state[i] >= 2 ? OF_ENVERS_MAD : OF_ENVERS, x, y, a | 2 | (grav_up ? 0x80 : 0));
        break;
    }
}
