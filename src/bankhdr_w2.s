; =====================================================================
;  TCHAK ! - en-tete de la banque du MONDE 2 (MMC1, $E000)
;  Les meubles a humeurs du monde 2 sont programmes dans world2.c.
; =====================================================================
        .export _bk_title, _bk_password, _bk_gameover, _bk_end
        .export _bk_wupdate, _bk_wdraw, _bk_wtchak, _bk_winit
        .import _w2_update, _w2_draw, _w2_tchak
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
_bk_wupdate:  jmp _w2_update
_bk_wdraw:    jmp _w2_draw
_bk_wtchak:   jmp _w2_tchak
_bk_winit:    rts
              nop
              nop
