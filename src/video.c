/* =====================================================================
 *  TCHAK ! - module 13 : gestion des graphismes
 *  File de mises a jour VRAM (envoyee par le NMI), sprites, palettes.
 * ===================================================================== */
#include "nes.h"

#pragma bss-name(push, "ZEROPAGE")
u8 vbuf_len;
u8 oam_i;
#pragma bss-name(pop)

#define PPU_ADDR (*(volatile u8 *)0x2006)
#define PPU_DATA (*(volatile u8 *)0x2007)

void vbuf_reset(void)
{
    vbuf_len = 0;
    vram_buf[0] = 0xFF;
}

/* place libre dans la file (une trame) */
u8 vbuf_room(void)
{
    return (u8)(120 - vbuf_len);
}

void vbuf_tiles(u16 adr, const u8 *src, u8 n)
{
    u8 i = vbuf_len;
    if ((u8)(i + n + 4) > 124) return;     /* file pleine : ignore */
    vram_buf[i++] = (u8)(adr >> 8);
    vram_buf[i++] = (u8)adr;
    vram_buf[i++] = n;
    while (n--) vram_buf[i++] = *src++;
    vram_buf[i] = 0xFF;
    vbuf_len = i;
}

void vbuf_tile(u16 adr, u8 t)
{
    vbuf_tiles(adr, &t, 1);
}

void vbuf_text(u16 adr, const char *s)
{
    u8 n = 0;
    while (s[n]) ++n;
    vbuf_tiles(adr, (const u8 *)s, n);
}

static u8 numbuf[3];
void vbuf_num(u16 adr, u8 n)
{
    numbuf[0] = '0' + n / 10;
    numbuf[1] = '0' + n % 10;
    vbuf_tiles(adr, numbuf, 2);
}

/* ---------------------------------------------------------------- OAM */
void oam_clear(void)
{
    oam_i = 0;
}

/* oam_spr() et oam_finish() : voir fast.s */

/* ------------------------------------------------------------ palettes */
void pal_set(const u8 *p)
{
    u8 i;
    for (i = 0; i < 32; ++i) palette[i] = p[i];
    pal_dirty = 1;
}

void pal_fade_black(void)
{
    u8 step, i, c;
    for (step = 0; step < 4; ++step) {
        for (i = 0; i < 32; ++i) {
            c = palette[i];
            if (c >= 0x10) c -= 0x10; else c = 0x0F;
            palette[i] = c;
        }
        pal_dirty = 1;
        wait_frames(3);
    }
}

/* ----------------------------------------- dessin, ecran eteint */
void screen_clear(void)
{
    vram_adr(0x2000);
    vram_fill(0, 0x800);
}

void put_text(u8 x, u8 y, const char *s)
{
    vram_adr(NT_ADR(x, y));
    while (*s) vram_put((u8)*s++);
}

void put_num(u8 x, u8 y, u8 n)
{
    vram_adr(NT_ADR(x, y));
    vram_put('0' + n / 10);
    vram_put('0' + n % 10);
}

void attr_fill(u8 v)
{
    vram_adr(0x23C0);
    vram_fill(v, 64);
}

void wait_frames(u8 n)
{
    while (n--) ppu_wait_nmi();
}
