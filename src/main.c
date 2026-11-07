/* =====================================================================
 *  TCHAK ! - LA GRANDE CATASTROPHE
 *  module 1 : boucle principale et machine a etats du jeu
 * ===================================================================== */
#include "nes.h"
#include "game.h"

u8 lag_frames;          /* trames ou la logique a deborde (controle perf) */
u8 paused;
static u8 last_frame, rot;

static void play_frame_end(void)
{
    oam_clear();
    bidule_draw();
    tchak_draw();
    objects_draw(rot);
    rot += 3;
    oam_finish();
    ppu_wait_nmi();
    vbuf_len = 0;
    if ((u8)(frame_cnt - last_frame) > 1) ++lag_frames;
    last_frame = frame_cnt;
    pad_poll();
}

static void start_room(void)
{
    ppu_off();
    vbuf_reset();
    room_load(room_idx);
    room_draw();
    pal_set(pal_world);
    oam_clear();
    oam_finish();
    ppu_on();
    vbuf_len = 0;
    last_frame = frame_cnt;
    music_play(boss_room ? MUS_BOSS : MUS_WORLD);
    hud_message(room->name);
}

static void play_room(void)
{
    start_room();
    paused = 0;
    for (;;) {
        /* ---- pause (START) / recommencer (SELECT) */
        if (pad_new & PAD_START && b_state != BS_WIN && b_state != BS_DEAD) {
            paused ^= 1;
            music_pause(paused);
            sfx_play(SFX_MENU);
            if (paused) hud_message("PAUSE  SELECT:RECOMMENCER");
            else hud_message(" ");
        }
        if ((pad_new & PAD_SELECT) && b_state != BS_WIN && b_state != BS_DEAD) {
            pal_fade_black();
            start_room();
            paused = 0;
            pad_new = 0;
            continue;
        }
        if (!paused) {
            bidule_update();
            objects_update();
            events_update();
            room_update();
            ++room_timer;
            if (b_state == BS_WIN && b_timer == 1) hud_message("BRAVO !");
            if (b_state == BS_DEAD && b_timer == 1) hud_message("A%E ! DANS LA GEL[E !");
        }
        play_frame_end();

        if (room_result == 1) {
            pal_fade_black();
            ++room_idx;
            /* fin d'un monde (8e salle) : ecran de fin de monde */
            game_state = ((room_idx & 7) == 0 || room_idx >= TOTAL_ROOMS) ? GS_END : GS_INTRO;
            return;
        }
        if (room_result == 2) {
            pal_fade_black();
            if (lives) --lives;
            if (lives == 0) { game_state = GS_GAMEOVER; return; }
            start_room();
        }
    }
}

/* banque (et table de decors) du monde de la salle courante */
void world_bank(void)
{
    u8 w = room_idx >> 3;
    bank_set(1 + w);
    chr_set(w);
}

static void menu_bank(void)
{
    bank_set(BANK_MENU);
    chr_set(0);
}

void main(void)
{
    game_state = GS_TITLE;
    progress_new_game();
    for (;;) {
        switch (game_state) {
        case GS_TITLE:    menu_bank(); bk_title();    break;
        case GS_PASSWORD: menu_bank(); bk_password(); break;
        case GS_INTRO:    world_bank(); intro_screen(); break;
        case GS_PLAY:     world_bank(); play_room();  break;
        case GS_GAMEOVER: menu_bank(); bk_gameover(); break;
        case GS_END:      menu_bank(); bk_end((u8)((room_idx - 1) >> 3) + 1); break;
        default:          game_state = GS_TITLE; break;
        }
    }
}
