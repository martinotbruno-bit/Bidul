#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Tests automatiques de la ROM TCHAK ! (emulateur headless cynes).

Chaque test joue reellement au jeu avec des entrees manette scriptees et
verifie l'etat de la RAM (lu grace aux etiquettes ld65).
Usage : python3 tests/test_rom.py [--shots]   (--shots : captures dans docs/img)
"""
import os, sys, time, traceback
sys.path.insert(0, os.path.dirname(__file__))
from nesharness import Game, A, B, SELECT, START, UP, DOWN, LEFT, RIGHT, ROOT

SHOTS = "--shots" in sys.argv
IMG = os.path.join(ROOT, "docs", "img")
RESULTS = []

ALPHA = "BCDFGHJKLMNPRSTV"

def password(room, socks):
    chk = (room + socks * 3 + 5) & 15
    v = (room | (socks << 6) | (chk << 12)) ^ 0x5A3C
    return "".join(ALPHA[(v >> s) & 15] for s in (12, 8, 4, 0))

def shot(g, name):
    if SHOTS:
        os.makedirs(IMG, exist_ok=True)
        g.screenshot(os.path.join(IMG, name + ".png"))

def test(fn):
    RESULTS.append(fn)
    return fn

def check(cond, msg):
    if not cond:
        raise AssertionError(msg)

def door_open(g, cx, cy):
    # metatuile EXIT_TOP = 9
    return g.u8("map", cy * 16 + cx) == 9

def climb_left_ladder(g, max_x):
    g.until(lambda: g.bx() <= max_x, 200, LEFT)
    ok = g.until(lambda: g.u8("b_state") == 0 and g.by() < 120, 200, UP)
    g.run(4, UP)
    return ok

# =================================================================== tests
@test
def t01_demarrage_et_titre():
    """La ROM demarre, l'ecran titre s'anime, le menu mene au jeu."""
    g = Game()
    g.run(120)
    shot(g, "01_titre_chute_logo")
    g.run(380)
    check(g.u8("game_state") == 0, "pas sur l'ecran titre")
    shot(g, "02_titre")
    g.press(START, 2, 20)
    shot(g, "03_menu")
    g.press(START, 2, 40)
    check(g.u8("game_state") == 3, "pas d'ecran de presentation")
    shot(g, "04_presentation")
    g.wait_room(0)
    check(g.u8("lives") == 5, "vies != 5")
    shot(g, "05_salle1")

def door1_open(g):
    return door_open(g, 1, 4)

def room1_correct_side(g):
    """Salle 1-1 : se placer a DROITE de la balle (entre elle et la gelee)."""
    g.until(lambda: g.bx() >= 146, 100, RIGHT)
    g.run(1, LEFT)

@test
def t02_salle1_premier_tchak():
    """Salle 1-1 : depuis la droite de la balle, TCHAK -> cible -> porte ; echelle -> sortie."""
    g = Game(); g.boot_to_room1()
    room1_correct_side(g)
    g.press(B, 2, 0)
    g.run(10)
    shot(g, "06_tchak")
    check(g.obj(0)["vx"] < 0, "la balle n'est pas partie vers la gauche")
    check(g.until(lambda: door1_open(g), 200), "la porte ne s'ouvre pas")
    g.until(lambda: g.bx() <= 72, 300, LEFT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 120, 200, UP), "echelle")
    g.run(4, UP)
    check(g.until(lambda: g.u8("b_state") == 3, 300, LEFT), "BIDULE n'atteint pas la sortie")
    g.wait_room(1)

@test
def t03_salle1_geste_reflexe():
    """Salle 1-1 : le TCHAK reflexe (au depart) envoie la balle dans la gelee ; elle reapparait."""
    g = Game(); g.boot_to_room1()
    g.press(B, 2, 0)
    check(g.until(lambda: g.obj(0)["flags"] & 4, 200), "la balle n'est pas tombee dans la gelee")
    check(not door1_open(g), "la porte s'est ouverte par hasard")
    check(g.until(lambda: not (g.obj(0)["flags"] & 4) and g.obj(0)["x"] == 128, 200),
          "la balle n'est pas revenue a sa place")
    g.run(10, LEFT)
    g.press(B, 2, 30)
    check(g.obj(0)["x"] == 128, "TCHAK hors de portee : la balle n'aurait pas du bouger")

def room2_on_ledge(g):
    """Salle 1-4 : caisse sur le ressort, puis BIDULE sur le rebord a droite de la colonne."""
    g.press(B, 2, 0)
    g.run(30)
    g.until(lambda: g.bx() <= 136, 200, LEFT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 110, 200, UP), "echelle")
    g.run(4, UP)
    g.until(lambda: g.bx() <= 130, 60, LEFT)

def crate_level(g):
    """Vrai quand la caisse monte et passe a hauteur du rebord (fenetre de TCHAK)."""
    c = g.obj(0)
    return c["x"] == 112 and c["vy"] < 0 and 112 <= c["y"] <= 126

@test
def t04_salle2_tchak_au_vol():
    """Salle 1-4 : caisse -> ressort -> TCHAK au vol depuis le rebord -> etagere -> plaque -> porte."""
    g = Game(); g.goto_room(3)
    room2_on_ledge(g)
    check(g.obj(0)["x"] == 112, "la caisse n'est pas sur le ressort")
    check(g.until(lambda: crate_level(g), 200), "la caisse ne passe pas a hauteur du rebord")
    g.press(B, 2, 0)
    check(g.until(lambda: door_open(g, 14, 9), 200), "la porte ne s'ouvre pas")
    shot(g, "07_caisse_etagere")
    g.until(lambda: g.u8("b_state") == 0 and g.by() > 180, 300, DOWN)
    check(g.until(lambda: g.u8("b_state") == 3, 300, RIGHT), "sortie non atteinte")

@test
def t05_salle2_mauvais_moment():
    """Salle 1-4 : TCHAK au mauvais moment -> la caisse continue de rebondir, porte fermee."""
    g = Game(); g.goto_room(3)
    room2_on_ledge(g)
    g.until(lambda: g.obj(0)["y"] > 170 and g.obj(0)["vy"] > 0, 200)   # caisse tout en bas
    g.press(B, 2, 120)
    check(not door_open(g, 14, 9), "porte ouverte alors que le TCHAK a rate")
    check(g.obj(0)["x"] == 112, "la caisse aurait du rester sur le ressort")

@test
def t06_salle2_chaussette_du_ressort():
    """Salle 1-4, secret : BIDULE rebondit lui-meme sur le ressort et attrape la chaussette."""
    g = Game(); g.goto_room(3)
    g.until(lambda: g.bx() <= 113, 200, LEFT)        # au-dessus du ressort, on lache tout
    check(g.until(lambda: g.u8("socks") == 1, 200), "chaussette non ramassee")
    shot(g, "09_chaussette")
    out = False
    for _ in range(600):
        g.run(1, RIGHT)
        if g.u8("b_ground") and g.bx() > 150:
            out = True
            break
    check(out, "BIDULE reste coince sur le ressort")

def room3_bridge(g):
    g.until(lambda: g.bx() >= 216, 100, RIGHT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 110, 200, UP), "echelle")
    g.run(3, UP)
    g.press(A, 2, 30)
    check(g.u8("map", 11 * 16 + 5) == 17 and g.u8("map", 11 * 16 + 6) == 17, "pas de planche sur la cuve")

def door3_open(g):
    return door_open(g, 1, 9)

@test
def t07_salle3_trouillard_dans_la_gelee():
    """Salle 1-6 : effrayer le Trouillard sans planche -> il fuit dans la gelee (et reapparait)."""
    g = Game(); g.goto_room(5)
    g.press(B, 2, 0)
    check(g.until(lambda: g.obj(0)["flags"] & 4, 200), "le Trouillard n'est pas tombe dans la cuve")
    check(not door3_open(g), "porte ouverte")
    check(g.until(lambda: g.obj(0)["state"] == 0 and not (g.obj(0)["flags"] & 4), 200), "pas de retour")

@test
def t08_salle3_solution_trouillard():
    """Salle 1-6, solution 1 : levier (planche sur la cuve) PUIS effrayer le Trouillard."""
    g = Game(); g.goto_room(5)
    room3_bridge(g)
    shot(g, "11_levier_planche")
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 200, DOWN)
    g.until(lambda: g.bx() <= 170, 100, LEFT)
    g.press(B, 2, 0)
    check(g.until(lambda: g.obj(0)["state"] == 2, 60), "le Trouillard ne s'enfuit pas")
    shot(g, "10_trouillard_fuit")
    check(g.until(lambda: door3_open(g), 300), "porte fermee")
    check(g.until(lambda: g.u8("b_state") == 3, 400, LEFT), "sortie non atteinte")

@test
def t09_salle3_solution_plateforme():
    """Salle 1-6, solution 2 : planche, puis TCHAK sur la caisse depuis la plate-forme :
    la caisse tombe et le Trouillard (reveille) fuit lui aussi vers la plaque."""
    g = Game(); g.goto_room(5)
    room3_bridge(g)
    g.until(lambda: g.bx() <= 168, 100, LEFT)
    g.press(B, 2, 0)
    check(g.until(lambda: door3_open(g), 400), "porte fermee")
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 300, DOWN)
    check(g.until(lambda: g.u8("b_state") == 3, 600, LEFT), "sortie non atteinte")

@test
def t10_gelee_perd_une_vie():
    """Tomber dans la gelee coute une vie et recommence la salle."""
    g = Game(); g.goto_room(5)
    check(g.until(lambda: g.u8("b_state") == 4, 300, LEFT), "BIDULE ne tombe pas dans la cuve")
    g.run(20)
    shot(g, "12_gelee")
    g.wait_room(5)
    check(g.u8("lives") == 4, "vie non retiree")
    check(g.bx() == 176, "BIDULE n'est pas revenu au depart")

@test
def t11_game_over():
    """Plus de vies : ecran de fin de partie, puis retour au titre."""
    g = Game(); g.goto_room(5)
    g.poke("lives", 1)
    g.until(lambda: g.u8("b_state") == 4, 300, LEFT)
    check(g.until(lambda: g.u8("game_state") == 7, 400), "pas d'ecran de fin de partie")
    g.run(30)
    shot(g, "13_game_over")
    g.press(START, 2, 40)
    check(g.until(lambda: g.u8("game_state") == 0, 100), "pas de retour au titre")

@test
def t12_pause_et_recommencer():
    """START met en pause (le jeu est fige), SELECT recommence la salle sans perte de vie."""
    g = Game(); g.boot_to_room1()
    g.press(B, 2, 12)
    g.press(START, 2, 2)
    check(g.u8("paused") == 1, "pas de pause")
    x0 = g.obj(0)["x"]
    g.run(30, RIGHT)
    check(g.obj(0)["x"] == x0, "le jeu n'est pas fige en pause")
    shot(g, "14_pause")
    g.press(SELECT, 2, 2)
    g.wait_room(0)
    check(g.obj(0)["x"] == 128 and g.u8("lives") == 5, "la salle n'a pas ete reinitialisee")

@test
def t13_mot_de_passe():
    """Le mot de passe affiche permet de reprendre a la salle 1-6."""
    g = Game()
    g.run(500)
    g.press(START, 2, 20)
    g.press(DOWN, 2, 4)
    g.press(START, 2, 40)
    check(g.u8("game_state") == 2, "pas d'ecran de mot de passe")
    code = password(5, 1)
    for slot, ch in enumerate(code):
        for _ in range(ALPHA.index(ch)):
            g.press(UP, 2, 2)
        if slot < 3:
            g.press(RIGHT, 2, 2)
    shot(g, "15_mot_de_passe")
    g.press(START, 2, 30)
    g.wait_room(5)
    check(g.u8("socks") == 1, "chaussettes non restaurees")

@test
def t14_mot_de_passe_faux():
    """Un mot de passe invalide est refuse."""
    g = Game()
    g.run(500)
    g.press(START, 2, 20)
    g.press(DOWN, 2, 4)
    g.press(START, 2, 40)
    g.press(START, 2, 10)       # BBBB
    check(g.u8("game_state") == 2, "code invalide accepte")

GAMEPLAY_VARS = [("map", 208), ("o_x", 16), ("o_y", 16), ("o_vx", 8), ("o_vy", 8), ("o_state", 8),
                 ("o_flags", 8), ("o_timer", 8), ("b_x", 2), ("b_y", 2), ("b_vx", 1), ("b_vy", 1),
                 ("b_state", 1), ("b_timer", 1), ("conv_dir", 1), ("room_timer", 1), ("socks", 1)]

def gameplay_state(g):
    """Etat de jeu (hors son : l'alignement CPU/PPU a l'allumage peut decaler les
    minuteries musicales d'une trame d'une instance d'emulateur a l'autre)."""
    out = bytearray()
    for name, n in GAMEPLAY_VARS:
        base = g.L["_" + name]
        out += bytes(g.nes[base + k] for k in range(n))
    return bytes(out)

@test
def t15_determinisme():
    """Memes entrees => memes effets : deux parties identiques donnent la meme RAM."""
    traces = []
    for _ in range(2):
        g = Game(); g.goto_room(3)
        g.press(B, 2, 0)
        tr = []
        for f in range(240):
            g.run(1, RIGHT if 100 < f < 140 else 0)
            tr.append(gameplay_state(g))
        traces.append(tr)
    check(traces[0] == traces[1], "divergence entre deux executions identiques")

@test
def t16_fin_du_prototype():
    """Apres le boss (salle 1-8), l'ecran de fin du monde 1 s'affiche."""
    g = Game(); g.goto_room(7)
    g.poke("room_result", 1)
    check(g.until(lambda: g.u8("game_state") == 8, 300), "pas d'ecran de fin du monde 1")
    g.run(90)
    shot(g, "16_fin")

@test
def t17_performances():
    """Aucune trame perdue (la logique tient dans 1/60 s) pendant une reaction en chaine."""
    g = Game(); g.goto_room(5)
    room3_bridge(g)
    g.poke("lag_frames", 0)
    g.until(lambda: g.bx() <= 168, 100, LEFT)
    g.press(B, 2, 0)
    g.run(300, 0)
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 300, DOWN)
    g.run(200, LEFT)
    lag = g.u8("lag_frames")
    check(lag == 0, "%d trames en retard" % lag)

@test
def t18_son():
    """La musique du monde joue sur 4 canaux et le TCHAK declenche son bruitage (carre 2 + bruit)."""
    g = Game(); g.boot_to_room1()
    check(all(g.u8("st_active", c) for c in range(4)), "musique inactive sur un canal")
    p0 = [g.u8("st_lo", c) for c in range(4)]
    g.run(60)
    check([g.u8("st_lo", c) for c in range(4)] != p0, "la musique n'avance pas")
    g.press(B, 2, 8)
    check(g.u8("st_active", 4) and g.u8("st_active", 5), "pas de bruitage TCHAK")
    g.run(240)   # (la balle rebondit : bruitages de chocs pendant un moment)
    check(not g.u8("st_active", 4) and not g.u8("st_active", 5), "le bruitage ne s'arrete pas")

@test
def t19_joueur_qui_ne_comprend_rien():
    """Aucune salle n'est resolue par un joueur qui appuie au hasard (6 parties de 20 s par salle, 8 salles)."""
    import random
    acts = [0, LEFT, RIGHT, UP, DOWN, B, A, LEFT | B, RIGHT | B, UP | A]
    g = Game(); g.boot_to_room1()
    for room in range(8):
        if room:
            g.skip_room(room - 1)
        st = g.nes.save()
        for seed in range(6):
            g.nes.load(st)
            rnd = random.Random(seed * 7 + room)
            for _ in range(200):
                g.run(6, rnd.choice(acts))
                check(g.u8("b_state") != 3 and g.u8("room_idx") == room,
                      "salle %d resolue au hasard (graine %d)" % (room + 1, seed))
        g.nes.load(st)

