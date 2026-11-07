; =====================================================================
;  TCHAK ! - demarrage, NMI, manette, acces PPU (module 1 : processeur et
;  boucle principale, cote assembleur)
; =====================================================================
        .export   __STARTUP__ : absolute = 1
        .export   _bank_set, _chr_set
        .exportzp _cur_bank
        .export   _ppu_wait_nmi, _ppu_off, _ppu_on, _pad_poll
        .export   _vram_adr, _vram_put, _vram_fill, _vram_write
        .exportzp _frame_cnt, _pad, _pad_new, _nmi_ready
        .exportzp _idle_cnt
        .exportzp _ppu_ctrl, _ppu_mask, _scroll_x, _scroll_y, _pal_dirty
        .export   _OAM_BUF, _vram_buf, _palette
        .import   _main, zerobss, copydata, initlib
        .import   sound_init, sound_update
        .include  "zeropage.inc"

PPUCTRL   = $2000
PPUMASK   = $2001
PPUSTATUS = $2002
OAMADDR   = $2003
PPUSCROLL = $2005
PPUADDR   = $2006
PPUDATA   = $2007
OAMDMA    = $4014
JOY1      = $4016

; ---------------------------------------------------------------------
.segment "ZEROPAGE"
_frame_cnt: .res 1
_pad:       .res 1
_pad_new:   .res 1
_pad_prev:  .res 1
_nmi_ready: .res 1
_ppu_ctrl:  .res 1
_ppu_mask:  .res 1
_scroll_x:  .res 1
_scroll_y:  .res 1
_pal_dirty: .res 1
vptr:       .res 2
_idle_cnt:  .res 2
_cur_bank:  .res 1

.segment "OAM"
_OAM_BUF:   .res 256

.segment "BSS"
_vram_buf:  .res 128            ; file de mises a jour VRAM pour le NMI
_palette:   .res 32             ; copie de la palette (envoyee si pal_dirty)

; ---------------------------------------------------------------------
.segment "STARTUP"
reset:
        sei
        cld
        ldx #$40
        stx $4017               ; pas d'IRQ trame APU
        ldx #$FF
        txs
        inx                     ; X = 0
        stx PPUCTRL
        stx PPUMASK
        stx $4010               ; pas d'IRQ DMC
        bit PPUSTATUS
@vb1:   bit PPUSTATUS           ; attente 1re VBlank
        bpl @vb1
        txa
@clr:   sta $00,x               ; RAM a zero
        sta $0100,x
        sta $0300,x
        sta $0400,x
        sta $0500,x
        sta $0600,x
        sta $0700,x
        lda #$F0
        sta $0200,x             ; sprites caches
        lda #0
        inx
        bne @clr
@vb2:   bit PPUSTATUS           ; attente 2e VBlank : PPU pret
        bpl @vb2

        ; pile logicielle C en haut de la RAM
        lda #<$0800
        sta sp
        lda #>$0800
        sta sp+1
        jsr zerobss
        jsr copydata
        jsr initlib

        lda #$FF
        sta _vram_buf
        ; --- MMC1 : PRG en mode 32 Ko, CHR en mode 8 Ko, miroir vertical
        lda #$80
        sta $8000               ; remise a zero du registre a decalage
        lda #$02
        ldx #5
@ctl:   sta $8000
        lsr a
        dex
        bne @ctl
        lda #0
        jsr _bank_set           ; banque 0 : menus
        lda #0
        jsr _chr_set
        jsr sound_init
        lda #%10001000          ; NMI on, sprites en $1000, decor en $0000
        sta _ppu_ctrl
        sta PPUCTRL
        jmp _main

; ---------------------------------------------------------------------
;  NMI : DMA sprites, palette, file VRAM, defilement, son
; ---------------------------------------------------------------------
nmi:
        pha
        txa
        pha
        tya
        pha
        lda _nmi_ready
        bne :+
        jmp @no_ppu
:

        lda #0
        sta OAMADDR
        lda #>_OAM_BUF
        sta OAMDMA

        lda _pal_dirty
        beq @no_pal
        bit PPUSTATUS
        lda #$3F
        sta PPUADDR
        ldx #0
        stx PPUADDR
@pal:   lda _palette,x
        sta PPUDATA
        inx
        cpx #32
        bne @pal
        lda #0
        sta _pal_dirty
@no_pal:
        ; file VRAM : [adr_hi, adr_lo, long (bit7 = vertical), octets...] ... $FF
        ldx #0
