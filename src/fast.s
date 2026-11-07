; =====================================================================
;  TCHAK ! - routines critiques en assembleur (module 12 : collisions)
; =====================================================================
        .export  _coll_at, _oam_spr, _oam_finish, _oam_meta
        .exportzp _m_x, _m_y, _m_attr, _m_ptr, _m_w, _m_h
        .import  _map, _mt_coll, _OAM_BUF
        .importzp _oam_i

.segment "ZEROPAGE"
_m_x:    .res 1
_m_y:    .res 1
_m_attr: .res 1
_m_ptr:  .res 2
_m_w:    .res 1
_m_h:    .res 1
m_cx:    .res 1
m_col:   .res 1
m_row:   .res 1
m_t:     .res 1
        .include "zeropage.inc"

.segment "CODE"

; unsigned char __fastcall__ coll_at(unsigned char x, unsigned char y)
;   renvoie le type de collision de la metatuile sous le pixel (x, y).
;   Au-dessus ou au-dessous de l'aire de jeu : solide.
_coll_at:
        cmp #32
        bcc @solid
        sbc #32                 ; C = 1
        cmp #208
        bcs @solid
        and #$F0
        sta tmp1
        ldy #0
        lda (sp),y              ; x
        lsr a
        lsr a
        lsr a
        lsr a
        ora tmp1
        tay
        lda _map,y
        tay
        lda _mt_coll,y
        jmp @pop
@solid: lda #1
@pop:   inc sp
        bne :+
        inc sp+1
:       ldx #0
        rts

; void __fastcall__ oam_spr(u8 x, u8 y, u8 tile, u8 attr)
;   pile C : (sp)+0 = tile, +1 = y, +2 = x ; A = attr
;   les sprites dont y >= $EF (hors ecran) sont ignores.
_oam_spr:
        ldx _oam_i
        sta _OAM_BUF+2,x
        ldy #1
        lda (sp),y              ; y
        cmp #$EF
        bcs @pop
        sbc #0                  ; C = 0 : y - 1 (le PPU affiche une ligne plus bas)
        sta _OAM_BUF,x
        dey
        lda (sp),y              ; tuile
        sta _OAM_BUF+1,x
        ldy #2
        lda (sp),y              ; x
        sta _OAM_BUF+3,x
        txa
        clc
        adc #4
        sta _oam_i
@pop:   lda sp
        clc
        adc #3
        sta sp
        bcc :+
        inc sp+1
:       rts

; void oam_finish(void) : cache les sprites inutilises
_oam_finish:
        ldx _oam_i
        lda #$F0
@lp:    sta _OAM_BUF,x
        inx
        inx
        inx
        inx
        bne @lp
        rts

; void oam_meta(void) : metasprite de m_w x m_h tuiles (lignes de gauche a
;   droite) a la position (m_x, m_y). Si m_attr a le bit $40 (miroir
;   horizontal), l'ordre des colonnes est inverse. Lignes hors ecran ignorees.
_oam_meta:
        ldx _oam_i
        ldy #0
        lda _m_h
        sta m_row
@row:   lda _m_y
        cmp #$EF
        bcs @skiprow
        lda _m_w
        sta m_col
        lda _m_x
        sta m_cx
        bit _m_attr
        bvc @col
        ; miroir : on part de la derniere tuile de la ligne
        tya
        clc
        adc _m_w
        sta m_t
        tay
@colf:  dey
        lda (_m_ptr),y
        sta _OAM_BUF+1,x
        lda _m_y
        sec
        sbc #1
        sta _OAM_BUF,x
        lda _m_attr
        sta _OAM_BUF+2,x
        lda m_cx
        sta _OAM_BUF+3,x
        clc
        adc #8
        sta m_cx
        inx
        inx
        inx
        inx
        dec m_col
        bne @colf
        ldy m_t
        jmp @next
@col:   lda (_m_ptr),y
        iny
        sta _OAM_BUF+1,x
        lda _m_y
        sec
        sbc #1
        sta _OAM_BUF,x
        lda _m_attr
        sta _OAM_BUF+2,x
        lda m_cx
        sta _OAM_BUF+3,x
        clc
        adc #8
        sta m_cx
        inx
        inx
        inx
        inx
        dec m_col
        bne @col
        jmp @next
@skiprow:
        tya
        clc
        adc _m_w
        tay
@next:  lda _m_y
        clc
        adc #8
        sta _m_y
        dec m_row
        bne @row
        stx _oam_i
        rts
