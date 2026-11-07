/* =====================================================================
 *  TCHAK ! - etat partage entre les modules de jeu
 * ===================================================================== */
#ifndef GAME_H
#define GAME_H
#include "defs.h"
#include "gen/gfx.h"
#include "gen/rooms.h"
#include "gen/music.h"

/* ---- physique (unites : 1/16 px, 1/16 px par trame) ------------------ */
#define GRAV        3
#define MAX_FALL    96
#define SPRING_V    116
#define CONV_SPEED  12
#define BOUNCE_MIN  24

/* ---- progression (progress.c) ---------------------------------------- */
extern u8 game_state;
extern u8 room_idx;
extern u8 lives;
extern u8 socks;            /* chaussettes ramassees (total)              */
extern u8 sock_mask[5];     /* une chaussette par salle au plus : bits    */
extern u8 room_result;      /* 0 en cours, 1 gagnee, 2 perdue             */
extern u8 room_timer;       /* compteur de trames de la salle            */
extern u8 rng_unused;
void progress_new_game(void);
void password_make(u8 *out);
u8   password_check(const u8 *in);

/* ---- salle (room.c) --------------------------------------------------- */
extern u8 map[MAP_SIZE];
extern const Room *room;
extern s8 conv_dir;
extern u8 shake;
extern u8 hit_cx, hit_cy;   /* case touchee par la derniere collision     */
u8   __fastcall__ coll_at(u8 x, u8 y);
void room_load(u8 idx);
void room_draw(void);
void map_set(u8 cx, u8 cy, u8 mt);
void tile_anim(u8 cx, u8 cy, u8 mt, u8 restore, u8 frames);
void room_update(void);
void hud_draw_full(void);
void hud_update(void);
void hud_message(const char *s);

/* ---- objets (objects.c) ----------------------------------------------- */
extern u8  o_type[MAX_OBJ], o_flags[MAX_OBJ], o_state[MAX_OBJ], o_timer[MAX_OBJ];
extern u8  o_anim[MAX_OBJ], o_gnd[MAX_OBJ], o_home_x[MAX_OBJ], o_home_y[MAX_OBJ];
extern s8  o_dir[MAX_OBJ];
extern u16 o_x[MAX_OBJ], o_y[MAX_OBJ];
extern s8  o_vx[MAX_OBJ], o_vy[MAX_OBJ];
extern u8  o_wallhit[MAX_OBJ], o_aux[MAX_OBJ];
extern const u8 p_weight[OT_COUNT];
extern const u8 p_tchak_vx[OT_COUNT];
extern const u8 p_tchak_vy[OT_COUNT];
void objects_init(void);
void objects_update(void);
void objects_draw(u8 rot);
void obj_kill(u8 i);
s8   s8_scale(s8 v, u8 eighths);

/* ---- ennemis (enemies.c) --------------------------------------------- */
#define TS_SLEEP  0
#define TS_ALERT  1
#define TS_RUN    2
#define TS_PANT   3
void enemy_update(u8 i);
void enemy_scare(u8 i, s8 dir);
void enemies_wake(s8 dir);
void boss_update(u8 i);
void boss_hit(u8 i);
extern u8 boss_room;

/* ---- BIDULE (bidule.c) ------------------------------------------------ */
#define BS_NORMAL  0
#define BS_TCHAK   1
#define BS_CLIMB   2
#define BS_WIN     3
#define BS_DEAD    4
#define EM_NONE     0
#define EM_SURPRISE 1
#define EM_SQUASH   2
#define EM_DIZZY    3
extern u16 b_x, b_y;
extern s8  b_vx, b_vy;
extern u8  b_state, b_timer, b_face, b_ground, b_emote, b_emote_t, b_idle;
extern u8  b_frame;
void bidule_init(u8 px, u8 py);
void bidule_update(void);
void bidule_draw(void);
void bidule_emote(u8 e, u8 t);

/* ---- TCHAK (tchak.c) -------------------------------------------------- */
extern u8 tchak_fx;          /* minuterie de l'onde visible              */
extern u8 tchak_count;       /* nombre de TCHAK dans la salle (stats)    */
void tchak_fire(void);
void tchak_draw(void);

/* ---- evenements (events.c) ------------------------------------------- */
void events_init(void);
void events_update(void);
void ev_target_hit(u8 cx, u8 cy);
void ev_plate_load(u8 cx, u8 cy, u8 w);
u8   ev_lever(u8 cx, u8 cy);
void act_push(u8 type, u8 x, u8 y);
void ev_boss(u8 on);
extern u8 b_on_plate;

/* ---- ecrans (menus.c) ------------------------------------------------- */
/* banque des menus : appels par la table de sauts (bankhdr) */
void bk_title(void);
void bk_password(void);
void bk_gameover(void);
void __fastcall__ bk_end(u8 world);
void title_screen(void);
void password_screen(void);
void gameover_screen(void);
void __fastcall__ end_screen(u8 world);
/* ecrans communs (screens.c) */
void intro_screen(void);
void frame(void);
u8   strlen8(const char *s);
void put_center(u8 y, const char *s);
void draw_bidule(u8 x, u8 y, u8 f, u8 flip);
extern const u8 pal_dark[32];
/* banques MMC1 (crt0.s) */
#define BANK_MENU 0
void __fastcall__ bank_set(u8 b);
void __fastcall__ chr_set(u8 b);
void world_bank(void);
void __fastcall__ bk_wupdate(u8 i);
void __fastcall__ bk_wdraw(u8 i);
void __fastcall__ bk_wtchak(u8 i);
void __fastcall__ bk_waction(u8 x);
extern s8 tk_dir;
extern u8 tk_adx, tk_ady;
extern u8 lock_count;
extern u8 grav_up;           /* monde a l'envers : les objets tombent vers le haut */
void tchak_impulse(u8 i);

#endif
