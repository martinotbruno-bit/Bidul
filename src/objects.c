/* =====================================================================
 *  TCHAK ! - modules 5 et 6 : gestion des objets et des interactions
 *
 *  Systeme generique de proprietes : chaque type d'objet est decrit par
 *  des tables (masse, elasticite, frottement, poids, reaction au TCHAK).
 *  Les objets sont mis a jour avec une physique entiere simple (pas de
 *  flottants), donc 100 % deterministe : memes entrees => memes effets.
 * ===================================================================== */
#include "nes.h"
#include "game.h"

/* pour les objets, une porte ouverte reste un obstacle (ils ne sortent pas) */
#define OBJ_SOLID(c) (IS_SOLID(c) || (c) == C_EXIT)

u8  o_type[MAX_OBJ], o_flags[MAX_OBJ], o_state[MAX_OBJ], o_timer[MAX_OBJ];
u8  o_anim[MAX_OBJ], o_gnd[MAX_OBJ], o_home_x[MAX_OBJ], o_home_y[MAX_OBJ];
u8  o_wallhit[MAX_OBJ], o_aux[MAX_OBJ];
static u8 o_gcx[MAX_OBJ], o_gcy[MAX_OBJ];   /* case du sol (plaques) */
s8  o_dir[MAX_OBJ];
u16 o_x[MAX_OBJ], o_y[MAX_OBJ];
s8  o_vx[MAX_OBJ], o_vy[MAX_OBJ];

/*                         -    BALLE CAISSE TROUIL. CHAUSS. */
static const u8 p_mass[OT_COUNT]   = { 0,   1,    4,     2,      0 };
static const u8 p_bounce[OT_COUNT] = { 0,   6,    0,     0,      0 };   /* /8 */
static const u8 p_fric[OT_COUNT]   = { 0,   1,    4,     4,      0 };
static const u8 p_body[OT_COUNT]   = { 0,   1,    1,     1,      0 };
static const u8 p_pal[OT_COUNT]    = { 0,   1,    2,     3,      3 };
const u8 p_weight[OT_COUNT]        = { 0,   1,    3,     2,      0 };
const u8 p_tchak_vx[OT_COUNT]      = { 0,  40,   30,    24,      0 };
const u8 p_tchak_vy[OT_COUNT]      = { 0,  56,   24,    32,      0 };

static u8 bump_cool;

/* v * e / 8, en conservant le signe */
s8 s8_scale(s8 v, u8 e)
{
    u8 a;
    if (v < 0) {
        a = (u8)(-v);
        a = (u8)(((u16)a * e) >> 3);
        return -(s8)a;
    }
    a = (u8)v;
    return (s8)(((u16)a * e) >> 3);
}

static void bump(u8 strength)
{
    if (!bump_cool) {
        sfx_play(SFX_BUMP);
        bump_cool = 8;
    }
    (void)strength;
}

void objects_init(void)
{
    u8 i, n;
    const u8 *p = room->objs;
    n = room->nobj;
    for (i = 0; i < MAX_OBJ; ++i) {
        o_type[i] = OT_NONE;
        if (i < n) {
            o_type[i]   = p[0];
            o_home_x[i] = p[1];
            o_home_y[i] = p[2];
            o_flags[i]  = p[3];
            p += 4;
        }
        o_x[i] = SUB(o_home_x[i]);
        o_y[i] = SUB(o_home_y[i]);
        o_vx[i] = o_vy[i] = 0;
        o_state[i] = o_timer[i] = o_anim[i] = o_gnd[i] = o_wallhit[i] = o_aux[i] = 0;
        o_dir[i] = 1;
    }
    bump_cool = 0;
}

/* objet tombe dans la gelee (ou sorti de l'ecran) : reapparait plus tard */
void obj_kill(u8 i)
{
    o_flags[i] |= OF_DEAD;
    o_timer[i] = 90;
    o_vx[i] = o_vy[i] = 0;
    sfx_play(SFX_SPLAT);
}

static void obj_respawn(u8 i)
{
    o_flags[i] &= ~OF_DEAD;
    o_x[i] = SUB(o_home_x[i]);
    o_y[i] = SUB(o_home_y[i]);
    o_vx[i] = o_vy[i] = 0;
    o_state[i] = TS_SLEEP;
}

