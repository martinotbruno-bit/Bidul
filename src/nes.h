/* =====================================================================
 *  TCHAK ! - interface materielle (crt0.s) et variables globales
 * ===================================================================== */
#ifndef NES_H
#define NES_H
#include "defs.h"

/* page zero (crt0.s) */
extern u8 frame_cnt, pad, pad_new, nmi_ready;
extern u8 ppu_ctrl, ppu_mask, scroll_x, scroll_y, pal_dirty;
#pragma zpsym("frame_cnt")
#pragma zpsym("pad")
#pragma zpsym("pad_new")
#pragma zpsym("nmi_ready")
#pragma zpsym("ppu_ctrl")
#pragma zpsym("ppu_mask")
#pragma zpsym("scroll_x")
#pragma zpsym("scroll_y")
#pragma zpsym("pal_dirty")

extern u8 OAM_BUF[256];
extern u8 vram_buf[128];
extern u8 palette[32];

void __fastcall__ ppu_wait_nmi(void);
void __fastcall__ ppu_off(void);
void __fastcall__ ppu_on(void);
u8   __fastcall__ pad_poll(void);
void __fastcall__ vram_adr(u16 adr);
void __fastcall__ vram_put(u8 v);
void __fastcall__ vram_fill(u8 v, u16 len);
void __fastcall__ vram_write(const u8 *src, u16 len);

/* son (sound.s) */
void __fastcall__ music_play(u8 song);
void __fastcall__ music_stop(void);
void __fastcall__ music_pause(u8 on);
void __fastcall__ sfx_play(u8 id);

#define NT_ADR(x, y) (0x2000 + ((u16)(y) << 5) + (x))

/* ---- video.c --------------------------------------------------------- */
extern u8 vbuf_len;
extern u8 oam_i;
#pragma zpsym("vbuf_len")
#pragma zpsym("oam_i")
void vbuf_reset(void);
void vbuf_tiles(u16 adr, const u8 *src, u8 n);
void vbuf_tile(u16 adr, u8 t);
void vbuf_text(u16 adr, const char *s);
void vbuf_num(u16 adr, u8 n);
u8   vbuf_room(void);
void oam_clear(void);
void __fastcall__ oam_spr(u8 x, u8 y, u8 tile, u8 attr);
void __fastcall__ oam_finish(void);
/* metasprites (fast.s) : parametres en page zero */
extern u8 m_x, m_y, m_attr, m_w, m_h;
extern const u8 *m_ptr;
#pragma zpsym("m_x")
#pragma zpsym("m_y")
#pragma zpsym("m_attr")
#pragma zpsym("m_w")
#pragma zpsym("m_h")
#pragma zpsym("m_ptr")
void __fastcall__ oam_meta(void);
void pal_set(const u8 *p);
void pal_fade_black(void);
void screen_clear(void);
void put_text(u8 x, u8 y, const char *s);
void put_num(u8 x, u8 y, u8 n);
void attr_fill(u8 v);
void wait_frames(u8 n);

#endif
