; =====================================================================
;  TCHAK ! - en-tete de la banque du MONDE 1 (MMC1, $E000)
;  Le monde 1 n'a pas de code propre : tout est dans le moteur commun.
; =====================================================================
        .export _bk_title, _bk_password, _bk_gameover, _bk_end
        .export _bk_wupdate, _bk_wdraw, _bk_wtchak, _bk_winit
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
_bk_wupdate:  rts
              nop
              nop
_bk_wdraw:    rts
              nop
              nop
_bk_wtchak:   rts
              nop
              nop
_bk_winit:    rts
              nop
              nop