@next:  lda _vram_buf,x
        bmi @vdone
        bit PPUSTATUS
        sta PPUADDR
        lda _vram_buf+1,x
        sta PPUADDR
        lda _vram_buf+2,x
        bpl @horiz
        and #$7F
        tay
        lda _ppu_ctrl
        ora #$04
        bne @setc
@horiz: tay
        lda _ppu_ctrl
@setc:  sta PPUCTRL
        inx
        inx
        inx
@copy:  lda _vram_buf,x
        sta PPUDATA
        inx
        dey
        bne @copy
        jmp @next
@vdone:
        lda #$FF
        sta _vram_buf
        lda _ppu_ctrl
        sta PPUCTRL
        bit PPUSTATUS
        lda _scroll_x
        sta PPUSCROLL
        lda _scroll_y
        sta PPUSCROLL
        lda _ppu_mask
        sta PPUMASK
        lda #0
        sta _nmi_ready
@no_ppu:
        inc _frame_cnt
        jsr sound_update
        pla
        tay
        pla
        tax
        pla
irq:    rti

; ---------------------------------------------------------------------
.segment "CODE"

; void bank_set(u8 b) : banque PRG de 32 Ko (MMC1, registre $E000)
;   Le moteur est identique dans toutes les banques : l'execution continue.
_bank_set:
        sta _cur_bank
        asl a                   ; en mode 32 Ko le bit 0 est ignore
        ldx #5
@w:     sta $E000
        lsr a
        dex
        bne @w
        rts

; void chr_set(u8 b) : banque CHR de 8 Ko (registre $A000)
_chr_set:
        asl a
        ldx #5
@w:     sta $A000
        lsr a
        dex
        bne @w
        rts

; void ppu_wait_nmi(void) : valide la trame et attend la VBlank suivante
_ppu_wait_nmi:
        lda #1
        sta _nmi_ready
        lda #0
        sta _idle_cnt
        sta _idle_cnt+1
        lda _frame_cnt
@w:     inc _idle_cnt           ; mesure du temps libre (~14 cycles/tour)
        bne :+
        inc _idle_cnt+1
:       cmp _frame_cnt
        beq @w
        rts

; void ppu_off(void) : coupe l'affichage (apres la VBlank)
_ppu_off:
        lda #0
        sta _ppu_mask
        jsr _ppu_wait_nmi
        lda #0
        sta PPUMASK
        rts

; void ppu_on(void) : rallume decor + sprites a la prochaine VBlank
_ppu_on:
        lda #%00011110
        sta _ppu_mask
        jmp _ppu_wait_nmi

; unsigned char pad_poll(void) : lit la manette 1 (A B Sel Start H B G D)
_pad_poll:
        lda _pad
        sta _pad_prev
        ldx #1
        stx JOY1
        dex
        stx JOY1
        ldx #8
@rd:    lda JOY1
        lsr a
        rol _pad
        dex
        bne @rd
        lda _pad_prev
        eor #$FF
        and _pad
        sta _pad_new
        lda _pad
        ldx #0
        rts

; void vram_adr(unsigned int adr)   (ecran eteint)
_vram_adr:
        bit PPUSTATUS
        stx PPUADDR
        sta PPUADDR
        rts

; void vram_put(unsigned char v)
_vram_put:
        sta PPUDATA
        rts

; void vram_fill(unsigned char v, unsigned int len)  -> fastcall : len dans A/X
_vram_fill:
        sta ptr1
        stx ptr1+1
        ldy #0
        lda (sp),y              ; v
        tay
        inc sp
        bne :+
        inc sp+1
:       tya
        ldx ptr1+1
        beq @rest
        ldy #0
@page:  sta PPUDATA
        iny
        bne @page
        dex
        bne @page
@rest:  ldy ptr1
        beq @end
@lp:    sta PPUDATA
        dey
        bne @lp
@end:   rts

; void vram_write(const unsigned char *src, unsigned int len)
_vram_write:
        sta ptr2
        stx ptr2+1              ; longueur
        ldy #1
        lda (sp),y
        sta ptr1+1
        dey
        lda (sp),y
        sta ptr1
        lda sp
        clc
        adc #2
        sta sp
        bcc :+
        inc sp+1
:       ldy #0
@lp:    lda ptr2
        ora ptr2+1
        beq @end
        lda (ptr1),y
        sta PPUDATA
        inc ptr1
        bne :+
        inc ptr1+1
:       lda ptr2
        bne :+
        dec ptr2+1
:       dec ptr2
        jmp @lp
@end:   rts

; ---------------------------------------------------------------------
.segment "VECTORS"
        .word nmi
        .word reset
        .word irq