/* ------------------------------------------------------------------ */
/* variables de travail (statiques : plus rapide sur 6502)              */
static u8 i_, t_, px_, py_, e_, c1_, c2_, cc_, hy_;
static s8 vx_, vy_, push_;
static s16 mv_;
static u16 nx_, ny_;

static void phys(void)
{
    vy_ = o_vy[i_];
    if (vy_ < MAX_FALL) vy_ += GRAV;
    vx_ = o_vx[i_];
    push_ = 0;
    if (o_gnd[i_] == C_CONV) push_ = conv_dir * CONV_SPEED;
    o_wallhit[i_] = 0;

    /* ----- deplacement horizontal */
    mv_ = (s16)vx_ + push_;
    if (mv_) {
        nx_ = (u16)((s16)o_x[i_] + mv_);
        px_ = PIX(nx_);
        py_ = PIX(o_y[i_]);
        e_ = (mv_ > 0) ? (u8)(px_ + 15) : px_;
        hy_ = py_ + 1;
        c1_ = coll_at(e_, hy_);
        c2_ = coll_at(e_, (u8)(py_ + 14));
        if (!OBJ_SOLID(c1_) || (c2_ == C_TARGET && c1_ != C_TARGET)) { c1_ = c2_; hy_ = py_ + 14; }
        if (OBJ_SOLID(c1_)) {
            if (c1_ == C_TARGET && (vx_ >= 12 || vx_ <= -12))
                ev_target_hit(e_ >> 4, (u8)(hy_ - PLAY_TOP) >> 4);
            nx_ = (mv_ > 0) ? SUB((e_ & 0xF0) - 16) : SUB((e_ & 0xF0) + 16);
            if (vx_ >= 24 || vx_ <= -24) { bump(0); o_anim[i_] = 6; }
            vx_ = p_bounce[t_] ? -s8_scale(vx_, p_bounce[t_]) : 0;
            o_wallhit[i_] = 1;
        }
        o_x[i_] = nx_;
    }

    /* ----- deplacement vertical */
    px_ = PIX(o_x[i_]);
    ny_ = (u16)((s16)o_y[i_] + vy_);
    py_ = PIX(ny_);
    o_flags[i_] &= ~OF_GROUND;
    o_gnd[i_] = C_EMPTY;
    if (vy_ >= 0) {
        e_ = py_ + 16;
        c1_ = coll_at((u8)(px_ + 1), e_);
        c2_ = coll_at((u8)(px_ + 14), e_);
        if (OBJ_SOLID(c1_) || OBJ_SOLID(c2_)) {
            ny_ = SUB((e_ & 0xF0) - 16);
            cc_ = coll_at((u8)(px_ + 8), e_);
            if (!OBJ_SOLID(cc_)) cc_ = OBJ_SOLID(c1_) ? c1_ : c2_;
            hit_cx = (u8)(px_ + 8) >> 4;
            hit_cy = (u8)(e_ - PLAY_TOP) >> 4;
            if (cc_ == C_SPRING) {
                /* le ressort projette a la verticale (vitesse horizontale absorbee) */
                vy_ = -SPRING_V;
                vx_ = 0;
                o_x[i_] = SUB(hit_cx << 4);     /* et recentre l'objet sur le ressort */
                tile_anim(hit_cx, hit_cy, MT_SPRING_DN, MT_SPRING, 8);
                sfx_play(SFX_BOING);
                o_anim[i_] = 8;
            } else {
                if (cc_ == C_TARGET && vy_ >= 24) ev_target_hit(hit_cx, hit_cy);
                if (cc_ == C_PLATE) ev_plate_load(hit_cx, hit_cy, p_weight[t_]);
                o_gcx[i_] = hit_cx;
                o_gcy[i_] = hit_cy;
                if (p_bounce[t_] && vy_ >= BOUNCE_MIN) {
                    vy_ = -s8_scale(vy_, p_bounce[t_]);
                    if (vy_ < -20) { bump(0); o_anim[i_] = 6; }
                } else {
                    if (vy_ >= 48) { bump(1); o_anim[i_] = 6; }
                    vy_ = 0;
                    o_flags[i_] |= OF_GROUND;
                    o_gnd[i_] = cc_;
                }
            }
        }
    } else {
        e_ = py_;
        c1_ = coll_at((u8)(px_ + 1), e_);
        c2_ = coll_at((u8)(px_ + 14), e_);
        if (OBJ_SOLID(c1_) || OBJ_SOLID(c2_)) {
            ny_ = SUB((e_ & 0xF0) + 16);
            cc_ = coll_at((u8)(px_ + 8), e_);
            if (cc_ == C_TARGET || c1_ == C_TARGET || c2_ == C_TARGET) {
                if (vy_ <= -12)
                    ev_target_hit((u8)(px_ + 8) >> 4, (u8)(e_ - PLAY_TOP) >> 4);
            }
            if (vy_ <= -24) bump(0);
            vy_ = p_bounce[t_] ? -s8_scale(vy_, 4) : 0;
        }
    }
    o_y[i_] = ny_;

    /* frottement au sol */
    if (o_flags[i_] & OF_GROUND) {
        e_ = p_fric[t_];
        if (vx_ > 0) { vx_ -= e_; if (vx_ < 0) vx_ = 0; }
        else if (vx_ < 0) { vx_ += e_; if (vx_ > 0) vx_ = 0; }
    }
    o_vx[i_] = vx_;
    o_vy[i_] = vy_;

    /* gelee ou sortie de l'ecran */
    py_ = PIX(ny_);
    if (py_ > 236 || coll_at((u8)(px_ + 8), (u8)(py_ + 10)) == C_HAZARD) obj_kill(i_);
}

