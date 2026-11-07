/* =====================================================================
 *  TCHAK ! - module 16 : gestion des menus (BANQUE DES MENUS, MMC1)
 *  Ecran titre (logo anime), menu, mot de passe, partie terminee, fin de
 *  monde. Compile dans les segments BANKCODE/BANKDATA : ce code n'existe
 *  que dans la banque 0 et n'est appele que par la table de sauts.
 * ===================================================================== */
#include "nes.h"
#include "game.h"

static const u8 pal_title[32] = {
    0x11, 0x0F, 0x28, 0x30,   0x11, 0x0F, 0x16, 0x27,
    0x11, 0x0F, 0x17, 0x27,   0x11, 0x0F, 0x28, 0x30,
    0x11, 0x0F, 0x24, 0x30,   0x11, 0x06, 0x16, 0x30,
    0x11, 0x07, 0x17, 0x28,   0x11, 0x0F, 0x28, 0x30
};
static u8 i_, k_;

/* =============================================================== TITRE */
#define LOGO_Y   40
static s16 ly;
static s8  lvy;
static u8  lbounce;

static void logo_letter_bg(u8 k, u8 show)
{
    static u8 row[3];
    u8 r, c;
    for (r = 0; r < 3; ++r) {
        for (c = 0; c < 3; ++c) row[c] = show ? logo_tiles[k][r * 3 + c] : 0;
        vbuf_tiles(NT_ADR(4 + k * 4, 5 + r), row, 3);
    }
}

static void logo_letter_spr(u8 k, u8 y)
{
    u8 r, c, x = (4 + k * 4) * 8;
    for (r = 0; r < 3; ++r)
        for (c = 0; c < 3; ++c)
            oam_spr(x + c * 8, y + r * 8, slogo_tiles[k][r * 3 + c], 3);
}

static void title_draw_static(void)
{
    ppu_off();
    oam_clear();
    oam_finish();
    screen_clear();
    put_center(10, "LA GRANDE CATASTROPHE");
    put_center(26, "VERSION 0.3 - MONDE 1");
    /* sol de girders pour BIDULE */
    vram_adr(NT_ADR(0, 18));
    for (i_ = 0; i_ < 16; ++i_) { vram_put(mt_tiles[MT_GIRDER][0]); vram_put(mt_tiles[MT_GIRDER][1]); }
    for (i_ = 0; i_ < 16; ++i_) { vram_put(mt_tiles[MT_GIRDER][2]); vram_put(mt_tiles[MT_GIRDER][3]); }
    /* attributs : sous-titre en palette 1 */
    vram_adr(0x23C0);
    for (i_ = 0; i_ < 64; ++i_) vram_put((i_ >= 16 && i_ < 24) ? 0x55 : 0x00);
    pal_set(pal_title);
    ppu_on();
}

