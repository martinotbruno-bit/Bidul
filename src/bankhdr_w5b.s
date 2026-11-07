; =====================================================================
;  TCHAK ! - en-tete de la banque du MONDE 5b (MMC1, $E000)
;  Le Grand Bazar : aiguillage dans world5.c.
; =====================================================================
        .export _bk_title, _bk_password, _bk_gameover, _bk_end
        .export _bk_wupdate, _bk_wdraw, _bk_wtchak, _bk_waction
        .import _w5b_update, _w5b_draw, _w5b_tchak, _w5b_action
.segment "BANKHDR"
_bk_title:    rts
              nop
              nop
_bk_password: rts
              nop
              nop
_bk_gameover: rts
              nop
              nop
_bk_end:      rts
              nop
              nop
_bk_wupdate:  jmp _w5b_update
_bk_wdraw:    jmp _w5b_draw
_bk_wtchak:   jmp _w5b_tchak
_bk_waction:  jmp _w5b_action
