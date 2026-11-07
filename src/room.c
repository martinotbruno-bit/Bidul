/* =====================================================================
 *  TCHAK ! - modules 10 et 11 : gestion des salles et des mondes
 *  Chargement, affichage, modifications de decor en cours de jeu,
 *  animations de tuiles, convoyeurs (cycle de palette), bandeau (HUD).
 * ===================================================================== */
#include "nes.h"
#include "game.h"

u8 map[MAP_SIZE];
const Room *room;
s8 conv_dir;
u8 shake;
u8 hit_cx, hit_cy;

static u8 attr_sh[64];
static u8 an_cx[4], an_cy[4], an_mt[4], an_t[4];
static u8 msg_t;


void room_load(u8 idx)
{
    u8 i;
    room = &rooms[idx & 7];
    for (i = 0; i < MAP_SIZE; ++i) map[i] = room->map[i];
    conv_dir = room->conv;
    shake = 0;
    msg_t = 0;
    for (i = 0; i < 4; ++i) an_t[i] = 0;
    objects_init();
    boss_room = 0;
    for (i = 0; i < MAX_OBJ; ++i) if (o_type[i] == OT_KING || o_type[i] == OT_WARDROBE || o_type[i] == OT_DERANGE) boss_room = 1;
    events_init();
    bidule_init(room->bx, room->by);
    tchak_count = 0;
    room_result = 0;
    room_timer = 0;
}

/* ---------------------------------------------- dessin complet (ecran eteint) */
void room_draw(void)
{
    u8 r, c, mt, sy, ai, sh;
    const u8 *t;
    screen_clear();
    for (r = 0; r < 64; ++r) attr_sh[r] = 0x55;        /* bandeau : palette 1 */
    for (r = 0; r < MAP_H; ++r) {
        vram_adr(NT_ADR(0, (r + 2) * 2));
        for (c = 0; c < MAP_W; ++c) {
            t = mt_tiles[map[(r << 4) | c]];
            vram_put(t[0]);
            vram_put(t[1]);
        }
        vram_adr(NT_ADR(0, (r + 2) * 2 + 1));
        for (c = 0; c < MAP_W; ++c) {
            t = mt_tiles[map[(r << 4) | c]];
            vram_put(t[2]);
            vram_put(t[3]);
        }
        sy = r + 2;
        for (c = 0; c < MAP_W; ++c) {
            mt = map[(r << 4) | c];
            ai = ((sy >> 1) << 3) | (c >> 1);
            sh = ((sy & 1) << 2) | ((c & 1) << 1);
            attr_sh[ai] = (attr_sh[ai] & ~(3 << sh)) | (mt_attr[mt] << sh);
        }
    }
    vram_adr(0x23C0);
    vram_write(attr_sh, 64);
    hud_draw_full();
}

/* ---------------------------------------------- modification en cours de jeu */
void map_set(u8 cx, u8 cy, u8 mt)
{
    u16 adr;
    u8 sy, ai, sh;
    map[(cy << 4) | cx] = mt;
    adr = NT_ADR(cx << 1, (cy + 2) << 1);
    vbuf_tiles(adr, &mt_tiles[mt][0], 2);
    vbuf_tiles(adr + 32, &mt_tiles[mt][2], 2);
    sy = cy + 2;
    ai = ((sy >> 1) << 3) | (cx >> 1);
    sh = ((sy & 1) << 2) | ((cx & 1) << 1);
    attr_sh[ai] = (attr_sh[ai] & ~(3 << sh)) | (mt_attr[mt] << sh);
    vbuf_tile(0x23C0 + ai, attr_sh[ai]);
}

/* animation de tuile temporaire (ressort comprime, etc.) */
void tile_anim(u8 cx, u8 cy, u8 mt, u8 restore, u8 frames)
{
    u8 i;
    for (i = 0; i < 4; ++i) {
        if (an_t[i] && an_cx[i] == cx && an_cy[i] == cy) break;
    }
    if (i == 4) {
        for (i = 0; i < 4; ++i) if (!an_t[i]) break;
        if (i == 4) return;
        map_set(cx, cy, mt);
    }
    an_cx[i] = cx; an_cy[i] = cy; an_mt[i] = restore; an_t[i] = frames;
}

void room_update(void)
{
    u8 i, c;
    /* animations de tuiles */
    for (i = 0; i < 4; ++i) {
        if (an_t[i] && --an_t[i] == 0) map_set(an_cx[i], an_cy[i], an_mt[i]);
    }
    /* convoyeurs : cycle de la palette 3 (illusion de mouvement) */
    if (conv_dir && (room_timer & 3) == 0) {
        if (conv_dir > 0) {
            c = palette[15]; palette[15] = palette[14]; palette[14] = palette[13]; palette[13] = c;
        } else {
            c = palette[13]; palette[13] = palette[14]; palette[14] = palette[15]; palette[15] = c;
        }
        pal_dirty = 1;
    }
    /* tremblement de l'ecran apres un TCHAK */
    if (shake) {
        --shake;
        scroll_x = (shake & 2) ? 2 : 0;
    } else {
        scroll_x = 0;
    }
    if (msg_t && --msg_t == 0) {
        vbuf_text(NT_ADR(4, 3), "                        ");
    }
}

/* ---------------------------------------------------------------- bandeau */
void hud_draw_full(void)
{
    put_text(2, 2, "_");
    put_num(3, 2, lives);
    put_text(8, 2, "MONDE");
    vram_adr(NT_ADR(14, 2));
    vram_put('0' + room->world);
    vram_put('-');
    vram_put('0' + room->num);
    put_text(26, 2, "@");
    put_num(27, 2, socks);
}

void hud_update(void)
{
    vbuf_num(NT_ADR(3, 2), lives);
    vbuf_num(NT_ADR(27, 2), socks);
}

void hud_message(const char *s)
{
    vbuf_text(NT_ADR(4, 3), "                        ");
    vbuf_text(NT_ADR(4, 3), s);
    msg_t = 150;
}