void title_screen(void)
{
    u8 letter, wave, bx, bstate, btimer, menu, blink, sel;
    u16 t;

    title_draw_static();
    music_play(MUS_TITLE);
    letter = 0;
    ly = -24;
    lvy = 0;
    lbounce = 0;
    wave = 0xFF;
    bx = 0;
    bstate = 0;        /* 0 attend, 1 marche, 2 TCHAK, 3 repos */
    btimer = 0;
    menu = 0;
    sel = 0;
    blink = 0;
    t = 0;

    for (;;) {
        ++t;
        oam_clear();

        /* ---- chute des lettres du logo */
        if (letter < 6) {
            if (pad_new & (PAD_START | PAD_A)) {
                for (; letter < 6; ++letter) logo_letter_bg(letter, 1);
                bstate = 1;
                pad_new = 0;
            } else {
                lvy += 1;
                ly += lvy;
                if (ly >= LOGO_Y) {
                    ly = LOGO_Y;
                    if (lvy > 3) {
                        lvy = -(lvy >> 1);
                        sfx_play(SFX_BUMP);
                    } else {
                        logo_letter_bg(letter, 1);
                        ++letter;
                        ly = -24;
                        lvy = 0;
                        if (letter == 6) { bstate = 1; sfx_play(SFX_SELECT); }
                    }
                }
                if (letter < 6 && ly > -24) logo_letter_spr(letter, (u8)ly);
            }
        }

        /* ---- BIDULE arrive, fait TCHAK, les lettres sautent */
        if (bstate == 1) {
            ++bx;
            if (bx >= 112) { bstate = 2; btimer = 0; }
        } else if (bstate == 2) {
            ++btimer;
            if (btimer == 9) { sfx_play(SFX_TCHAK); wave = 0; scroll_x = 2; }
            if (btimer >= 24) { bstate = 3; btimer = 0; }
        } else if (bstate == 3) {
            if (++btimer == 240) { bstate = 2; btimer = 0; }
        }
        if (bstate) {
            if (bstate == 1) k_ = ((bx >> 3) & 1) ? BF_WALK1 : BF_WALK2;
            else if (bstate == 2) k_ = btimer < 4 ? BF_PREP : btimer < 9 ? BF_CONTRACT : btimer < 14 ? BF_BLAST : BF_RECOIL;
            else k_ = (btimer & 64) ? BF_LOOK : BF_IDLE;
            draw_bidule(bx, 120, k_, 0);
        }
        if (wave != 0xFF) {
            scroll_x = (wave < 8 && (wave & 2)) ? 2 : 0;
            for (k_ = 0; k_ < 6; ++k_) {
                i_ = wave - k_ * 8;
                if (wave >= k_ * 8 && i_ < 20) {
                    if (i_ == 0) logo_letter_bg(k_, 0);
                    logo_letter_spr(k_, (u8)(LOGO_Y - (i_ * (20 - i_)) / 4));
                    if (i_ == 19) logo_letter_bg(k_, 1);
                }
            }
            if (++wave > 6 * 8 + 20) wave = 0xFF;
        }

        /* ---- texte / menu */
        if (letter == 6) {
            ++blink;
            if (!menu) {
                if ((blink & 31) == 0) vbuf_text(NT_ADR(8, 21), (blink & 32) ? "                " : "APPUIE SUR START");
                if (pad_new & (PAD_START | PAD_A)) {
                    menu = 1;
                    sfx_play(SFX_SELECT);
                    vbuf_text(NT_ADR(8, 21), "                ");
                    vbuf_text(NT_ADR(11, 21), "COMMENCER");
                    vbuf_text(NT_ADR(11, 23), "MOT DE PASSE");
                }
            } else {
                if (pad_new & (PAD_UP | PAD_DOWN | PAD_SELECT)) { sel ^= 1; sfx_play(SFX_MENU); }
                oam_spr(80, sel ? 184 : 168, FX_CURSOR, 3);
                if (pad_new & (PAD_START | PAD_A)) {
                    sfx_play(SFX_SELECT);
                    if (sel == 0) { progress_new_game(); game_state = GS_INTRO; }
                    else game_state = GS_PASSWORD;
                    pal_fade_black();
                    return;
                }
            }
        }
        frame();
    }
}

