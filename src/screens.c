/* =====================================================================
 *  TCHAK ! - ecrans communs (moteur partage par toutes les banques) :
 *  presentation du monde et de la salle, outils d'affichage des menus.
 *  (Les menus eux-memes sont dans la banque des menus : menus.c)
 * ===================================================================== */
#include "nes.h"
#include "game.h"

const u8 pal_dark[32] = {
    0x0F, 0x01, 0x21, 0x30,   0x0F, 0x05, 0x15, 0x27,
    0x0F, 0x0A, 0x1A, 0x2A,   0x0F, 0x01, 0x11, 0x21,
    0x0F, 0x0F, 0x24, 0x30,   0x0F, 0x06, 0x16, 0x30,
    0x0F, 0x07, 0x17, 0x28,   0x0F, 0x0F, 0x2A, 0x30
};

/* noms des mondes sur deux lignes (32 colonnes maximum) */
static const char *const world_names[10] = {
    "LA FABRIQUE DES CHOSES MOLLES", "",
    "LA MAISON QUI N'EN FAIT", "QU'] SA T&TE",
    "L'USINE DES CATASTROPHES", "",
    "LE MONDE QUI MARCHE DE TRAVERS", "",
    "LE GRAND BAZAR", ""
};


void frame(void)
{
    oam_finish();
    ppu_wait_nmi();
    vbuf_len = 0;
    pad_poll();
}

u8 strlen8(const char *s)
{
    u8 n = 0;
    while (s[n]) ++n;
    return n;
}

void put_center(u8 y, const char *s)
{
    put_text((u8)(16 - (strlen8(s) >> 1)), y, s);
}

void draw_bidule(u8 x, u8 y, u8 f, u8 flip)
{
    const u8 *t = bidule_frames[f];
    u8 k;
    for (k = 0; k < 3; ++k) {
        if (flip) { oam_spr(x, y, t[1], 0x40); oam_spr(x + 8, y, t[0], 0x40); }
        else      { oam_spr(x, y, t[0], 0);    oam_spr(x + 8, y, t[1], 0); }
        t += 2;
        y += 8;
    }
}

/* ======================================= PRESENTATION DU MONDE / SALLE */
void intro_screen(void)
{
    static u8 pw[5];
    u8 t;
    const Room *r = &rooms[room_idx & 7];

    ppu_off();
    oam_clear(); oam_finish();
    screen_clear();
    attr_fill(0);
    vram_adr(NT_ADR(12, 5));
    put_text(12, 5, "MONDE");
    vram_adr(NT_ADR(18, 5));
    vram_put('0' + r->world);
    put_center(8, world_names[(r->world - 1) * 2]);
    put_center(9, world_names[(r->world - 1) * 2 + 1]);
    put_text(11, 13, "SALLE");
    vram_adr(NT_ADR(17, 13));
    vram_put('0' + r->world);
    vram_put('-');
    vram_put('0' + r->num);
    put_center(15, r->name);
    put_text(10, 21, "_");
    put_num(12, 21, lives);
    put_text(18, 21, "@");
    put_num(20, 21, socks);
    password_make(pw);
    put_text(10, 25, "CODE :");
    put_text(17, 25, (const char *)pw);
    pal_set(pal_dark);
    ppu_on();
    music_stop();
    sfx_play(SFX_SELECT);

    for (t = 0; t < 150; ++t) {
        oam_clear();
        draw_bidule(120, 136, (t & 32) ? BF_LOOK : BF_IDLE, (t >> 6) & 1);
        frame();
        if (t > 20 && (pad_new & (PAD_START | PAD_A | PAD_B))) break;
    }
    pal_fade_black();
    game_state = GS_PLAY;
}

