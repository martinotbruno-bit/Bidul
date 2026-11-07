; =====================================================================
;  TCHAK ! - en-tete de la BANQUE DES MENUS (MMC1, $E000)
;  Toutes les banques ont une table de sauts de meme forme au meme endroit :
;  le moteur commun appelle bk_xxx sans savoir quelle banque est en place.
; =====================================================================
        .export _bk_title, _bk_password, _bk_gameover, _bk_end
        .export _bk_wupdate, _bk_wdraw, _bk_wtchak, _bk_waction
        .import _title_screen, _password_screen, _gameover_screen, _end_screen
.segment "BANKHDR"
_bk_title:    jmp _title_screen
_bk_password: jmp _password_screen
_bk_gameover: jmp _gameover_screen
_bk_end:      jmp _end_screen
_bk_wupdate:  rts
              nop
              nop
_bk_wdraw:    rts
              nop
              nop
_bk_wtchak:   rts
              nop
              nop
_bk_waction:    rts
              nop
              nop
