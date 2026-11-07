/* =====================================================================
 *  TCHAK ! - modules 2 et 3 : commandes et gestion de BIDULE
 *
 *  BIDULE ne saute pas (il en est incapable, ses genoux sont trop gros).
 *  Il marche, court (en maintenant une direction), grimpe aux echelles,
 *  rebondit sur les ressorts, actionne les leviers (A) et fait TCHAK (B).
 *
 *  Boite de collision : x+3..x+12, y+2..y+23 (sprite 16x24).
 * ===================================================================== */
#include "nes.h"
#include "game.h"

u16 b_x, b_y;
s8  b_vx, b_vy;
u8  b_state, b_timer, b_face, b_ground, b_emote, b_emote_t, b_idle, b_frame;
static u8 b_walk, b_hold, b_fall, b_land, b_onobj;

#define WALK_SPEED 16
#define RUN_SPEED  28
#define RUN_DELAY  45

void bidule_init(u8 px, u8 py)
{
    b_x = SUB(px);
    b_y = SUB(py);
    b_vx = b_vy = 0;
    b_state = BS_NORMAL;
    b_timer = 0;
    b_face = 0;
    b_ground = C_SOLID;
    b_emote = EM_NONE;
    b_emote_t = 0;
    b_idle = 0;
    b_walk = b_hold = b_fall = b_land = b_onobj = 0;
}

void bidule_emote(u8 e, u8 t)
{
    if (b_state == BS_DEAD || b_state == BS_WIN) return;
    if (b_emote == EM_SQUASH && e != EM_SQUASH) return;
    b_emote = e;
    b_emote_t = t;
    if (e == EM_SURPRISE) sfx_play(SFX_SURPRISE);
}

static void die(void)
{
    if (b_state == BS_DEAD || b_state == BS_WIN) return;
    b_state = BS_DEAD;
    b_timer = 0;
    b_vx = 0;
    b_vy = 0;
    sfx_play(SFX_SPLAT);
    music_play(MUS_DEFEAT);
}

/* ------------------------------------------------------------------ */
static u8 px_, py_, e_, c1_, c2_, c3_, oldfeet_;
static s16 mv_;

/* sol sous les pieds ? (gere le haut des echelles comme une plate-forme) */
static u8 floor_type(u8 x, u8 y)
{
    u8 c = coll_at(x, y);
    if (IS_SOLID(c)) return c;
    if (c == C_LADDER && b_state != BS_CLIMB && !(pad & PAD_DOWN)) {
        /* haut d'echelle : la case du dessus n'est pas une echelle */
        if (coll_at(x, (u8)(y - 16)) != C_LADDER && oldfeet_ < (y & 0xF0)) return C_SOLID;
    }
    return 0;
}

static void move_x(void)
{
    s8 push = 0;
    if (b_ground == C_CONV) push = conv_dir * CONV_SPEED;
    mv_ = (s16)b_vx + push;
    if (!mv_) return;
    b_x = (u16)((s16)b_x + mv_);
    px_ = PIX(b_x);
    py_ = PIX(b_y);
    e_ = (mv_ > 0) ? (u8)(px_ + 12) : (u8)(px_ + 3);
    c1_ = coll_at(e_, (u8)(py_ + 3));
    c2_ = coll_at(e_, (u8)(py_ + 12));
    c3_ = coll_at(e_, (u8)(py_ + 22));
    if (IS_SOLID(c1_) || IS_SOLID(c2_) || IS_SOLID(c3_)) {
        if (mv_ > 0) b_x = SUB((e_ & 0xF0) - 13);
        else         b_x = SUB((e_ & 0xF0) + 13);
    }
}

static void land_effects(u8 c, u8 cx, u8 cy)
{
    if (c == C_SPRING) {
        b_vy = -SPRING_V;
        tile_anim(cx, cy, MT_SPRING_DN, MT_SPRING, 8);
        sfx_play(SFX_BOING);
        b_ground = 0;
        return;
    }
    if (c == C_PLATE) { ev_plate_load(cx, cy, 1); b_on_plate = 1; }
    if (b_fall > 36) b_land = 10;          /* grosse chute : il s'ecrase un peu */
    b_fall = 0;
}