/* ------------------------------------------------ boites de 16x16 libres ? */
static u8 box_free(u8 x, u8 y)
{
    if (OBJ_SOLID(coll_at((u8)(x + 1), (u8)(y + 1)))) return 0;
    if (OBJ_SOLID(coll_at((u8)(x + 14), (u8)(y + 1)))) return 0;
    if (OBJ_SOLID(coll_at((u8)(x + 1), (u8)(y + 14)))) return 0;
    if (OBJ_SOLID(coll_at((u8)(x + 14), (u8)(y + 14)))) return 0;
    return 1;
}

/* ------------------------------------------------ chocs objet contre objet */
static u8 a_, b_, ax_, ay_, bx_, by_, ox_, oy_;
static s16 dx_, dy_;

static void collide_pair(void)
{
    u8 top, bot, left, right, m1, m2, el;
    s16 vl, vr, rel, P, nvl, nvr;

    if (oy_ <= ox_) {
        /* --- contact vertical */
        if (dy_ > 0) { top = a_; bot = b_; } else { top = b_; bot = a_; }
        if (o_vy[top] >= o_vy[bot]) {
            /* l'objet du haut tombe sur celui du bas : il s'y pose */
            o_y[top] = o_y[bot] - SUB(16);
            if (o_type[bot] == OT_TROU && o_vy[top] >= 24) enemy_scare(bot, o_vx[top] < 0 ? -1 : 1);
            if (p_bounce[o_type[top]] && o_vy[top] >= BOUNCE_MIN) {
                o_vy[top] = -s8_scale(o_vy[top], p_bounce[o_type[top]]);
                o_anim[top] = 6;
            } else {
                o_vy[top] = o_vy[bot] > 0 ? o_vy[bot] : 0;
                o_flags[top] |= OF_GROUND;
                o_gnd[top] = o_gnd[bot];
            }
        } else {
            /* celui du bas monte et cogne celui du haut */
            o_y[bot] = o_y[top] + SUB(16);
            if (o_vy[top] > o_vy[bot] / 2) o_vy[top] = o_vy[bot] / 2;
            o_vy[bot] = 0;
            o_anim[top] = 6;
        }
        return;
    }
    /* --- contact horizontal */
    if (dx_ > 0) { left = a_; right = b_; } else { left = b_; right = a_; }
    vl = o_vx[left];
    vr = o_vx[right];
    rel = vl - vr;
    if (rel > 0) {
        /* ils se rapprochent : echange de quantite de mouvement */
        m1 = p_mass[o_type[left]];
        m2 = p_mass[o_type[right]];
        el = (p_bounce[o_type[left]] | p_bounce[o_type[right]]) ? 6 : 2;
        P = (s16)m1 * vl + (s16)m2 * vr;
        nvl = (P - ((s16)m2 * el * rel) / 8) / (s16)(m1 + m2);
        nvr = (P + ((s16)m1 * el * rel) / 8) / (s16)(m1 + m2);
        if (nvl > 96) nvl = 96; if (nvl < -96) nvl = -96;
        if (nvr > 96) nvr = 96; if (nvr < -96) nvr = -96;
        o_vx[left] = (s8)nvl;
        o_vx[right] = (s8)nvr;
        if (rel >= 24) { bump(0); o_anim[left] = 6; o_anim[right] = 6; }
        if (rel >= 12) {
            if (o_type[right] == OT_TROU) enemy_scare(right, 1);
            if (o_type[left] == OT_TROU) enemy_scare(left, -1);
        }
    }
    /* separation : on deplace celui qui va le plus vite, si la place est libre */
    if ((vl > 0 ? vl : -vl) >= (vr > 0 ? vr : -vr)) {
        if (box_free((u8)(PIX(o_x[left]) - ox_), PIX(o_y[left]))) o_x[left] -= SUB(ox_);
        else o_x[right] += SUB(ox_);
    } else {
        if (box_free((u8)(PIX(o_x[right]) + ox_), PIX(o_y[right]))) o_x[right] += SUB(ox_);
        else o_x[left] -= SUB(ox_);
    }
}

