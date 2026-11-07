; =====================================================================
;  TCHAK ! - en-tete de la banque du MONDE 4 (MMC1, $E000)
;  Gravite inversee, ballons, aimants, Roi a l Envers : world4.c.
; =====================================================================
        .export _bk_title, _bk_password, _bk_gameover, _bk_end
        .export _bk_wupdate, _bk_wdraw, _bk_wtchak, _bk_waction
        .import _w4_update, _w4_draw, _w4_tchak, _w4_action
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
_bk_wupdate:  jmp _w4_update
_bk_wdraw:    jmp _w4_draw
_bk_wtchak:   jmp _w4_tchak
_bk_waction:  jmp _w4_action
