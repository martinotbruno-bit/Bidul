/* =====================================================================
 *  TCHAK ! - module 15 : gestion de la progression
 *
 *  Mot de passe de 4 lettres (alphabet de 16 lettres sans voyelles,
 *  pour eviter les mots genants) = 16 bits :
 *     bits 0-5   salle (0-39)
 *     bits 6-11  nombre de chaussettes (0-40)
 *     bits 12-15 somme de controle
 *  le tout brouille par un XOR. Pas de pile de sauvegarde : NROM.
 * ===================================================================== */
#include "nes.h"
#include "game.h"

u8 game_state;
u8 room_idx;
u8 lives;
u8 socks;
u8 sock_mask[5];
u8 room_result;
u8 room_timer;
u8 rng_unused;

static const char alpha[16] = "BCDFGHJKLMNPRSTV";
#define PW_XOR 0x5A3C

void progress_new_game(void)
{
    u8 i;
    room_idx = 0;
    lives = 5;
    socks = 0;
    for (i = 0; i < 5; ++i) sock_mask[i] = 0;
}

static u8 checksum(u8 r, u8 s)
{
    return (u8)(r + s * 3 + 5) & 15;
}

void password_make(u8 *out)
{
    u16 v = room_idx | ((u16)socks << 6) | ((u16)checksum(room_idx, socks) << 12);
    v ^= PW_XOR;
    out[0] = alpha[(v >> 12) & 15];
    out[1] = alpha[(v >> 8) & 15];
    out[2] = alpha[(v >> 4) & 15];
    out[3] = alpha[v & 15];
    out[4] = 0;
}

/* in : 4 index (0-15) dans l'alphabet. Renvoie 1 si valide. */
u8 password_check(const u8 *in)
{
    u16 v;
    u8 r, s, i;
    v = ((u16)in[0] << 12) | ((u16)in[1] << 8) | ((u16)in[2] << 4) | in[3];
    v ^= PW_XOR;
    r = v & 63;
    s = (v >> 6) & 63;
    if ((u8)(v >> 12) != checksum(r, s) || r >= TOTAL_ROOMS || s > 40) return 0;
    progress_new_game();
    room_idx = r;
    socks = s;
    /* les chaussettes des salles precedentes sont considerees comme comptees */
    for (i = 0; i < r; ++i) sock_mask[i >> 3] |= 1 << (i & 7);
    return 1;
}
