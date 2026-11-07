/* =====================================================================
 *  TCHAK ! - module 7 : gestion des reactions en chaine
 *
 *  Declencheurs (definis par salle) :
 *    CIBLE  : touchee par un objet en mouvement  -> actions ON
 *    PLAQUE : poids >= 2 pose dessus             -> ON ; libere -> OFF
 *    LEVIER : bouton A                           -> ON / OFF en alternance
 *  Les actions passent par une FILE avec un petit delai (8 trames) pour
 *  que le joueur voie l'ordre des evenements.
 *
 *  Protection contre les boucles infinies :
 *    - delai de recharge par declencheur (40 trames) ;
 *    - file limitee a 8 actions ;
 *    - "budget" de 16 actions, regenere d'une unite toutes les 8 trames.
 * ===================================================================== */
#include "nes.h"
#include "game.h"

#define QMAX 8
#define TR(t) (room->trigs + (u8)((t) * TRIG_SIZE))

static u8 t_cool[8], t_on[8], t_load[8], t_rel[8];
static u8 q_type[QMAX], q_x[QMAX], q_y[QMAX], q_delay[QMAX];
static u8 q_n;
static u8 budget;
static u8 hint_light;
u8 b_on_plate;
u8 lock_count;

void events_init(void)
{
    u8 i;
    for (i = 0; i < 8; ++i) t_cool[i] = t_on[i] = t_load[i] = t_rel[i] = 0;
    q_n = 0;
    budget = 16;
    hint_light = 0;
    lock_count = 0;
    b_on_plate = 0;
}

static u8 trig_find(u8 cx, u8 cy, u8 kind)
{
    u8 t;
    const u8 *p;
    for (t = 0; t < room->ntrig; ++t) {
        p = TR(t);
        if (p[0] == kind && cx >= p[1] && cx < (u8)(p[1] + p[3]) && cy >= p[2] && cy < (u8)(p[2] + p[4]))
            return t;
    }
    return 0xFF;
}

void act_push(u8 type, u8 x, u8 y)
{
    if (type == A_NONE || q_n >= QMAX) return;
    q_type[q_n] = type;
    q_x[q_n] = x;
    q_y[q_n] = y;
    q_delay[q_n] = 8;
    ++q_n;
}

static void fire(u8 t, u8 on)
{
    const u8 *p = TR(t);
    if (on) {
        act_push(p[5], p[6], p[7]);
        act_push(p[8], p[9], p[10]);
    } else {
        act_push(p[11], p[12], p[13]);
    }
}

void ev_target_hit(u8 cx, u8 cy)
{
    u8 t = trig_find(cx, cy, TK_TARGET);
    if (t == 0xFF || t_cool[t]) return;
    t_cool[t] = 40;
    fire(t, 1);
    tile_anim(cx, cy, MT_TARGET_ON, MT_TARGET, 40);
    sfx_play(SFX_CLICK);
}

void ev_plate_load(u8 cx, u8 cy, u8 w)
{
    u8 t = trig_find(cx, cy, TK_PLATE);
    if (t != 0xFF) t_load[t] += w;
}

u8 ev_lever(u8 cx, u8 cy)
{
    u8 t = trig_find(cx, cy, TK_LEVER);
    if (t == 0xFF) return 0;
    t_on[t] ^= 1;
    /* levier sans action OFF : chaque coup declenche les actions ON */
    fire(t, TR(t)[11] == A_NONE ? 1 : t_on[t]);
    map_set(cx, cy, t_on[t] ? MT_LEVER_ON : MT_LEVER_OFF);
    sfx_play(SFX_LEVER);
    return 1;
}

static void plate_tiles(u8 t, u8 mt)
{
    const u8 *p = TR(t);
    u8 x, y;
    for (y = p[2]; y < (u8)(p[2] + p[4]); ++y)
        for (x = p[1]; x < (u8)(p[1] + p[3]); ++x)
            map_set(x, y, mt);
}

void ev_boss(u8 on)
{
    u8 t;
    for (t = 0; t < room->ntrig; ++t)
        if (TR(t)[0] == TK_BOSS) { fire(t, on); return; }
}