static void move_y(void)
{
    u8 i, ox, oy, c;
    oldfeet_ = PIX(b_y) + 23;
    b_y = (u16)((s16)b_y + b_vy);
    px_ = PIX(b_x);
    py_ = PIX(b_y);
    b_ground = 0;
    b_onobj = 0;
    if (b_vy >= 0) {
        e_ = py_ + 24;
        c1_ = floor_type((u8)(px_ + 4), e_);
        c2_ = floor_type((u8)(px_ + 11), e_);
        if (c1_ || c2_) {
            b_y = SUB((e_ & 0xF0) - 24);
            c = floor_type((u8)(px_ + 8), e_);
            if (!c) c = c1_ ? c1_ : c2_;
            b_vy = 0;
            b_ground = c;
            land_effects(c, (u8)(px_ + 8) >> 4, (u8)(e_ - PLAY_TOP) >> 4);
            return;
        }
        /* se poser sur une caisse */
        for (i = 0; i < MAX_OBJ; ++i) {
            if (o_type[i] != OT_CRATE || (o_flags[i] & (OF_DEAD | OF_HIDDEN))) continue;
            ox = PIX(o_x[i]);
            oy = PIX(o_y[i]);
            if ((u8)(ox + 16) <= (u8)(px_ + 4) || ox >= (u8)(px_ + 12)) continue;
            if (oldfeet_ < oy && (u8)(py_ + 23) >= oy) {
                b_y = SUB(oy - 24);
                b_vy = 0;
                b_ground = C_SOLID;
                b_onobj = i + 1;
                land_effects(C_SOLID, 0, 0);
                return;
            }
        }
    } else {
        e_ = py_ + 2;
        if (IS_SOLID(coll_at((u8)(px_ + 4), e_)) || IS_SOLID(coll_at((u8)(px_ + 11), e_))) {
            b_y = SUB((e_ & 0xF0) + 16 - 2);
            b_vy = 0;
        }
    }
}

static void start_tchak(void)
{
    b_state = BS_TCHAK;
    b_timer = 0;
    b_vx = 0;
    b_idle = 0;
}

static void input_normal(void)
{
    u8 cx, cy;
    if (b_emote == EM_SQUASH) { b_vx = 0; return; }
    if (b_land) {
        --b_land;
        b_vx = 0;
        if (pad_new & PAD_B) { b_land = 0; start_tchak(); }
        return;
    }

    if (pad & (PAD_LEFT | PAD_RIGHT)) {
        if (b_hold < 255) ++b_hold;
        b_face = (pad & PAD_LEFT) ? 1 : 0;
        b_vx = (b_hold > RUN_DELAY) ? RUN_SPEED : WALK_SPEED;
        if (b_face) b_vx = -b_vx;
        ++b_walk;
        b_idle = 0;
    } else {
        b_hold = 0;
        b_vx = 0;
        b_walk = 0;
        if (b_idle < 255) ++b_idle;
    }
    if (pad_new & PAD_B) { start_tchak(); return; }

    cx = (u8)(PIX(b_x) + 8) >> 4;
    cy = (u8)(PIX(b_y) + 12 - PLAY_TOP) >> 4;
    if (pad_new & PAD_A) {
        if (ev_lever(cx, cy)) { b_idle = 0; bidule_emote(EM_SURPRISE, 20); }
    }
    /* echelles */
    if ((pad & PAD_UP) && coll_at((u8)(PIX(b_x) + 8), (u8)(PIX(b_y) + 12)) == C_LADDER) {
        b_state = BS_CLIMB;
    } else if ((pad & PAD_DOWN) && b_ground &&
               coll_at((u8)(PIX(b_x) + 8), (u8)(PIX(b_y) + 24)) == C_LADDER) {
        b_state = BS_CLIMB;
        b_y += SUB(2);
    }
    if (b_state == BS_CLIMB) {
        b_x = SUB((u8)(PIX(b_x) + 8) & 0xF0);
        b_vx = b_vy = 0;
        b_idle = 0;
    }
}

static void update_climb(void)
{
    u8 x = PIX(b_x) + 8, feet;
    b_vx = 0;
    b_vy = 0;
    if (pad & (PAD_LEFT | PAD_RIGHT)) {        /* on lache l'echelle */
        b_state = BS_NORMAL;
        return;
    }
    if (pad & PAD_UP) {
        b_y -= 16;
        feet = PIX(b_y) + 23;
        if (coll_at(x, feet) != C_LADDER) {
            /* sommet atteint : debout sur le haut de l'echelle */
            b_y = SUB((u8)((feet & 0xF0) + 16 - 24));
            b_state = BS_NORMAL;
            b_ground = C_SOLID;
        } else if (IS_SOLID(coll_at(x, (u8)(PIX(b_y) + 2)))) {
            b_y += 16;
        }
        ++b_walk;
    } else if (pad & PAD_DOWN) {
        b_y += 16;
        feet = PIX(b_y) + 24;
        if (IS_SOLID(coll_at(x, feet))) {
            b_y = SUB((feet & 0xF0) - 24);
            b_state = BS_NORMAL;
            b_ground = C_SOLID;
        } else if (coll_at(x, (u8)(feet - 1)) != C_LADDER && coll_at(x, (u8)(PIX(b_y) + 12)) != C_LADDER) {
            b_state = BS_NORMAL;
        }
        ++b_walk;
    }
}