/* ======================================================= MOT DE PASSE */
void password_screen(void)
{
    static u8 code[4];
    static char txt[2];
    u8 slot = 0, ok;
    static const char alpha[17] = "BCDFGHJKLMNPRSTV";

    ppu_off();
    oam_clear(); oam_finish();
    screen_clear();
    attr_fill(0);
    put_center(6, "MOT DE PASSE");
    put_center(18, "HAUT/BAS : CHANGER LA LETTRE");
    put_center(20, "GAUCHE/DROITE : CHOISIR");
    put_center(22, "START : VALIDER  B : RETOUR");
    for (slot = 0; slot < 4; ++slot) code[slot] = 0;
    slot = 0;
    vram_adr(NT_ADR(12, 12));
    for (ok = 0; ok < 4; ++ok) { vram_put('B'); vram_put(' '); }
    pal_set(pal_dark);
    ppu_on();
    txt[1] = 0;

    for (;;) {
        oam_clear();
        if (pad_new & PAD_LEFT && slot) { --slot; sfx_play(SFX_MENU); }
        if (pad_new & PAD_RIGHT && slot < 3) { ++slot; sfx_play(SFX_MENU); }
        if (pad_new & (PAD_UP | PAD_DOWN)) {
            code[slot] = (code[slot] + ((pad_new & PAD_UP) ? 1 : 15)) & 15;
            txt[0] = alpha[code[slot]];
            vbuf_text(NT_ADR(12 + slot * 2, 12), txt);
            sfx_play(SFX_MENU);
        }
        if ((frame_cnt & 16)) oam_spr(96 + slot * 16, 104, FX_CURSOR, 0x83);
        if (pad_new & PAD_B) { game_state = GS_TITLE; return; }
        if (pad_new & PAD_START) {
            if (password_check(code)) {
                sfx_play(SFX_SELECT);
                game_state = GS_INTRO;
                pal_fade_black();
                return;
            }
            sfx_play(SFX_SPLAT);
            vbuf_text(NT_ADR(9, 15), "CODE INCORRECT !");
        }
        frame();
    }
}

/* ===================================================== PARTIE TERMINEE */
void gameover_screen(void)
{
    static u8 pw[5];
    ppu_off();
    oam_clear(); oam_finish();
    screen_clear();
    attr_fill(0);
    put_center(9, "PARTIE TERMIN[E");
    put_center(13, "BIDULE EST TOUT COLLANT...");
    password_make(pw);
    put_text(10, 18, "CODE :");
    put_text(17, 18, (const char *)pw);
    put_center(23, "APPUIE SUR START");
    pal_set(pal_dark);
    ppu_on();
    music_play(MUS_DEFEAT);
    for (;;) {
        oam_clear();
        draw_bidule(120, 120, BF_DEFEAT, 0);
        frame();
        if (pad_new & PAD_START) break;
    }
    pal_fade_black();
    game_state = GS_TITLE;
}

/* ======================================================= FIN DE MONDE */
static const char *const end_txt[NWORLDS * 3] = {
    "LE ROI DU REBOND EST VAINCU.", "LA FABRIQUE DES CHOSES MOLLES", "EST SAUV[E... OU PRESQUE.",
    "MADAME ARMOIRE A TOUT RECRACH[.", "LES MEUBLES SE SONT CALM[S...", "POUR CETTE NUIT.",
    "LE GRAND D[RANG[ EST RANG[.", "L'USINE TOURNE ROND...", "ENFIN, PRESQUE ROND.",
    "LE ROI ] L'ENVERS EST REMIS", "] L'ENDROIT. LE MONDE MARCHE", "DROIT... LA T&TE EN BAS.",
};

void __fastcall__ end_screen(u8 world)
{
    u8 t = 0, hop, last;
    last = (room_idx >= TOTAL_ROOMS);
    ppu_off();
    oam_clear(); oam_finish();
    screen_clear();
    attr_fill(0);
    put_center(4, "BRAVO !");
    put_center(8, end_txt[(world - 1) * 3]);
    put_center(10, end_txt[(world - 1) * 3 + 1]);
    put_center(12, end_txt[(world - 1) * 3 + 2]);
    put_text(9, 22, "CHAUSSETTES :");
    put_num(23, 22, socks);
    put_text(10, 26, "FIN DU MONDE");
    vram_adr(NT_ADR(23, 26));
    vram_put('0' + world);
    pal_set(pal_title);
    ppu_on();
    music_play(MUS_ENDING);
    for (;;) {
        oam_clear();
        ++t;
        hop = (t & 31) < 16 ? (t & 15) >> 1 : (15 - (t & 15)) >> 1;
        draw_bidule(120, 136 - hop, BF_VICTORY, (t >> 5) & 1);
        frame();
        if (t > 60 && (pad_new & PAD_START)) break;
    }
    pal_fade_black();
    game_state = last ? GS_TITLE : GS_INTRO;
}