# ------------------------------------------------------------- nouvelles salles
@test
def t20_salle2_boing():
    """Salle 1-2 : le reflexe envoie la balle dans la gelee ; depuis la droite, balle -> puits ->
    ressort -> cibles -> porte sur la corniche, atteinte en rebondissant soi-meme."""
    g = Game(); g.goto_room(1); st = g.nes.save()
    door = lambda: door_open(g, 5, 4)
    g.press(B, 2, 0)
    check(g.until(lambda: g.obj(0)["flags"] & 4, 300), "la balle n'est pas tombee dans la gelee")
    check(not door(), "porte ouverte par le reflexe")
    g.nes.load(st)
    g.until(lambda: g.bx() >= 146, 100, RIGHT)
    g.run(1, LEFT)
    g.press(B, 2, 0)
    check(g.until(door, 300), "la porte ne s'ouvre pas")
    shot(g, "17_boing")
    g.until(lambda: g.s8("b_vy") < -50, 400, LEFT)
    for k in range(200):
        g.run(1, RIGHT if 7 <= k < 67 else 0)
        if g.u8("b_state") == 3:
            break
    check(g.u8("b_state") == 3, "corniche / sortie non atteinte")
    check(g.u8("socks") == 1, "chaussette de la corniche non prise")

@test
def t21_salle3_tunnel():
    """Salle 1-3 : effraye depuis le toit, le Trouillard fuit dans la gelee ; depuis sa gauche,
    il fonce dans le tunnel (trop bas pour BIDULE) et pousse la caisse sur la plaque."""
    g = Game(); g.goto_room(2); st = g.nes.save()
    door = lambda: door_open(g, 14, 7)
    g.until(lambda: g.bx() <= 130, 200, LEFT)
    g.press(B, 2, 0)
    check(g.until(lambda: g.obj(0)["flags"] & 4, 200), "le Trouillard n'est pas tombe dans la gelee")
    g.nes.load(st)
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 300, LEFT)
    g.until(lambda: g.bx() <= 52, 200, LEFT)
    g.press(B, 2, 0)
    check(g.until(door, 400), "porte fermee")
    shot(g, "18_tunnel")
    g.until(lambda: g.bx() >= 104, 200, RIGHT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 160, 200, UP), "echelle")
    g.run(4, UP)
    check(g.until(lambda: g.u8("b_state") == 3, 300, RIGHT), "sortie non atteinte")