static void execute(u8 type, u8 x, u8 y)
{
    u8 w;
    if (!budget) return;            /* garde-fou : reaction ignoree */
    --budget;
    w = (y >> 4) + 1;               /* trappes : largeur dans les bits 4-6 de y */
    y &= 15;
    switch (type) {
    case A_DOOR_OPEN:
        if (mt_coll[map[(y << 4) | x]] != C_EXIT) {
            map_set(x, y, MT_EXIT_TOP);
            map_set(x, (u8)(y + 1), MT_EXIT_BOT);
            sfx_play(SFX_DOOR);
            bidule_emote(EM_SURPRISE, 30);
        }
        break;
    case A_DOOR_CLOSE:
        if (mt_coll[map[(y << 4) | x]] == C_EXIT) {
            map_set(x, y, MT_DOOR_TOP);
            map_set(x, (u8)(y + 1), MT_DOOR_BOT);
            sfx_play(SFX_BUMP);
        }
        break;
    case A_CONV_RIGHT: conv_dir = 1;  sfx_play(SFX_CLICK); break;
    case A_CONV_LEFT:  conv_dir = -1; sfx_play(SFX_CLICK); break;
    case A_CONV_STOP:  conv_dir = 0;  sfx_play(SFX_CLICK); break;
    case A_CONV_REV:   conv_dir = -conv_dir; sfx_play(SFX_CLICK); break;
    case A_HATCH_OPEN:
        while (w--) map_set(x++, y, MT_EMPTY);
        sfx_play(SFX_LEVER);
        break;
    case A_WORLD:
        bk_waction(x);
        break;
    case A_LOCK:
        /* definitif : annule aussi une animation de tuile en cours sur la case */
        while (w--) {
            tile_anim(x, y, MT_BLOCK, MT_BLOCK, 1);
            map_set(x++, y, MT_BLOCK);
        }
        sfx_play(SFX_DOOR);
        ++lock_count;
        ev_boss(1);
        if (lock_count >= 3) ev_boss(0);
        break;
    case A_HATCH_PULSE:
        while (w--) tile_anim(x++, y, MT_EMPTY, MT_HATCH, 80);
        sfx_play(SFX_LEVER);
        break;
    case A_HATCH_CLOSE:
        while (w--) map_set(x++, y, MT_HATCH);
        sfx_play(SFX_LEVER);
        break;
    case A_WAKE:
        enemies_wake((s8)x);
        break;
    case A_REVEAL:
        o_flags[x] &= ~OF_HIDDEN;
        sfx_play(SFX_SURPRISE);
        break;
    }
}

void events_update(void)
{
    u8 t, i, pressed;
    const u8 *p;

    for (t = 0; t < room->ntrig; ++t) {
        if (t_cool[t]) --t_cool[t];
        p = TR(t);
        if (p[0] != TK_PLATE) continue;
        if (mt_coll[map[(p[2] << 4) | p[1]]] != C_PLATE) continue;   /* verrouillee */
        pressed = t_load[t] >= 2;
        /* indice : BIDULE seul sur une plaque */
        if (t_load[t] == 1 && b_on_plate && !hint_light) {
            hint_light = 1;
            hud_message("BIDULE EST TROP L[GER !");
        }
        t_load[t] = 0;
        if (pressed) {
            t_rel[t] = 0;
            if (!t_on[t]) {
                t_on[t] = 1;
                fire(t, 1);
                plate_tiles(t, MT_PLATE_DN);
                sfx_play(SFX_CLICK);
            }
        } else if (t_on[t]) {
            /* liberation confirmee apres 6 trames (rebonds) */
            if (++t_rel[t] >= 6) {
                t_on[t] = 0;
                fire(t, 0);
                plate_tiles(t, MT_PLATE);
            }
        }
    }

    b_on_plate = 0;
    if ((room_timer & 7) == 0 && budget < 16) ++budget;

    /* file d'actions */
    i = 0;
    while (i < q_n) {
        if (q_delay[i]) {
            --q_delay[i];
            ++i;
            continue;
        }
        execute(q_type[i], q_x[i], q_y[i]);
        --q_n;
        for (t = i; t < q_n; ++t) {
            q_type[t] = q_type[t + 1];
            q_x[t] = q_x[t + 1];
            q_y[t] = q_y[t + 1];
            q_delay[t] = q_delay[t + 1];
        }
    }
}