void bidule_update(void)
{
    u8 cx, cy;
    if (b_emote_t && --b_emote_t == 0) b_emote = EM_NONE;

    switch (b_state) {
    case BS_NORMAL:
        input_normal();
        break;
    case BS_CLIMB:
        update_climb();
        if (b_state == BS_CLIMB) goto checks;
        break;
    case BS_TCHAK:
        ++b_timer;
        if (b_timer == 9) tchak_fire();
        if (b_timer >= 14 && b_timer < 24) {
            /* recul */
            b_vx = (b_timer < 19) ? 12 : 6;
            if (!b_face) b_vx = -b_vx;
        } else {
            b_vx = 0;
        }
        if (b_timer >= 24) b_state = BS_NORMAL;
        break;
    case BS_WIN:
        ++b_timer;
        b_vx = 0;
        if (b_timer >= 100) room_result = 1;
        break;
    case BS_DEAD:
        ++b_timer;
        if ((b_timer & 7) == 0 && b_timer < 40) b_y += SUB(1);   /* s'enfonce */
        if (b_timer >= 110) room_result = 2;
        return;
    }

    /* physique : pas de gravite pendant la contraction (il "plane") */
    if (b_state == BS_TCHAK && b_timer < 14 && !b_ground) {
        b_vy = 0;
    } else if (b_vy < MAX_FALL) {
        b_vy += GRAV;
    }
    move_x();
    move_y();
    if (!b_ground && b_vy > 16) { if (b_fall < 255) ++b_fall; }

checks:
    px_ = PIX(b_x);
    py_ = PIX(b_y);
    if (py_ > 228 || coll_at((u8)(px_ + 8), (u8)(py_ + 20)) == C_HAZARD) {
        die();
        return;
    }
    if (b_state != BS_WIN && coll_at((u8)(px_ + 8), (u8)(py_ + 12)) == C_EXIT) {
        b_state = BS_WIN;
        b_timer = 0;
        b_vx = 0;
        music_play(MUS_VICTORY);
    }
    (void)cx; (void)cy;
}

/* ------------------------------------------------------------------ dessin */
void bidule_draw(void)
{
    u8 f, x, y, k;

    switch (b_state) {
    case BS_DEAD:
        f = BF_DEFEAT;
        break;
    case BS_WIN:
        f = BF_VICTORY;
        break;
    case BS_CLIMB:
        f = BF_CLIMB;
        b_face = (b_walk >> 3) & 1;
        break;
    case BS_TCHAK:
        if (b_timer < 4)       f = BF_PREP;
        else if (b_timer < 9)  f = BF_CONTRACT;
        else if (b_timer < 14) f = BF_BLAST;
        else                   f = BF_RECOIL;
        break;
    default:
        if (b_emote == EM_SQUASH)      f = BF_SQUASH;
        else if (b_land)               f = BF_PREP;
        else if (!b_ground && b_vy > 24) f = BF_FALL;
        else if (!b_ground && b_vy < -24) f = BF_SURPRISE;
        else if (b_vx) {
            if (b_hold > RUN_DELAY) f = (b_walk & 4) ? BF_RUN1 : BF_RUN2;
            else {
                k = (b_walk >> 3) & 3;
                f = (k == 0) ? BF_WALK1 : (k == 2) ? BF_WALK2 : BF_IDLE;
            }
        }
        else if (b_emote == EM_SURPRISE) f = BF_SURPRISE;
        else if (b_idle > 150) {
            /* observation : il regarde autour de lui */
            f = BF_LOOK;
            if ((b_idle & 63) == 0) b_face ^= 1;
            if (b_idle == 255) b_idle = 151;
        }
        else f = BF_IDLE;
        break;
    }
    b_frame = f;
    x = PIX(b_x);
    y = PIX(b_y);
    if (b_state == BS_WIN) y -= (b_timer & 16) ? ((b_timer & 15) < 8 ? (b_timer & 7) : 8 - (b_timer & 7)) : 0;
    m_ptr = bidule_frames[f];
    m_x = x; m_y = y; m_attr = b_face ? 0x40 : 0x00; m_w = 2; m_h = 3;
    oam_meta();
    y = PIX(b_y);
    if (b_emote == EM_SURPRISE && b_state == BS_NORMAL) oam_spr(x + 4, y - 10, FX_BANG, 3);
    if (b_emote == EM_SQUASH || b_emote == EM_DIZZY) {
        k = room_timer & 15;
        oam_spr(x + (k < 8 ? k : 15 - k), y + 4, FX_STAR, 3);
        oam_spr(x + 8 - (k < 8 ? k : 15 - k), y + 1, FX_STAR, 3);
    }
}