@test
def t22_salle5_poids_plume():
    """Salle 1-5 : BIDULE seul sur la plaque est trop leger (indice) ; BIDULE + balle = 2 ->
    trappe -> la caisse tombe sur la 2e plaque -> porte."""
    g = Game(); g.goto_room(4)
    door = lambda: door_open(g, 14, 9)
    g.until(lambda: g.by() >= 195 and g.u8("b_ground"), 200, LEFT)
    g.run(10)
    check(g.u8("map", 3 * 16 + 3) != 0, "la trappe ne doit pas s'ouvrir avec BIDULE seul")
    shot(g, "19_trop_leger")
    g.until(lambda: g.bx() <= 112, 100, LEFT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() <= 184, 200, UP), "echelle gauche")
    g.run(3, UP)
    g.until(lambda: g.bx() <= 60, 300, LEFT)
    g.run(1, RIGHT)
    g.press(B, 2, 100)
    g.until(lambda: g.by() >= 195 and g.u8("b_ground"), 300, RIGHT)
    check(g.until(door, 200), "porte fermee")
    g.until(lambda: g.bx() >= 158, 100, RIGHT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() <= 184, 200, UP), "echelle droite")
    g.run(3, UP)
    check(g.until(lambda: g.u8("b_state") == 3, 300, RIGHT), "sortie non atteinte")

