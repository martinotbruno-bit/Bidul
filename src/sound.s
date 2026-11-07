; =====================================================================
;  TCHAK ! - moteur son APU (module 14 : gestion du son)
;
;  6 flux : 0 = carre 1, 1 = carre 2, 2 = triangle, 3 = bruit  (musique)
;           4 = carre 2, 5 = bruit                              (bruitages)
;  Un bruitage actif sur un canal masque la musique de ce canal.
;
;  Format d'un flux (octets) :
;    $00-$5F nn dd   note nn (table de periodes ; bruit : periode+mode)
;                     duree dd trames
;    $60 dd          silence de dd trames
;    $F0 vv          instrument : rapport cyclique (bits 7-6) + volume (3-0)
;    $F1 lo hi       saut (boucle)
;    $F2             fin du flux
;    $F3 rr          decroissance : -1 de volume toutes les rr trames (0 = aucune)
;    $F4 ss          glissement : ss (signe) ajoute a la periode a chaque trame
; =====================================================================
        .export  sound_init, sound_update
        .export  _music_play, _music_stop, _sfx_play, _music_pause
        .export  _st_active = st_active, _st_lo = st_lo   ; (pour les tests)
        .import  period_lo, period_hi, song_table, sfx_table, _world_song

NSTREAM = 6

.segment "ZEROPAGE"
sptr:     .res 2
stmp:     .res 1

.segment "BSS"
st_lo:     .res NSTREAM
st_hi:     .res NSTREAM
st_timer:  .res NSTREAM
st_active: .res NSTREAM
st_vol:    .res NSTREAM
st_base:   .res NSTREAM
st_duty:   .res NSTREAM
st_env:    .res NSTREAM
st_envc:   .res NSTREAM
st_plo:    .res NSTREAM
st_phi:    .res NSTREAM
st_slide:  .res NSTREAM
st_rest:   .res NSTREAM
st_new:    .res NSTREAM
last_hi:   .res 4
snd_busy:  .res 1
mus_pause: .res 1

.segment "RODATA"
chan_of:   .byte 0, 1, 2, 3, 1, 3

.segment "CODE"

sound_init:
        lda #$0F
        sta $4015
        lda #$30
        sta $4000
        sta $4004
        sta $400C
        lda #$08
        sta $4001
        sta $4005
        lda #$80
        sta $4008
        ldx #NSTREAM-1
        lda #0
@c:     sta st_active,x
        dex
        bpl @c
        lda #$FF
        ldx #3
@h:     sta last_hi,x
        dex
        bpl @h
        lda #0
        sta snd_busy
        sta mus_pause
        rts

; ---------------------------------------------------------------------
; void music_play(unsigned char song)
_music_play:
        inc snd_busy
        cmp #$FF                ; MUS_WORLD : theme du monde en place
        bne @std
        lda #<_world_song       ; (en-tete de la banque du monde, adresse fixe)
        sta sptr
        lda #>_world_song
        sta sptr+1
        jmp @go
@std:   asl a
        asl a
        asl a                   ; 8 octets par morceau (4 pointeurs)
        clc
        adc #<song_table
        sta sptr
        lda #>song_table
        adc #0
        sta sptr+1
@go:    ldy #0
        ldx #0
@lp:    lda (sptr),y
        sta st_lo,x
        iny
        lda (sptr),y
        sta st_hi,x
        iny
        ora st_lo,x
        beq :+
        lda #1
:       sta st_active,x
        sty stmp
        jsr stream_reset
        ldy stmp
        inx
        cpx #4
        bne @lp
        lda #0
        sta mus_pause
        jsr silence_music
        dec snd_busy
        rts

; void music_stop(void)
_music_stop:
        inc snd_busy
        lda #0
        ldx #3
@lp:    sta st_active,x
        dex
        bpl @lp
        jsr silence_music
        dec snd_busy
        rts

; void music_pause(unsigned char on)
_music_pause:
        sta mus_pause
        jmp silence_music

silence_music:
        lda #$30
        sta $4000
        sta $400C
        ldx st_active+4
        bne :+
        sta $4004
:       lda #$80
        sta $4008
        lda #$FF
        sta last_hi
        sta last_hi+1
        sta last_hi+2
        rts

; void sfx_play(unsigned char id)
_sfx_play:
        inc snd_busy
        asl a
        asl a                   ; 4 octets : flux carre 2, flux bruit
        tay
        ldx #4
@lp:    lda sfx_table,y
        sta st_lo,x
        lda sfx_table+1,y
        sta st_hi,x
        ora st_lo,x
        beq @skip               ; pas de flux : on laisse l'eventuel bruitage en cours
        lda #1
        sta st_active,x
        jsr stream_reset
@skip:  iny
        iny
        inx
        cpx #6
        bne @lp
        dec snd_busy
        rts

; X = flux
stream_reset:
        lda #0
        sta st_timer,x
        sta st_env,x
        sta st_slide,x
        sta st_rest,x
        lda #$B0
        sta st_duty,x
        lda #$0F
        sta st_base,x
        rts

; ---------------------------------------------------------------------
sound_update:
        lda snd_busy
        bne @out
        ldx #0