static u8 c_px[MAX_OBJ], c_py[MAX_OBJ], c_list[MAX_OBJ];

static void collisions(void)
{
    u8 n = 0, ia, ib;
    for (a_ = 0; a_ < MAX_OBJ; ++a_) {
        if (!p_body[o_type[a_]] || (o_flags[a_] & (OF_DEAD | OF_HIDDEN))) continue;
        c_px[a_] = PIX(o_x[a_]);
        c_py[a_] = PIX(o_y[a_]);
        c_list[n++] = a_;
    }
    for (ia = 0; ia < n; ++ia) {
        a_ = c_list[ia];
        for (ib = ia + 1; ib < n; ++ib) {
            b_ = c_list[ib];
            ax_ = c_px[a_]; bx_ = c_px[b_];
            if (bx_ >= ax_) { if ((u8)(bx_ - ax_) >= 16) continue; }
            else if ((u8)(ax_ - bx_) >= 16) continue;
            ay_ = c_py[a_]; by_ = c_py[b_];
            if (by_ >= ay_) { if ((u8)(by_ - ay_) >= 16) continue; }
            else if ((u8)(ay_ - by_) >= 16) continue;
            dx_ = (s16)bx_ - ax_;
            dy_ = (s16)by_ - ay_;
            ox_ = 16 - (u8)(dx_ < 0 ? -dx_ : dx_);
            oy_ = 16 - (u8)(dy_ < 0 ? -dy_ : dy_);
            collide_pair();
            c_px[a_] = PIX(o_x[a_]); c_py[a_] = PIX(o_y[a_]);
            c_px[b_] = PIX(o_x[b_]); c_py[b_] = PIX(o_y[b_]);
        }
    }
}

/* ------------------------------------------------ interactions avec BIDULE */
static u8 bpx_, bpy_;
static void touch_bidule(void)
{
    u8 bx = bpx_, by = bpy_;
    px_ = PIX(o_x[i_]); py_ = PIX(o_y[i_]);
    /* recouvrement boite objet / boite BIDULE (x+3..x+12, y+2..y+23) */
    if ((u8)(px_ + 16) <= (u8)(bx + 3) || px_ >= (u8)(bx + 13)) return;
    if ((u8)(py_ + 16) <= (u8)(by + 2) || py_ >= (u8)(by + 24)) return;
    if (b_state == BS_DEAD || b_state == BS_WIN) return;

    switch (t_) {
    case OT_SOCK:
        o_flags[i_] |= OF_HIDDEN;
        if (!(sock_mask[room_idx >> 3] & (1 << (room_idx & 7)))) {
            sock_mask[room_idx >> 3] |= (1 << (room_idx & 7));
            ++socks;
        }
        sfx_play(SFX_SOCK);
        hud_update();
        hud_message("UNE CHAUSSETTE !");
        break;
    case OT_BALL:
        /* la balle retombe sur la tete de BIDULE : il est aplati (gag) */
        if (o_vy[i_] >= 32 && py_ + 8 < by + 8) {
            o_vy[i_] = -s8_scale(o_vy[i_], 6);
            o_y[i_] = SUB(by - 14);
            bidule_emote(EM_SQUASH, 40);
            sfx_play(SFX_SQUASH);
        }
        break;
    case OT_TROU:
        if (o_state[i_] == TS_RUN && b_emote == EM_NONE) bidule_emote(EM_SURPRISE, 30);
        break;
    }
}