@test
def t23_salle7_grande_machine():
    """Salle 1-7 : planche (levier) + balle -> ressort -> cibles -> convoyeur -> le Trouillard tombe ;
    l'effrayer depuis sa droite -> plaque -> porte."""
    g = Game(); g.goto_room(6)
    door = lambda: door_open(g, 1, 9)
    g.until(lambda: g.bx() >= 168, 100, RIGHT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 110, 200, UP), "echelle")
    g.run(3, UP)
    g.press(A, 2, 30)
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 200, DOWN)
    g.until(lambda: g.bx() <= 108, 200, LEFT)
    g.run(1, RIGHT)
    g.press(B, 2, 0)
    check(g.until(lambda: g.s8("conv_dir") == 1, 200), "le convoyeur ne demarre pas")
    check(g.until(lambda: g.obj(0)["y"] == 192, 400), "le Trouillard n'est pas descendu")
    shot(g, "20_grande_machine")
    g.until(lambda: g.bx() >= 170, 200, RIGHT)
    g.run(1, LEFT)
    g.press(B, 2, 0)
    check(g.until(door, 300), "porte fermee")
    check(g.until(lambda: g.u8("b_state") == 3, 500, LEFT), "sortie non atteinte")

def boss_to_lever(g):
    """Levier du bas, a droite de la trappe (le Roi vient vers BIDULE en phase 3)."""
    g.until(lambda: g.bx() >= 200, 100, RIGHT)

@test
def t24_boss_roi_du_rebond():
    """Salle 1-8 : le Roi tombe 3 fois dans la trappe (levier au bon moment) -> porte -> fin."""
    g = Game(); g.goto_room(7)
    check(g.u8("boss_room") == 1 and g.u8("st_active", 0), "musique / salle de boss")
    shot(g, "21_boss")
    boss_to_lever(g)
    for phase in range(3):
        st = g.nes.save()
        done = False
        for d in range(0, 720, 6):
            g.nes.load(st)
            g.run(d)
            g.press(A, 2, 0)
            if g.until(lambda: g.u8("o_state", 0) > phase, 120):
                done = True
                break
        check(done, "phase %d : le Roi ne tombe jamais" % (phase + 1))
        if phase < 2:
            check(g.until(lambda: not (g.u8("o_flags", 0) & 4), 200), "le Roi ne revient pas")
    check(g.until(lambda: door_open(g, 14, 9), 100), "la porte ne s'ouvre pas")
    shot(g, "22_boss_vaincu")
    check(g.until(lambda: g.u8("b_state") == 3, 500, RIGHT), "sortie non atteinte")
    check(g.until(lambda: g.u8("game_state") == 8, 400), "pas d'ecran de fin")

@test
def t25_boss_attendre_ne_suffit_pas():
    """Salle 1-8 : sans le levier le Roi ne tombe jamais ; s'il rebondit sur BIDULE, celui-ci est
    aplati (sans perte de vie)."""
    g = Game(); g.goto_room(7)
    squashed = False
    for _ in range(1800):
        g.run(1)
        squashed |= g.u8("b_emote") == 2
    check(g.u8("o_state", 0) == 0, "le Roi est tombe sans levier")
    check(squashed, "le Roi n'a jamais touche BIDULE immobile")
    check(g.u8("lives") == 5, "vie perdue")

# ============================================================== MONDE 2
def w2(g, n):
    g.goto_room(7 + n)

@test
def t30_w2_1_fauteuil():
    """2-1 : depuis la gauche le fauteuil fonce vers BIDULE et tombe dans la gelee ; depuis la droite
    il fonce jusqu'a la porte et s'arrete sur la plaque."""
    g = Game(); w2(g, 1); st = g.nes.save()
    check(g.u8("cur_bank") == 2, "banque du monde 2 non selectionnee")
    shot(g, "30_w2_fauteuil")
    g.until(lambda: g.bx() <= 18, 200, LEFT)
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 200, DOWN)
    g.press(B, 2, 120)
    check(g.obj(0)["flags"] & 4 or g.obj(0)["x"] == 112, "le fauteuil aurait du tomber dans la gelee")
    g.nes.load(st)
    g.until(lambda: g.bx() >= 170, 200, RIGHT)
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 200, DOWN)
    g.press(B, 2, 0)
    check(g.until(lambda: door_open(g, 14, 9), 200), "porte fermee")
    check(g.until(lambda: g.u8("b_state") == 3, 300, RIGHT), "sortie")

@test
def t31_w2_2_lampe():
    """2-2 : la lampe vient la ou BIDULE fait TCHAK : il faut le faire SUR la plaque."""
    g = Game(); w2(g, 2); st = g.nes.save()
    g.press(B, 2, 150)
    check(not door_open(g, 1, 9), "porte ouverte par le reflexe")
    g.nes.load(st)
    g.until(lambda: g.bx() <= 48, 300, LEFT)
    g.press(B, 2, 0)
    check(g.until(lambda: door_open(g, 1, 9), 300), "porte fermee")
    shot(g, "31_w2_lampe")
    check(g.until(lambda: g.u8("b_state") == 3, 300, LEFT), "sortie")

