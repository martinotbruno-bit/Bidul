; =====================================================================
;  TCHAK ! - en-tete de la banque du MONDE 3 (MMC1, $E000)
;  Pistons, Copieur, Machinal et Grand Derange : world3.c.
; =====================================================================
        .export _bk_title, _bk_password, _bk_gameover, _bk_end
        .export _bk_wupdate, _bk_wdraw, _bk_wtchak, _bk_waction
        .import _w3_update, _w3_draw, _w3_tchak, _w3_action
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
_bk_wupdate:  jmp _w3_update
_bk_wdraw:    jmp _w3_draw
_bk_wtchak:   jmp _w3_tchak
_bk_waction:  jmp _w3_action