void objects_update(void)
{
    if (bump_cool) --bump_cool;
    bpx_ = PIX(b_x);
    bpy_ = PIX(b_y);
    for (i_ = 0; i_ < MAX_OBJ; ++i_) {
        t_ = o_type[i_];
        if (t_ == OT_NONE) continue;
        if (o_flags[i_] & OF_DEAD) {
            if (--o_timer[i_] == 0) obj_respawn(i_);
            continue;
        }
        if (o_flags[i_] & OF_HIDDEN) continue;
        if (o_anim[i_]) --o_anim[i_];
        if (t_ != OT_SOCK) {
            if (t_ == OT_TROU) enemy_update(i_);
            /* objet au repos : on ne refait la physique complete que s'il
               recoit une vitesse, si son convoyeur tourne, ou 1 trame sur 8
               (pour detecter un sol qui disparait). */
            if ((o_flags[i_] & OF_REST) && !o_vx[i_] && !o_vy[i_]
                && !(o_gnd[i_] == C_CONV && conv_dir) && ((room_timer + i_) & 7)) {
                if (o_gnd[i_] == C_PLATE) ev_plate_load(o_gcx[i_], o_gcy[i_], p_weight[t_]);
            } else {
                phys();
                if ((o_flags[i_] & OF_GROUND) && !o_vx[i_] && !o_vy[i_]) o_flags[i_] |= OF_REST;
                else o_flags[i_] &= ~OF_REST;
            }
        }
        touch_bidule();
    }
    collisions();
}

/* ------------------------------------------------------------------ dessin */
static void draw_obj(u8 i)
{
    u8 x, y, f, a, t;
    t = o_type[i];
    if (t == OT_NONE || (o_flags[i] & OF_HIDDEN)) return;
    x = PIX(o_x[i]);
    y = PIX(o_y[i]);
    if (o_flags[i] & OF_DEAD) {
        /* eclaboussures de gelee pendant quelques trames */
        if (o_timer[i] > 70) {
            f = 90 - o_timer[i];
            oam_spr(x - f, y + 4 - f, FX_DROP, 1);
            oam_spr(x + 8 + f, y + 4 - f, FX_DROP, 1);
        }
        return;
    }
    a = p_pal[t];
    switch (t) {
    case OT_BALL:  f = o_anim[i] ? OF_BALL_SQ : OF_BALL; break;
    case OT_CRATE: f = o_anim[i] ? OF_CRATE_HIT : OF_CRATE; break;
    case OT_SOCK:
        f = OF_SOCK;
        y += ((room_timer >> 3) & 1);
        break;
    default: /* OT_TROU */
        switch (o_state[i]) {
        case TS_SLEEP: f = OF_TROU_SLEEP; break;
        case TS_RUN:   f = (room_timer & 4) ? OF_TROU_RUN1 : OF_TROU_RUN2; break;
        default:       f = OF_TROU_ALERT; break;
        }
        if (o_dir[i] < 0) a |= 0x40;
        break;
    }
    m_ptr = obj_frames[f];
    m_x = x; m_y = y; m_attr = a; m_w = 2; m_h = 2;
    oam_meta();
    /* bulles d'emotion du Trouillard */
    if (t == OT_TROU) {
        if (o_state[i] == TS_SLEEP && (room_timer & 32)) oam_spr(x + 12, y - 6, FX_ZZZ, 3);
        if (o_state[i] == TS_ALERT) oam_spr(x + 4, y - 10, FX_BANG, 3);
        if (o_state[i] == TS_PANT && (room_timer & 8)) oam_spr(x + 4, y - 8, FX_DROP, 3);
    }
}

void objects_draw(u8 rot)
{
    u8 k, i;
    for (k = 0; k < MAX_OBJ; ++k) {
        i = (k + rot) & (MAX_OBJ - 1);
        draw_obj(i);
    }
}