@test
def t32_w2_3_commode():
    """2-3 : un seul TCHAK ne suffit pas ; 3 TCHAK depuis sa gauche -> elle file sur la plaque."""
    g = Game(); w2(g, 3)
    g.until(lambda: g.bx() <= 60, 300, LEFT)
    g.press(B, 2, 60)
    check(g.obj(0)["x"] == 96, "la commode a bouge au premier TCHAK")
    g.press(B, 2, 26)
    g.press(B, 2, 0)
    check(g.until(lambda: door_open(g, 14, 9), 300), "porte fermee")
    shot(g, "32_w2_commode")
    check(g.until(lambda: g.u8("b_state") == 3, 300, RIGHT), "sortie")

@test
def t33_w2_4_coffre():
    """2-4 : doser la poussee du biscuit pour que le coffre le mange SUR la plaque."""
    g = Game(); w2(g, 4)
    g.until(lambda: g.bx() <= 25, 100, LEFT)
    g.run(1, RIGHT)
    g.press(B, 2, 0)
    check(g.until(lambda: door_open(g, 14, 9), 400), "porte fermee")
    shot(g, "33_w2_coffre")

@test
def t34_w2_5_serrures():
    """2-5 : trois serrures, une par humeur (commode, fauteuil, lampe)."""
    g = Game(); w2(g, 5)
    g.until(lambda: g.bx() <= 60, 100, LEFT)
    for _ in range(3):
        g.press(B, 2, 26)
    g.run(150)
    check(g.u8("lock_count") == 1, "serrure de la commode")
    g.until(lambda: g.bx() >= 140, 200, RIGHT)
    g.run(1, LEFT)
    g.press(B, 2, 150)
    check(g.u8("lock_count") == 2, "serrure du fauteuil")
    g.until(lambda: g.bx() <= 34, 300, LEFT)
    g.press(B, 2, 250)
    check(g.u8("lock_count") == 3 and door_open(g, 1, 9), "serrure de la lampe / porte")
    shot(g, "34_w2_serrures")
    check(g.until(lambda: g.u8("b_state") == 3, 300, LEFT), "sortie")

@test
def t35_w2_6_au_choix():
    """2-6 : deux coupables possibles : la commode (3 TCHAK) ou le fauteuil qui la pousse."""
    for mode in ("commode", "fauteuil"):
        g = Game(); w2(g, 6)
        if mode == "commode":
            g.until(lambda: g.bx() <= 120, 200, LEFT)
            for _ in range(3):
                g.press(B, 2, 26)
        else:
            g.until(lambda: g.bx() <= 150, 200, LEFT)
            g.press(B, 2, 0)
        check(g.until(lambda: door_open(g, 1, 9), 300), "porte fermee (%s)" % mode)
    check(g.until(lambda: g.u8("b_state") == 3, 400, LEFT), "sortie")

@test
def t36_w2_7_grand_menage():
    """2-7 : lampe + fauteuil d'un seul TCHAK bien place, planche (levier), puis la commode."""
    g = Game(); w2(g, 7)
    g.until(lambda: g.bx() <= 178, 100, LEFT)
    g.press(B, 2, 220)
    check(g.u8("lock_count") == 2, "lampe + fauteuil")
    g.until(lambda: g.bx() <= 162, 100, LEFT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 110, 200, UP), "echelle")
    g.run(3, UP)
    g.press(A, 2, 30)
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 200, DOWN)
    g.until(lambda: g.bx() <= 140, 100, LEFT)
    for _ in range(3):
        g.press(B, 2, 26)
    check(g.until(lambda: door_open(g, 1, 9), 300), "porte fermee")
    check(g.until(lambda: g.u8("b_state") == 3, 400, LEFT), "sortie")

@test
def t37_w2_8_madame_armoire():
    """2-8 : Madame Armoire ne se bat pas seule (30 s) ; il faut lui LANCER des biscuits pour
    qu'elle les recrache dans les 3 serrures."""
    g = Game(); w2(g, 8); st0 = g.nes.save()
    check(g.u8("boss_room") == 1, "musique de boss")
    shot(g, "35_w2_armoire")
    g.run(1800)
    check(g.u8("lock_count") == 0, "elle s'est vaincue toute seule")
    g.nes.load(st0)
    def approach(j):
        o = g.obj(j); k = g.obj(0)
        tx = max(16, min(208, o["x"] - 20 if k["x"] > o["x"] else o["x"] + 20))
        if g.bx() > tx: g.until(lambda: g.bx() <= tx, 200, LEFT)
        else: g.until(lambda: g.bx() >= tx, 200, RIGHT)
        g.run(1, RIGHT if k["x"] > o["x"] else LEFT)
    for lock in range(1, 4):
        found = False
        for j in (1, 2):
            st = g.nes.save(); approach(j); st2 = g.nes.save()
            for d in range(0, 360, 6):
                g.nes.load(st2); g.run(d); g.press(B, 2, 0)
                if g.until(lambda: g.u8("lock_count") >= lock, 220):
                    found = True; break
            if found: break
            g.nes.load(st)
        check(found, "serrure %d introuvable" % lock)
        g.run(60)
    check(door_open(g, 14, 9), "porte fermee")
    shot(g, "36_w2_armoire_vaincue")
    check(g.until(lambda: g.u8("b_state") == 3, 500, RIGHT), "sortie")
    check(g.until(lambda: g.u8("game_state") == 8, 400), "pas d'ecran de fin du monde 2")
    g.run(90)
    shot(g, "37_w2_fin")

