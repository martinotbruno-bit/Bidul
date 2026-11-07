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

@test
def t02_salle1_premier_tchak():
    """Salle 1-1 : TCHAK sur la balle -> cible -> porte -> sortie."""
    g = Game(); g.boot_to_room1()
    check(not door_open(g, 1, 9), "porte deja ouverte")
    g.press(B, 2, 0)
    g.run(10)
    shot(g, "06_tchak")
    check(g.obj(0)["vx"] > 0, "la balle n'est pas partie vers la droite")
    check(g.until(lambda: door_open(g, 1, 9), 300), "la porte ne s'ouvre pas")
    check(g.until(lambda: g.u8("b_state") == 3, 400, LEFT), "BIDULE n'atteint pas la sortie")
    g.wait_room(1)

@test
def t03_salle1_trop_loin():
    """Salle 1-1 : un TCHAK hors de portee ne fait rien (regle lisible)."""
    g = Game(); g.boot_to_room1()
    g.run(40, LEFT)                     # s'eloigner de la balle
    g.press(B, 2, 30)
    check(g.obj(0)["vx"] == 0 and g.obj(0)["x"] == 96, "la balle a bouge alors qu'elle est hors de portee")

@test
def t04_salle2_reaction_en_chaine():
    """Salle 1-2 : balle -> ressort -> cible -> convoyeur -> caisse -> plaque -> porte."""
    g = Game(); g.boot_to_room1(); g.skip_room(0)
    g.press(B, 2, 0)
    check(g.until(lambda: g.s8("conv_dir") == 1, 200), "le convoyeur ne demarre pas")
    shot(g, "07_chaine_convoyeur")
    check(g.until(lambda: door_open(g, 14, 9), 400), "la porte ne s'ouvre pas")
    check(g.obj(1)["x"] // 16 + 0 in (12, 13) and g.obj(1)["y"] == 192, "la caisse n'est pas sur la plaque")
    shot(g, "08_chaine_porte")
    check(g.until(lambda: g.u8("b_state") == 3, 300, RIGHT), "BIDULE n'atteint pas la sortie")

@test
def t05_salle2_solution_echelle():
    """Salle 1-2, solution alternative : monter a l'echelle et pousser la caisse a coups de TCHAK."""
    g = Game(); g.boot_to_room1(); g.skip_room(0)
    g.until(lambda: g.bx() <= 98, 200, LEFT)
    check(g.until(lambda: g.u8("b_state") == 0 and g.by() < 110, 200, UP), "echelle impossible a grimper")
    for _ in range(8):
        cx = g.obj(1)["x"]
        if g.obj(1)["y"] > 140:
            break
        g.until(lambda: g.bx() >= cx - 18, 120, RIGHT)
        g.run(1, RIGHT)
        g.press(B, 2, 40)
    check(g.until(lambda: door_open(g, 14, 9), 200), "la caisse n'a pas ouvert la porte")
    check(g.s8("conv_dir") == 0, "le convoyeur n'aurait pas du demarrer")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie non atteinte")

@test
def t06_salle2_chaussette_du_puits():
    """Salle 1-2, secret : sauter dans le puits a ressorts rapporte une chaussette."""
    g = Game(); g.boot_to_room1(); g.skip_room(0)
    check(g.until(lambda: g.u8("socks") == 1, 600, LEFT), "chaussette non ramassee")
    shot(g, "09_chaussette")
    # BIDULE doit pouvoir ressortir du puits (pas de blocage permanent)
    out = False
    for _ in range(600):
        g.run(1, RIGHT)
        if g.u8("b_ground") and g.bx() > 60:
            out = True
            break
    check(out, "BIDULE reste coince dans le puits")

@test
def t07_salle3_solution_trouillard():
    """Salle 1-3, solution 1 : effrayer le Trouillard depuis la plate-forme."""
    g = Game(); g.boot_to_room1(); g.skip_room(0); g.skip_room(1)
    check(climb_left_ladder(g, 18), "echelle")
    g.until(lambda: g.bx() >= 120, 200, RIGHT)
    g.press(B, 2, 0)
    check(g.until(lambda: g.obj(1)["state"] == 2, 60), "le Trouillard ne s'enfuit pas")
    shot(g, "10_trouillard_fuit")
    check(g.until(lambda: door_open(g, 14, 9), 300), "porte fermee")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie non atteinte")

@test
def t08_salle3_solution_levier():
    """Salle 1-3, solution 2 : le levier demarre le convoyeur, la caisse tombe sur la plaque."""
    g = Game(); g.boot_to_room1(); g.skip_room(0); g.skip_room(1)
    climb_left_ladder(g, 18)
    g.until(lambda: g.bx() >= 88, 200, RIGHT)
    g.press(A, 2, 2)
    check(g.until(lambda: g.s8("conv_dir") == 1, 30), "le levier ne demarre pas le convoyeur")
    check(g.until(lambda: door_open(g, 14, 9), 300), "porte fermee")
    check(g.obj(1)["state"] == 0, "le Trouillard aurait du rester endormi")
    shot(g, "11_levier")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie non atteinte")

@test
def t09_salle3_solution_ressort():
    """Salle 1-3, solution 3 : ressort -> atterrir sur la caisse -> TCHAK -> la caisse tombe."""
    g = Game(); g.boot_to_room1(); g.skip_room(0); g.skip_room(1)
    climb_left_ladder(g, 18)
    g.until(lambda: g.bx() >= 140, 300, RIGHT)
    check(g.until(lambda: g.s8("b_vy") < -50, 120), "pas de rebond sur le ressort")
    g.run(26)
    for k in range(150):
        g.run(1, RIGHT if k < 40 else 0)
        if g.u8("b_ground") and k > 3:
            break
    check(g.by() < 120, "BIDULE n'est pas sur l'etagere")
    g.press(B, 2, 0)
    check(g.until(lambda: door_open(g, 14, 9), 200), "porte fermee")
    check(g.u8("socks") == 1, "la chaussette au-dessus du ressort aurait du etre prise")
    check(g.until(lambda: g.u8("b_state") == 3, 400, RIGHT), "sortie non atteinte")

@test
def t10_gelee_perd_une_vie():
    """Tomber dans la gelee coute une vie et recommence la salle."""
    g = Game(); g.boot_to_room1(); g.skip_room(0); g.skip_room(1)
    check(g.until(lambda: g.u8("b_state") == 4, 200, RIGHT), "BIDULE ne tombe pas dans la cuve")
    g.run(20)
    shot(g, "12_gelee")
    g.wait_room(2)
    check(g.u8("lives") == 4, "vie non retiree")
    check(g.bx() == 48, "BIDULE n'est pas revenu au depart")

@test
def t11_game_over():
    """Plus de vies : ecran de fin de partie, puis retour au titre."""
    g = Game(); g.boot_to_room1(); g.skip_room(0); g.skip_room(1)
    g.poke("lives", 1)
    g.until(lambda: g.u8("b_state") == 4, 200, RIGHT)
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
    check(g.obj(0)["x"] == 96 and g.u8("lives") == 5, "la salle n'a pas ete reinitialisee")

@test
def t13_mot_de_passe():
    """Le mot de passe affiche permet de reprendre a la salle 1-3."""
    g = Game()
    g.run(500)
    g.press(START, 2, 20)
    g.press(DOWN, 2, 4)
    g.press(START, 2, 40)
    check(g.u8("game_state") == 2, "pas d'ecran de mot de passe")
    code = password(2, 1)
    for slot, ch in enumerate(code):
        for _ in range(ALPHA.index(ch)):
            g.press(UP, 2, 2)
        if slot < 3:
            g.press(RIGHT, 2, 2)
    shot(g, "15_mot_de_passe")
    g.press(START, 2, 30)
    g.wait_room(2)
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
        g = Game(); g.boot_to_room1(); g.skip_room(0)
        g.press(B, 2, 0)
        tr = []
        for f in range(240):
            g.run(1, RIGHT if 100 < f < 140 else 0)
            tr.append(gameplay_state(g))
        traces.append(tr)
    check(traces[0] == traces[1], "divergence entre deux executions identiques")

@test
def t16_fin_du_prototype():
    """Apres la salle 1-3, l'ecran de fin s'affiche."""
    g = Game(); g.boot_to_room1(); g.skip_room(0); g.skip_room(1)
    g.poke("room_result", 1)
    check(g.until(lambda: g.u8("game_state") == 8, 300), "pas d'ecran de fin")
    g.run(90)
    shot(g, "16_fin")

@test
def t17_performances():
    """Aucune trame perdue (la logique tient dans 1/60 s) pendant une reaction en chaine."""
    g = Game(); g.boot_to_room1(); g.skip_room(0)
    g.poke("lag_frames", 0)
    g.press(B, 2, 0)
    g.run(400, 0)
    g.run(200, RIGHT)
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