@lp:    lda st_active,x
        beq @next
        cpx #4
        bcs @run
        lda mus_pause
        bne @next
@run:   lda st_timer,x
        beq @read
        dec st_timer,x
        bne @tick
@read:  jsr stream_read
        lda st_active,x
        beq @next
        jmp @outp
@tick:  jsr stream_tick
@outp:  jsr stream_output
@next:  inx
        cpx #NSTREAM
        bne @lp
@out:   rts

; --- lecture des commandes jusqu'a une note ou un silence (X = flux)
stream_read:
        lda st_lo,x
        sta sptr
        lda st_hi,x
        sta sptr+1
        ldy #0
@cmd:   lda (sptr),y
        iny
        cmp #$60
        bcc @note
        beq @rest
        cmp #$F0
        beq @instr
        cmp #$F1
        beq @jump
        cmp #$F3
        beq @envc
        cmp #$F4
        beq @slide
        ; $F2 ou inconnu : fin du flux
        lda #0
        sta st_active,x
        jmp stream_silence
@instr: lda (sptr),y
        iny
        pha
        and #$C0
        ora #$30
        sta st_duty,x
        pla
        and #$0F
        sta st_base,x
        jmp @cmd
@envc:  lda (sptr),y
        iny
        sta st_env,x
        jmp @cmd
@slide: lda (sptr),y
        iny
        sta st_slide,x
        jmp @cmd
@jump:  lda (sptr),y
        pha
        iny
        lda (sptr),y
        sta sptr+1
        pla
        sta sptr
        ldy #0
        jmp @cmd
@rest:  lda #1
        sta st_rest,x
        jmp @dur
@note:  sta stmp
        lda chan_of,x
        cmp #3
        bne @tone
        lda stmp                ; bruit : bits 0-3 periode, bit 4 mode
        and #$10
        beq :+
        lda #$80
:       sta st_phi,x
        lda stmp
        and #$0F
        ora st_phi,x
        sta st_plo,x
        jmp @on
@tone:  lda stmp                ; index de note
        sty stmp
        tay
        lda period_lo,y
        sta st_plo,x
        lda period_hi,y
        sta st_phi,x
        ldy stmp
@on:    lda #0
        sta st_rest,x
        lda st_base,x
        sta st_vol,x
        lda st_env,x
        sta st_envc,x
        lda #1
        sta st_new,x
@dur:   lda (sptr),y
        iny
        sta st_timer,x
        tya
        clc
        adc sptr
        sta st_lo,x
        lda sptr+1
        adc #0
        sta st_hi,x
        rts

; --- evolution par trame : enveloppe et glissement (X = flux)
stream_tick:
        lda st_env,x
        beq @noenv
        dec st_envc,x
        bne @noenv
        sta st_envc,x
        lda st_vol,x
        beq @noenv
        dec st_vol,x
@noenv: lda st_slide,x
        beq @end
        clc
        adc st_plo,x
        sta st_plo,x
        lda st_slide,x
        bmi @neg
        lda st_phi,x
        adc #0
        jmp @st
@neg:   lda st_phi,x
        adc #$FF
@st:    and #$07
        sta st_phi,x
@end:   rts

; --- ecriture dans l'APU (X = flux)
stream_output:
        lda chan_of,x
        sta stmp
        cpx #4
        bcs @ok                 ; bruitage : toujours
        cmp #1
        bne :+
        lda st_active+4
        bne @skip
:       lda stmp
        cmp #3
        bne @ok
        lda st_active+5
        bne @skip
@ok:    lda stmp
        asl a
        asl a
        tay                     ; Y = 4 * canal
        lda stmp
        cmp #2
        beq @tri
        cmp #3
        beq @noise
        ; --- carres
        lda st_rest,x
        bne @pmute
        lda st_duty,x
        ora st_vol,x
        sta $4000,y
        lda st_plo,x
        sta $4002,y
        lda st_phi,x
        ldy stmp
        cmp last_hi,y
        beq @skip
        sta last_hi,y
        pha
        tya
        asl a
        asl a
        tay
        pla
        ora #$08
        sta $4003,y
@skip:  lda #0
        sta st_new,x
        rts
@pmute: lda #$30
        sta $4000,y
        jmp @skip
@tri:   lda st_rest,x
        bne @tmute
        lda #$81
        sta $4008
        lda st_plo,x
        sta $400A
        lda st_phi,x
        cmp last_hi+2
        beq @skip
        sta last_hi+2
        sta $400B
        jmp @skip
@tmute: lda #$80
        sta $4008
        jmp @skip
@noise: lda st_rest,x
        bne @nmute
        lda st_vol,x
        ora #$30
        sta $400C
        lda st_plo,x
        sta $400E
        lda st_new,x
        beq @skip
        lda #$08
        sta $400F
        jmp @skip
@nmute: lda #$30
        sta $400C
        jmp @skip

; silence du canal d'un flux qui se termine (X = flux)
stream_silence:
        lda chan_of,x
        tay
        lda #$FF
        sta last_hi,y
        cpy #2
        beq @t
        tya
        asl a
        asl a
        tay
        lda #$30
        sta $4000,y
        rts
@t:     lda #$80
        sta $4008
        rts