@test
def t38_w2_joueur_au_hasard():
    """Monde 2 : un joueur qui appuie au hasard (4 parties de 20 s par salle) ne resout aucune salle."""
    import random
    acts = [0, LEFT, RIGHT, UP, DOWN, B, A, LEFT | B, RIGHT | B, UP | A]
    g = Game(); g.goto_room(8)
    for room in range(8, 16):
        if room > 8:
            g.skip_room(room - 1)
        st = g.nes.save()
        for seed in range(4):
            g.nes.load(st)
            rnd = random.Random(seed * 13 + room)
            for _ in range(200):
                g.run(6, rnd.choice(acts))
                check(g.u8("b_state") != 3 and g.u8("room_idx") == room,
                      "salle 2-%d resolue au hasard (graine %d)" % (room - 7, seed))
        g.nes.load(st)

# ============================================================== MONDE 3
def w3(g, n):
    g.goto_room(15 + n)

@test
def t40_w3_1_piston():
    """3-1 : reflexe -> la balle part dans la gelee ; du bon cote : balle -> cible -> piston -> caisse -> plaque."""
    g = Game(); w3(g, 1); st = g.nes.save()
    check(g.u8("cur_bank") == 3, "banque du monde 3 non selectionnee")
    shot(g, "40_w3_piston")
    g.press(B, 2, 200)
    check(not door_open(g, 1, 4), "porte ouverte par le reflexe")
    g.nes.load(st)
    g.until(lambda: g.bx() >= 112, 100, RIGHT); g.run(1, LEFT)
    g.press(B, 2, 0)
    check(g.until(lambda: door_open(g, 1, 4), 300), "porte fermee")
    g.until(lambda: g.bx() <= 64, 300, LEFT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 120, 200, UP), "echelle")
    g.run(4, UP)
    check(g.until(lambda: g.u8("b_state") == 3, 300, LEFT), "sortie")

@test
def t41_w3_2_copieur():
    """3-2 : marcher droit fait tomber le Copieur de son etage ; TCHAK (miroir) d'abord -> il va sur la plaque."""
    g = Game(); w3(g, 2); st = g.nes.save()
    g.until(lambda: g.u8("b_state") == 3, 400, RIGHT)
    check(not door_open(g, 14, 9), "porte ouverte sans miroir")
    g.nes.load(st)
    g.press(B, 2, 30)
    check(g.obj(0)["state"] == 1, "pas de mode miroir")
    shot(g, "41_w3_copieur")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie")

@test
def t42_w3_3_machinal():
    """3-3 : attendre au bord du trou fait tomber BIDULE ; figer le Machinal sur la plaque -> passerelle."""
    g = Game(); w3(g, 3); st = g.nes.save()
    bridge = lambda: g.u8("map", 11 * 16 + 3) == 17
    g.until(lambda: g.bx() <= 97, 200, LEFT)
    for _ in range(1200):
        g.run(1, LEFT if bridge() else 0)
        if g.u8("b_state") in (3, 4):
            break
    check(g.u8("b_state") != 3, "on a pu passer sans figer le robot")
    g.nes.load(st)
    g.until(lambda: g.bx() >= 120, 100, RIGHT)
    g.until(lambda: 160 <= g.obj(0)["x"] <= 172 and g.s8("o_vx", 0) > 0, 900)
    g.press(B, 2, 20)
    check(g.obj(0)["state"] == 1 and bridge(), "robot non fige sur la plaque")
    shot(g, "42_w3_machinal")
    check(g.until(lambda: g.u8("b_state") == 3, 250, LEFT), "sortie")

@test
def t43_w3_4_chaine():
    """3-4 : la balle sur la cible inverse le convoyeur, la caisse tombe devant le piston, levier -> plaque."""
    g = Game(); w3(g, 4)
    g.until(lambda: g.bx() >= 74, 100, RIGHT); g.run(1, LEFT)
    g.press(B, 2, 0)
    check(g.until(lambda: g.s8("conv_dir") == 1, 120), "convoyeur non inverse")
    check(g.until(lambda: g.obj(1)["y"] == 192, 600), "la caisse n'est pas descendue")
    g.until(lambda: g.bx() >= 110, 100, RIGHT)
    g.press(A, 2, 100)
    shot(g, "43_w3_chaine")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie")

@test
def t44_w3_5_copie_conforme():
    """3-5 : BIDULE enferme : aller-retour + miroir a chaque demi-tour (cliquet) ; pousser contre le mur ne sert a rien."""
    g = Game(); w3(g, 5); st = g.nes.save()
    g.run(150, RIGHT)
    check(g.obj(0)["x"] < 100, "pousser contre le mur fait avancer le Copieur")
    g.nes.load(st)
    cage = lambda: g.u8("map", 9 * 16 + 10) != 17
    for _ in range(8):
        g.until(lambda: g.bx() >= 140, 80, RIGHT); g.press(B, 2, 26)
        g.until(lambda: g.bx() <= 98, 80, LEFT); g.press(B, 2, 26)
        if cage():
            break
    check(cage(), "la cage ne s'ouvre pas")
    check(g.until(lambda: g.u8("b_state") == 3, 300, RIGHT), "sortie")

@test
def t45_w3_6_deux_facons():
    """3-6 : deux solutions : le piston (levier en haut de l'echelle) ou pousser la caisse soi-meme."""
    g = Game(); w3(g, 6); st = g.nes.save()
    g.until(lambda: g.bx() <= 170, 100, LEFT)
    g.until(lambda: g.u8("b_state") == 0 and g.by() < 110, 200, UP); g.run(3, UP)
    g.press(A, 2, 120)
    check(door_open(g, 1, 9), "solution piston")
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 200, DOWN)
    check(g.until(lambda: g.u8("b_state") == 3, 600, LEFT), "sortie")
    g.nes.load(st)
    for _ in range(5):
        cx = g.obj(0)["x"]
        if g.bx() < cx + 10: g.until(lambda: g.bx() >= cx + 12, 200, RIGHT)
        else: g.until(lambda: g.bx() <= cx + 16, 200, LEFT)
        g.run(1, LEFT); g.press(B, 2, 40)
        if door_open(g, 1, 9):
            break
    check(door_open(g, 1, 9), "solution TCHAK")

