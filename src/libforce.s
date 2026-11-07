; =====================================================================
;  TCHAK ! - routines de la bibliotheque cc65 forcees dans le moteur commun
;  Une banque (menus, monde) ne doit jamais tirer une routine de none.lib
;  absente du moteur : elle serait placee dans le moteur commun de CETTE
;  banque seulement, qui ne serait plus identique aux autres.
;  tools/make_rom.py detecte ce cas ; la routine manquante s'ajoute ici.
; =====================================================================
        .forceimport pusha, pushax, popa, popax, incsp1, incsp2, incsp3, incsp4
        .forceimport ldaxysp, ldaidx, tosaddax, tossubax, tosmula0, tosmulax
        .forceimport tosdivax, tosmodax, shlax4, shrax4, asrax1, aslax1, boolult
        .forceimport booleq, boolne, decsp1, decsp2, decsp4, pushwysp
        .forceimport aslax3, incax5, mulax9, shrax1, aslax2, shlax3, shrax2, shrax3, incax1, incax2, incax3, incax4, mulax3, mulax5, mulax6, mulax7, mulax10, tosicmp, tosicmp0, toseqax, tosneax, tosltax, tosgtax, tosleax, tosgeax, tosandax, tosorax, tosxorax, negax, complax, bnegax
