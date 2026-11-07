/* =====================================================================
 *  TCHAK ! - MONDE 5 : LE GRAND BAZAR
 *  Deux banques MMC1 (le code de tous les mondes ne tient pas dans une) :
 *   - banque 5a (salles 5-1 a 5-4) : meubles du monde 2 + gravite du monde 4
 *   - banque 5b (salles 5-5 a 5-8) : machines du monde 3 + gravite du monde 4
 *     + LE MAITRE DU CHAOS, boss final.
 *  Ce fichier ne fait qu'aiguiller vers le code des autres mondes, compile
 *  dans la meme banque (world2.c / world3.c / world4.c).
 * ===================================================================== */
#include "nes.h"
#include "game.h"

void __fastcall__ w2_update(u8 i);
void __fastcall__ w2_draw(u8 i);
void __fastcall__ w2_tchak(u8 i);
void __fastcall__ w3_update(u8 i);
void __fastcall__ w3_draw(u8 i);
void __fastcall__ w3_tchak(u8 i);
void __fastcall__ w3_action(u8 n);
void __fastcall__ w4_update(u8 i);
void __fastcall__ w4_draw(u8 i);
void __fastcall__ w4_tchak(u8 i);
void __fastcall__ w4_action(u8 n);

#define IS_W4(t) ((t) >= OT_BALLOON && (t) <= OT_ENVERS)

#ifdef BANK_5A
/* ------------------------------------------------------------ banque 5a */
void __fastcall__ w5a_update(u8 i) { if (IS_W4(o_type[i])) w4_update(i); else w2_update(i); }
void __fastcall__ w5a_draw(u8 i)   { if (IS_W4(o_type[i])) w4_draw(i);   else w2_draw(i); }
void __fastcall__ w5a_tchak(u8 i)  { if (IS_W4(o_type[i])) w4_tchak(i);  else w2_tchak(i); }
void __fastcall__ w5a_action(u8 n) { w4_action(n); }

#else
/* ------------------------------------------------------------ banque 5b */
/* LE MAITRE DU CHAOS : il flotte, intouchable, et lance un sort a
   intervalles reguliers (annonce par un "!" une seconde avant) :
     0 : la gravite des objets s'inverse
     1 : les caisses deviennent legeres comme des ballons (ou retombent)
     2 : les convoyeurs s'inversent (ou demarrent)
   Plus les serrures s'ouvrent, plus il s'enerve (sorts plus frequents).
   A la troisieme serrure, il est vaincu : tout redevient normal. */
static u16 chaos_clock;
static const u16 chaos_period[3] = { 330, 240, 170 };

static void cast_spell(u8 i)
{
    u8 j;
    switch (o_state[i]) {
    case 0:
        w4_action(9);
        break;
    case 1:
        for (j = 0; j < MAX_OBJ; ++j)
            if (o_type[j] == OT_CRATE) { o_flags[j] ^= OF_FLOAT; o_flags[j] &= ~OF_REST; }
        hud_message("LES CAISSES PERDENT LA T&TE !");
        sfx_play(SFX_SURPRISE);
        break;
    default:
        conv_dir = conv_dir ? -conv_dir : 1;
        hud_message("LES TAPIS S'AFFOLENT !");
        sfx_play(SFX_CLICK);
        break;
    }
    o_state[i] = (o_state[i] + 1) % 3;
}

static void master_update(u8 i)
{
    u8 j;
    o_x[i] = SUB(o_home_x[i]);
    o_y[i] = SUB(o_home_y[i]);
    o_vx[i] = o_vy[i] = 0;
    if (lock_count >= 3) {
        if (o_aux[i] != 2) {                     /* vaincu : tout redevient normal */
            o_aux[i] = 2;
            grav_up = 0;
            for (j = 0; j < MAX_OBJ; ++j) o_flags[j] &= ~(OF_REST | (o_type[j] == OT_CRATE ? OF_FLOAT : 0));
            hud_message("LE MAITRE DU CHAOS EST VAINCU !");
            shake = 30;
        }
        return;
    }
    ++chaos_clock;
    if (chaos_clock + 60 >= chaos_period[lock_count]) o_anim[i] = 2;   /* il prepare son sort */
    if (chaos_clock >= chaos_period[lock_count]) {
        chaos_clock = 0;
        cast_spell(i);
    }
}

void __fastcall__ w5b_update(u8 i)
{
    u8 t = o_type[i];
    if (t == OT_MASTER) master_update(i);
    else if (IS_W4(t)) w4_update(i);
    else w3_update(i);
}

void __fastcall__ w5b_draw(u8 i)
{
    u8 t = o_type[i], x, y;
    if (t == OT_MASTER) {
        x = PIX(o_x[i]); y = PIX(o_y[i]) + ((room_timer >> 4) & 1);
        m_ptr = obj_frames[(o_anim[i] || o_aux[i] == 2) && (room_timer & 4) ? OF_MASTER_CAST : OF_MASTER];
        m_x = x; m_y = y; m_attr = 3; m_w = 2; m_h = 2;
        oam_meta();
        if (o_anim[i] && (room_timer & 8)) oam_spr(x + 4, y - 9, FX_BANG, 3);
        if (o_aux[i] == 2) oam_spr(x + (room_timer & 15), y - 6, FX_STAR, 3);
        return;
    }
    if (IS_W4(t)) w4_draw(i); else w3_draw(i);
}

void __fastcall__ w5b_tchak(u8 i)
{
    u8 t = o_type[i];
    if (t == OT_MASTER) return;
    if (IS_W4(t)) w4_tchak(i); else w3_tchak(i);
}

void __fastcall__ w5b_action(u8 n)
{
    if (n < MAX_OBJ && o_type[n] == OT_PISTON) w3_action(n); else w4_action(n);
}
#endif