@test
def t46_w3_7_grande_chaine():
    """3-7 : trois serrures (cible, piston/caisse, Copieur) ; chaque TCHAK fait aussi basculer le Copieur."""
    g = Game(); w3(g, 7)
    g.press(B, 2, 30)
    g.until(lambda: g.bx() <= 150, 100, LEFT)
    g.press(B, 2, 80)
    g.until(lambda: g.bx() <= 112, 100, LEFT)
    g.press(A, 2, 100)
    check(g.u8("lock_count") >= 2, "serrures cible + caisse")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie")
    check(g.u8("lock_count") == 3, "serrure du Copieur")

@test
def t47_w3_8_grand_derange():
    """3-8 : le Grand Derange ne tombe jamais seul ; l'etourdir pres du trou quand le convoyeur y va."""
    g = Game(); w3(g, 8); st0 = g.nes.save()
    check(g.u8("boss_room") == 1, "musique de boss")
    shot(g, "44_w3_derange")
    for _ in range(1200):
        g.run(1)
        check(g.obj(0)["state"] == 0, "il est tombe tout seul")
    g.nes.load(st0)
    cool = 0
    for _ in range(9000):
        c = g.s8("conv_dir"); k = g.obj(0); bx = g.bx()
        btn = 0
        if not (k["flags"] & 4) and c != 0:
            t = max(40, min(184, k["x"] - 40 * c))
            btn = RIGHT if bx < t - 4 else (LEFT if bx > t + 4 else 0)
            near = (k["x"] >= 176) if c > 0 else (k["x"] <= 48)
            if near and abs(k["x"] - bx) < 56 and cool == 0:
                btn = B; cool = 40
        else:
            btn = RIGHT if bx < 110 else (LEFT if bx > 130 else 0)
        if cool: cool -= 1
        g.run(1, btn)
        if g.u8("o_aux", 0):
            break
    check(g.u8("o_aux", 0) == 1, "boss non vaincu")
    check(g.until(lambda: door_open(g, 1, 4), 100), "porte fermee")
    shot(g, "45_w3_derange_vaincu")

@test
def t48_w3_joueur_au_hasard():
    """Monde 3 : un joueur qui appuie au hasard (4 parties de 20 s par salle) ne resout aucune salle."""
    import random
    acts = [0, LEFT, RIGHT, UP, DOWN, B, A, LEFT | B, RIGHT | B, UP | A]
    g = Game(); g.goto_room(16)
    for room in range(16, 24):
        if room > 16:
            g.skip_room(room - 1)
        st = g.nes.save()
        for seed in range(4):
            g.nes.load(st)
            rnd = random.Random(seed * 17 + room)
            for _ in range(200):
                g.run(6, rnd.choice(acts))
                check(g.u8("b_state") != 3 and g.u8("room_idx") == room,
                      "salle 3-%d resolue au hasard (graine %d)" % (room - 15, seed))
        g.nes.load(st)

# ============================================================== MONDE 4
def w4(g, n):
    g.goto_room(23 + n)

def lever_at(g, x):
    if g.bx() > x: g.until(lambda: g.bx() <= x, 200, LEFT)
    else: g.until(lambda: g.bx() >= x, 200, RIGHT)
    g.press(A, 2, 0)

def push_to(g, j, lo, hi, n=12):
    """Pousse l'objet j a coups de TCHAK jusqu'a ce que son x soit dans [lo, hi].
    Loin du but : TCHAK de pres (fort) ; pres du but : de plus loin (doux)."""
    for _ in range(n):
        cx = g.obj(j)["x"]
        if lo <= cx <= hi:
            return True
        gap = (cx - hi) if cx > hi else (lo - cx)
        d = 18 if gap > 40 else 34
        if cx > hi:
            if g.bx() < cx + 8: g.until(lambda: g.bx() >= cx + d, 200, RIGHT)
            else: g.until(lambda: g.bx() <= cx + d, 200, LEFT)
            g.run(1, LEFT)
        else:
            if g.bx() > cx - 8: g.until(lambda: g.bx() <= cx - d, 200, LEFT)
            else: g.until(lambda: g.bx() >= cx - d, 200, RIGHT)
            g.run(1, RIGHT)
        g.press(B, 2, 60)
    return lo <= g.obj(j)["x"] <= hi

@test
def t50_w4_1_tete_en_bas():
    """4-1 : inverser tout de suite : la caisse tombe au plafond, a cote de la plaque ; la placer
    d'abord sous la plaque du plafond."""
    g = Game(); w4(g, 1); st = g.nes.save()
    check(g.u8("cur_bank") == 4, "banque du monde 4 non selectionnee")
    lever_at(g, 34); g.run(90)
    check(g.u8("grav_up") == 1 and g.obj(0)["y"] == 48, "la caisse n'est pas au plafond")
    check(not door_open(g, 14, 9), "porte ouverte par le reflexe")
    g.nes.load(st)
    check(push_to(g, 0, 152, 167), "caisse non placee")
    lever_at(g, 34)
    check(g.until(lambda: door_open(g, 14, 9), 200), "porte fermee")
    shot(g, "50_w4_tete_en_bas")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie")

@test
def t51_w4_2_ballon():
    """4-2 : le ballon tombe vers le haut ; inverser, le pousser au sol sous la cheminee, revenir a la normale."""
    g = Game(); w4(g, 2)
    lever_at(g, 34); g.run(90)
    check(push_to(g, 0, 177, 191), "ballon non place")
    lever_at(g, 34)
    check(g.until(lambda: door_open(g, 14, 9), 200), "porte fermee")
    shot(g, "51_w4_ballon")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie")

@test
def t52_w4_3_aimant():
    """4-3 : l'aimant seul bloque la caisse contre l'obstacle ; inverser, puis revenir -> plaque."""
    g = Game(); w4(g, 3)
    lever_at(g, 66); g.run(150)
    check(not door_open(g, 1, 9), "l'aimant seul ne doit pas suffire")
    lever_at(g, 82); g.run(150)
    g.press(A, 2, 60)
    check(g.until(lambda: door_open(g, 1, 9), 100), "porte fermee")
    check(g.until(lambda: g.u8("b_state") == 3, 300, LEFT), "sortie")

@test
def t53_w4_4_trois_serrures():
    """4-4 : caisse sur la plaque du sol, puis sous la plaque du plafond, inverser, ballon sous la cheminee, revenir."""
    g = Game(); w4(g, 4)
    push_to(g, 0, 121, 135); g.run(20)
    check(g.u8("lock_count") >= 1, "serrure du sol")
    check(push_to(g, 0, 41, 71), "caisse sous la plaque du plafond")
    lever_at(g, 34); g.run(80)
    check(g.u8("lock_count") >= 2, "serrure du plafond")
    check(push_to(g, 1, 177, 191), "ballon")
    lever_at(g, 34)
    check(g.until(lambda: door_open(g, 14, 9), 200), "porte fermee")
    shot(g, "52_w4_serrures")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie")

@test
def t54_w4_5_zigzag():
    """4-5 : aimant + deux retournements pour contourner les colonnes du sol puis du plafond."""
    g = Game(); w4(g, 5)
    g.until(lambda: g.bx() >= 56, 100, RIGHT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 110, 200, UP), "echelle")
    g.run(3, UP); g.press(A, 2, 150)
    g.until(lambda: g.u8("b_ground") and g.by() > 180, 200, DOWN)
    lever_at(g, 34); g.run(150)
    check(g.obj(1)["x"] < 120 and g.obj(1)["y"] == 48, "la caisse doit buter contre la colonne du plafond")
    lever_at(g, 34)
    check(g.until(lambda: door_open(g, 1, 9), 250), "porte fermee")
    shot(g, "53_w4_zigzag")
    check(g.until(lambda: g.u8("b_state") == 3, 300, LEFT), "sortie")

@test
def t55_w4_6_par_dessus():
    """4-6 : l'aimant seul fait tomber la caisse dans la cuve ; par le plafond, elle passe."""
    g = Game(); w4(g, 6); st = g.nes.save()
    lever_at(g, 34)
    check(g.until(lambda: g.obj(1)["flags"] & 4, 250), "la caisse aurait du tomber dans la cuve")
    g.nes.load(st)
    lever_at(g, 50); g.run(60); lever_at(g, 34); g.run(200); lever_at(g, 50)
    check(g.until(lambda: door_open(g, 1, 9), 150), "porte fermee")
    check(g.until(lambda: g.u8("b_state") == 3, 300, LEFT), "sortie")

@test
def t56_w4_7_grand_retournement():
    """4-7 : zigzag, ballon a glisser sous la cheminee entre deux retournements, deux plaques."""
    g = Game(); w4(g, 7)
    lever_at(g, 34); g.run(120)
    lever_at(g, 50); g.run(150)
    for _ in range(5):
        bx = g.obj(2)["x"]
        if 31 <= bx <= 49: break
        g.until(lambda: g.bx() >= bx + 12, 100, RIGHT); g.run(1, LEFT); g.press(B, 2, 50)
    lever_at(g, 50); g.run(150)
    check(g.u8("lock_count") == 2, "serrures ballon + plaque du sol")
    lever_at(g, 50)
    check(g.until(lambda: door_open(g, 1, 9), 200), "porte fermee")
    shot(g, "54_w4_retournement")
    check(g.until(lambda: g.u8("b_state") == 3, 300, LEFT), "sortie")

@test
def t57_w4_8_roi_a_l_envers():
    """4-8 : le Roi a l'Envers ne tombe pas seul ; retourner la gravite quand il passe sous le trou
    du plafond (ou au-dessus du trou du sol)."""
    g = Game(); w4(g, 8); st0 = g.nes.save()
    check(g.u8("boss_room") == 1, "musique de boss")
    shot(g, "55_w4_envers")
    for _ in range(1500):
        g.run(1)
    check(g.obj(0)["state"] == 0, "il est tombe tout seul")
    g.nes.load(st0)
    cool = 0
    for _ in range(12000):
        k = g.obj(0); gu = g.u8("grav_up"); btn = 0
        if not (k["flags"] & 4):
            if g.bx() > 32: btn = LEFT
            elif gu == 0 and 150 <= k["x"] <= 162 and k["flags"] & 1 and cool == 0: btn = A; cool = 40
            elif gu == 1 and 86 <= k["x"] <= 96 and k["flags"] & 1 and cool == 0: btn = A; cool = 40
        if cool: cool -= 1
        g.run(1, btn)
        if g.u8("o_aux", 0):
            break
    check(g.u8("o_aux", 0) == 1, "boss non vaincu")
    check(g.until(lambda: door_open(g, 1, 9), 100), "porte fermee")
    shot(g, "56_w4_envers_vaincu")

@test
def t58_w4_joueur_au_hasard():
    """Monde 4 : un joueur qui appuie au hasard (4 parties de 20 s par salle) ne resout aucune salle."""
    import random
    acts = [0, LEFT, RIGHT, UP, DOWN, B, A, LEFT | B, RIGHT | B, UP | A]
    g = Game(); g.goto_room(24)
    for room in range(24, 32):
        if room > 24:
            g.skip_room(room - 1)
        st = g.nes.save()
        for seed in range(4):
            g.nes.load(st)
            rnd = random.Random(seed * 19 + room)
            for _ in range(200):
                g.run(6, rnd.choice(acts))
                check(g.u8("b_state") != 3 and g.u8("room_idx") == room,
                      "salle 4-%d resolue au hasard (graine %d)" % (room - 23, seed))
        g.nes.load(st)

# =================================================================== main
def main():
    ok = 0
    t0 = time.time()
    report = []
    for fn in RESULTS:
        name = fn.__name__
        try:
            fn()
            ok += 1
            status = "OK"
            print("[ OK ] %s - %s" % (name, fn.__doc__.strip()))
        except Exception as e:
            status = "ECHEC : %s" % e
            print("[ECHEC] %s - %s\n        %s" % (name, fn.__doc__.strip(), e))
            if "-v" in sys.argv:
                traceback.print_exc()
        report.append((name, fn.__doc__.strip(), status))
    print("\n%d/%d tests reussis en %.1f s" % (ok, len(RESULTS), time.time() - t0))
    with open(os.path.join(ROOT, "build", "test_report.txt"), "w") as f:
        for r in report:
            f.write("%s | %s | %s\n" % r)
    return 0 if ok == len(RESULTS) else 1

if __name__ == "__main__":
    sys.exit(main())
