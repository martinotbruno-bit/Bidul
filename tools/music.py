#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Compilateur de musique de TCHAK ! -> src/gen/music.s (format du moteur src/sound.s).

Notation par canal (jetons separes par des espaces, '|' ignore) :
  C5.2      note Do5 pendant 2 unites (unite = 'tempo' trames)  ; dieses : C#5
  r.2       silence
  @d2v12    instrument : rapport cyclique 0-3, volume 0-15 (bruit : @v12)
  e3        decroissance de volume : -1 toutes les 3 trames (e0 = aucune)
  ~-12      glissement de periode par trame (negatif = monte)
  n5.2      bruit : periode 5 (N5 = mode "metallique")
  k.2 s.2 h.1   batterie (grosse caisse, caisse claire, charleston)
  [         point de boucle ; la fin du canal reboucle ici (sinon : fin)
Toutes les musiques sont des compositions originales.
"""
import os

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
CPU = 1789773.0
NAMES = {"C": 0, "C#": 1, "D": 2, "D#": 3, "E": 4, "F": 5, "F#": 6, "G": 7, "G#": 8, "A": 9, "A#": 10, "B": 11}

def note_index(tok):
    name, octv = tok[:-1], int(tok[-1])
    return (octv - 2) * 12 + NAMES[name]       # C2 = 0

def periods():
    lo, hi = [], []
    for i in range(96):
        midi = 36 + i                           # C2 = midi 36
        f = 440.0 * 2 ** ((midi - 69) / 12.0)
        p = int(round(CPU / (16 * f) - 1))
        p = max(8, min(p, 0x7FF))
        lo.append(p & 0xFF); hi.append(p >> 8)
    return lo, hi

def compile_channel(text, tempo, kind):
    """kind : 'pulse', 'tri', 'noise'. Retourne (liste d'elements, a_boucle)."""
    out, loop_at = [], None
    for tok in text.replace("|", " ").split():
        if tok == "[":
            loop_at = len(out); out.append(("label",)); continue
        if tok.startswith("@"):
            if kind == "noise":
                v = int(tok[2:]); out.append(0xF0); out.append(v & 0x0F)
            else:
                d = int(tok[2]); v = int(tok[4:]); out.append(0xF0); out.append((d << 6) | v)
            continue
        if tok.startswith("e") and tok[1:].isdigit():
            out += [0xF3, int(tok[1:])]; continue
        if tok.startswith("~"):
            out += [0xF4, int(tok[1:]) & 0xFF]; continue
        what, units = tok.split(".")
        dur = int(round(float(units) * tempo))
        assert 1 <= dur <= 255, tok
        if what == "r":
            out += [0x60, dur]; continue
        if kind == "noise":
            drums = {"k": (15, 12), "s": (13, 6), "h": (5, 1)}
            if what in drums:
                v, p = drums[what]
                out += [0xF0, v, 0xF3, 1, p, dur]; continue
            p = int(what[1:]) | (0x10 if what[0] == "N" else 0)
            out += [p, dur]; continue
        n = note_index(what)
        if kind == "tri":
            n += 12                             # le triangle sonne une octave plus bas
        assert 0 <= n < 0x60, tok
        out += [n, dur]
    return out, loop_at

TEMPO_TITLE = 7
SONGS = [
 ("TITLE", 7, {
  "p1": "@d2v10 e6 [ G5.2 r.1 G5.1 E5.2 C5.2 | D5.2 E5.2 F5.2 D5.2 | E5.2 r.1 E5.1 C5.2 G4.2 | A4.2 B4.2 C5.4 |"
        " G5.2 r.1 G5.1 A5.2 G5.2 | F5.2 E5.2 D5.2 C5.2 | D5.2 E5.1 D5.1 C5.2 B4.2 | C5.6 r.2",
  "p2": "@d1v6 e3 [ r.2 E4.1 G4.1 r.2 E4.1 G4.1 | r.2 D4.1 G4.1 r.2 D4.1 F4.1 | r.2 C4.1 E4.1 r.2 C4.1 G4.1 |"
        " r.2 C4.1 F4.1 r.2 D4.1 G4.1 | r.2 E4.1 A4.1 r.2 E4.1 C5.1 | r.2 C4.1 F4.1 r.2 C4.1 A4.1 |"
        " r.2 D4.1 G4.1 r.2 B3.1 F4.1 | r.2 E4.1 G4.1 r.2 r.2",
  "tri": "[ C3.2 C4.2 G2.2 G3.2 | G2.2 G3.2 G2.2 B2.2 | C3.2 C4.2 E2.2 E3.2 | F2.2 F3.2 G2.2 G3.2 |"
         " C3.2 C4.2 A2.2 A3.2 | F2.2 F3.2 F2.2 A2.2 | G2.2 G3.2 G2.2 G3.2 | C3.2 G2.2 C3.4",
  "noise": "[ k.2 h.2 s.2 h.2",
 }),
 ("WORLD1", 8, {
  "p1": "@d1v10 e5 [ F4.1 A4.1 C5.2 A4.1 C5.1 F5.2 | E5.2 C5.2 G4.4 | D5.1 E5.1 F5.2 E5.1 D5.1 C5.2 | A4.2 G4.2 F4.4 |"
        " F4.1 A4.1 C5.2 A4.1 C5.1 F5.2 | G5.2 F5.1 E5.1 D5.2 C5.2 | A#4.2 D5.2 C5.2 A4.2 | F4.6 r.2",
  "p2": "@d2v5 e2 [ r.1 A4.1 r.1 A4.1 r.1 A4.1 r.1 A4.1 | r.1 G4.1 r.1 G4.1 r.1 E4.1 r.1 E4.1 |"
        " r.1 F4.1 r.1 F4.1 r.1 D4.1 r.1 D4.1 | r.1 E4.1 r.1 E4.1 r.1 A4.1 r.1 A4.1 |"
        " r.1 A4.1 r.1 A4.1 r.1 A4.1 r.1 A4.1 | r.1 D4.1 r.1 D4.1 r.1 G4.1 r.1 G4.1 |"
        " r.1 F4.1 r.1 F4.1 r.1 E4.1 r.1 G4.1 | r.1 A4.1 r.1 C5.1 r.1 A4.1 r.2",
  "tri": "[ F2.2 C3.2 F2.2 C3.2 | C2.2 G2.2 C2.2 G2.2 | D2.2 A2.2 A#1.2 F2.2 | C2.2 G2.2 F2.2 C2.2 |"
         " F2.2 C3.2 F2.2 C3.2 | A#1.2 F2.2 C2.2 G2.2 | A#1.2 F2.2 C2.2 E2.2 | F2.2 C2.2 F2.4",
  "noise": "[ k.2 h.1 h.1 s.2 h.1 k.1",
 }),
 ("VICTORY", 6, {
  "p1": "@d2v12 e0 C5.1 E5.1 G5.1 C6.3 G5.1 C6.5 r.2",
  "p2": "@d1v7 e0 E4.1 G4.1 C5.1 E5.3 D5.1 E5.5 r.2",
  "tri": "C3.3 G2.3 C3.6 r.2",
  "noise": "s.1 s.1 s.1 k.3 s.1 k.5",
 }),
 ("DEFEAT", 9, {
  "p1": "@d2v11 e0 G4.2 F#4.2 F4.2 @d2v11 e2 E4.6",
  "tri": "C2.2 B1.2 A#1.2 A1.6",
  "noise": "r.6 k.6",
 }),
 ("BOSS", 6, {
  "p1": "@d1v10 e3 [ A4.2 A4.1 C5.1 A4.2 G4.2 | A4.2 A4.1 C5.1 D5.2 C5.2 | A4.2 A4.1 C5.1 A4.2 G4.2 | E4.2 F4.2 G4.2 G#4.2 |"
        " A5.1 E5.1 C5.1 A4.1 A5.1 E5.1 C5.1 A4.1 | G5.1 D5.1 B4.1 G4.1 G5.1 D5.1 B4.1 G4.1 |"
        " F5.1 C5.1 A4.1 F4.1 F5.1 C5.1 A4.1 F4.1 | E5.2 G#4.2 B4.2 E5.2",
  "p2": "@d2v6 e1 [ A3.1 r.1 A3.1 r.1 A3.1 r.1 A3.1 r.1 | A3.1 r.1 A3.1 r.1 F3.1 r.1 F3.1 r.1 |"
        " A3.1 r.1 A3.1 r.1 A3.1 r.1 A3.1 r.1 | E3.1 r.1 E3.1 r.1 E3.1 r.1 E3.1 r.1 |"
        " A3.1 r.1 A3.1 r.1 A3.1 r.1 A3.1 r.1 | G3.1 r.1 G3.1 r.1 G3.1 r.1 G3.1 r.1 |"
        " F3.1 r.1 F3.1 r.1 F3.1 r.1 F3.1 r.1 | E3.1 r.1 E3.1 r.1 G#3.1 r.1 B3.1 r.1",
  "tri": "[ A1.2 A2.2 A1.2 A2.2 | F1.2 F2.2 F1.2 F2.2 | A1.2 A2.2 A1.2 A2.2 | E1.2 E2.2 E1.2 E2.2 |"
         " A1.2 A2.2 A1.2 A2.2 | G1.2 G2.2 G1.2 G2.2 | F1.2 F2.2 F1.2 F2.2 | E1.2 E2.2 G#1.2 B1.2",
  "noise": "[ k.1 h.1 s.1 h.1 k.1 k.1 s.1 h.1",
 }),
 ("ENDING", 9, {
  "p1": "@d2v9 e8 [ C5.4 E5.4 G5.4 E5.4 | F5.4 A5.4 G5.8 | E5.4 C5.4 D5.4 B4.4 | C5.12 r.4",
  "p2": "@d1v5 e6 [ E4.4 G4.4 C5.4 G4.4 | A4.4 C5.4 B4.8 | G4.4 E4.4 F4.4 D4.4 | E4.12 r.4",
  "tri": "[ C2.8 G2.8 | F2.8 C3.8 | A2.8 G2.8 | C2.16",
  "noise": "[ k.4 h.4 s.4 h.4",
 }),
 ("WORLD2", 8, {
  "p1": "@d2v9 e6 [ D5.3 F#5.1 A5.2 | G5.3 E5.1 C#5.2 | D5.2 A4.2 F#4.2 | G4.4 A4.2 |"
        " B4.3 D5.1 G5.2 | F#5.3 D5.1 A4.2 | E5.2 G5.2 C#5.2 | D5.4 r.2",
  "p2": "@d1v5 e3 [ r.2 F#4.2 F#4.2 | r.2 E4.2 E4.2 | r.2 D4.2 D4.2 | r.2 C#4.2 C#4.2 |"
        " r.2 D4.2 D4.2 | r.2 D4.2 D4.2 | r.2 C#4.2 E4.2 | r.2 F#4.2 r.2",
  "tri": "[ D2.2 r.4 | A1.2 r.4 | D2.2 r.4 | A1.2 r.4 | G1.2 r.4 | D2.2 r.4 | A1.2 r.4 | D2.2 r.4",
  "noise": "[ k.2 h.2 h.2",
 }),
 ("WORLD3", 6, {
  "p1": "@d1v9 e4 [ E4.2 E4.1 G4.1 E4.2 A4.2 | G4.2 E4.1 D4.1 E4.4 | E4.2 E4.1 G4.1 B4.2 A4.2 | G4.1 A4.1 G4.1 E4.1 D4.4 |"
        " C5.2 B4.1 A4.1 G4.2 E4.2 | A4.2 G4.1 E4.1 D4.4 | E4.1 G4.1 A4.1 B4.1 D5.2 B4.2 | E5.4 r.4",
  "p2": "@d2v4 e1 [ E3.1 E3.1 r.1 E3.1 E3.1 r.1 E3.1 r.1 | G3.1 G3.1 r.1 G3.1 A3.1 r.1 A3.1 r.1 |"
        " E3.1 E3.1 r.1 E3.1 E3.1 r.1 E3.1 r.1 | D3.1 D3.1 r.1 D3.1 D3.1 r.1 B2.1 r.1 |"
        " C3.1 C3.1 r.1 C3.1 C3.1 r.1 C3.1 r.1 | A2.1 A2.1 r.1 A2.1 B2.1 r.1 B2.1 r.1 |"
        " C3.1 C3.1 r.1 D3.1 D3.1 r.1 D3.1 r.1 | E3.1 E3.1 r.1 E3.1 r.4",
  "tri": "[ E1.2 E2.2 E1.2 E2.2 | G1.2 G2.2 A1.2 A2.2 | E1.2 E2.2 E1.2 E2.2 | D1.2 D2.2 B1.2 D2.2 |"
         " C2.2 C3.2 C2.2 C3.2 | A1.2 A2.2 B1.2 B2.2 | C2.2 C3.2 D2.2 D3.2 | E1.2 E2.2 E1.4",
  "noise": "[ k.1 N3.1 s.1 N3.1 k.1 k.1 s.1 N3.1",
 }),
 ("WORLD4", 7, {
  "p1": "@d2v9 e5 [ C5.2 B4.2 A#4.2 B4.2 | C5.1 E5.1 G5.2 F#5.4 | G5.2 F#5.2 F5.2 F#5.2 | G5.1 E5.1 C5.2 B4.4 |"
        " A4.2 C5.2 E5.2 D#5.2 | D5.2 C5.2 B4.4 | C5.1 D5.1 E5.1 F#5.1 G5.2 D#5.2 | E5.6 r.2",
  "p2": "@d1v5 e2 [ r.1 E4.1 r.1 E4.1 r.1 D#4.1 r.1 D#4.1 | r.1 E4.1 r.1 G4.1 r.1 A#4.1 r.1 A4.1 |"
        " r.1 B4.1 r.1 B4.1 r.1 A#4.1 r.1 A#4.1 | r.1 B4.1 r.1 G4.1 r.1 F#4.1 r.1 F#4.1 |"
        " r.1 C4.1 r.1 E4.1 r.1 G4.1 r.1 F#4.1 | r.1 F4.1 r.1 E4.1 r.1 D#4.1 r.1 D#4.1 |"
        " r.1 E4.1 r.1 G4.1 r.1 B4.1 r.1 A4.1 | r.1 G4.1 r.1 E4.1 r.2 r.2",
  "tri": "[ C2.4 F#1.4 | C2.4 D2.4 | E2.4 A#1.4 | E2.4 B1.4 | A1.4 E2.4 | D2.4 G1.4 | C2.4 B1.4 | C2.4 C2.4",
  "noise": "[ k.2 N2.1 h.1 s.1 h.1 N2.2",
 }),
 ("WORLD5", 6, {
  "p1": "@d2v10 e4 [ G5.2 r.1 G5.1 E5.2 C5.2 | D5.1 F5.1 A5.2 F#5.4 | E5.2 r.1 E5.1 C5.2 A4.2 | B4.1 D5.1 F5.2 E5.4 |"
        " C6.2 B5.1 A5.1 G5.2 F5.2 | E5.1 D5.1 C5.2 D5.4 | G4.1 A4.1 B4.1 C5.1 D5.1 E5.1 F5.1 F#5.1 | G5.6 r.2",
  "p2": "@d1v6 e2 [ r.2 E4.1 G4.1 r.2 E4.1 G4.1 | r.2 F4.1 A4.1 r.2 D4.1 A4.1 | r.2 C4.1 E4.1 r.2 C4.1 E4.1 |"
        " r.2 D4.1 G4.1 r.2 C4.1 G4.1 | r.2 E4.1 A4.1 r.2 F4.1 A4.1 | r.2 C4.1 G4.1 r.2 B3.1 F4.1 |"
        " r.2 D4.1 G4.1 r.2 D4.1 A4.1 | r.2 B3.1 D4.1 r.2 r.2",
  "tri": "[ C2.2 C3.2 G1.2 G2.2 | D2.2 D3.2 D2.2 A1.2 | A1.2 A2.2 E2.2 E1.2 | G1.2 G2.2 C2.2 C3.2 |"
         " F1.2 F2.2 F1.2 A1.2 | C2.2 G1.2 G2.2 B1.2 | G1.2 A1.2 B1.2 D2.2 | G1.2 G2.2 G1.4",
  "noise": "[ k.1 h.1 s.1 N2.1 k.1 k.1 s.1 h.1",
 }),
]

SFX = [
 ("TCHAK",    {"p2": "r.3 @d1v15 e1 ~10 A5.14", "noise": "@v13 n2.3 @v15 e1 n12.14"}),
 ("BOING",    {"p2": "@d2v12 e2 ~-10 C4.18"}),
 ("BUMP",     {"noise": "@v10 e1 n9.6"}),
 ("CLICK",    {"p2": "@d1v10 C6.2 G6.4"}),
 ("DOOR",     {"p2": "@d2v10 C5.4 E5.4 G5.4 C6.8", "noise": "@v5 e2 n14.16"}),
 ("SURPRISE", {"p2": "@d0v12 ~-24 C5.6 ~-40 G5.8"}),
 ("SPLAT",    {"p2": "@d2v12 e2 ~24 C4.14", "noise": "@v15 e1 n13.16"}),
 ("SOCK",     {"p2": "@d1v10 E6.3 G6.3 C7.8"}),
 ("MENU",     {"p2": "@d1v8 C6.3"}),
 ("SELECT",   {"p2": "@d1v10 C6.3 E6.3 G6.6"}),
 ("SQUASH",   {"p2": "@d2v12 e1 ~18 G5.10", "noise": "@v8 e1 n6.5"}),
 ("PANIC",    {"p2": "@d0v9 C6.2 A5.2 C6.2 A5.2 C6.2 A5.2 C6.2 A5.2"}),
 ("LEVER",    {"p2": "@d1v9 G4.2 C5.4", "noise": "@v6 e1 n4.3"}),
]

def emit_stream(lines, label, text, tempo, kind):
    data, loop_at = compile_channel(text, tempo, kind)
    lines.append("%s:" % label)
    cur = []
    def flush():
        if cur:
            lines.append("        .byte " + ",".join("$%02X" % b for b in cur)); cur.clear()
    for el in data:
        if isinstance(el, tuple):
            flush(); lines.append("%s_loop:" % label); continue
        cur.append(el)
    flush()
    if loop_at is not None:
        lines.append("        .byte $F1")
        lines.append("        .word %s_loop" % label)
    else:
        lines.append("        .byte $F2")

WORLD_SONGS = {1: "WORLD1", 2: "WORLD2", 3: "WORLD3", 4: "WORLD4", 5: "WORLD5"}

def main():
    lo, hi = periods()
    kinds = {"p1": "pulse", "p2": "pulse", "tri": "tri", "noise": "noise"}
    world_names = set(WORLD_SONGS.values())
    common = [sg for sg in SONGS if sg[0] not in world_names]
    L = ["; GENERE PAR tools/music.py - NE PAS MODIFIER A LA MAIN (moteur commun)",
         "        .export period_lo, period_hi, song_table, sfx_table",
         '.segment "RODATA"',
         "period_lo: .byte " + ",".join("$%02X" % v for v in lo),
         "period_hi: .byte " + ",".join("$%02X" % v for v in hi),
         ""]
    L.append("song_table:")
    for name, tempo, chans in common:
        L.append("        .word " + ",".join(("mus_%s_%s" % (name, c)) if c in chans else "0"
                                           for c in ("p1", "p2", "tri", "noise")))
    L.append("sfx_table:")
    for name, chans in SFX:
        L.append("        .word " + ",".join(("sfx_%s_%s" % (name, c)) if c in chans else "0"
                                           for c in ("p2", "noise")))
    for name, tempo, chans in common:
        for c, text in chans.items():
            emit_stream(L, "mus_%s_%s" % (name, c), text, tempo, kinds[c])
    for name, chans in SFX:
        for c, text in chans.items():
            emit_stream(L, "sfx_%s_%s" % (name, c), text, 1, kinds[c])
    with open(os.path.join(ROOT, "src", "gen", "music.s"), "w") as f:
        f.write("\n".join(L) + "\n")
    # theme de chaque monde : dans la banque du monde (pointeurs en en-tete)
    for w, sname in WORLD_SONGS.items():
        name, tempo, chans = next(sg for sg in SONGS if sg[0] == sname)
        L = ["; GENERE PAR tools/music.py : theme du monde %d (banque du monde)" % w,
             "        .export _world_song", '.segment "BANKHDR"', "_world_song:",
             "        .word " + ",".join(("mus_%s_%s" % (name, c)) if c in chans else "0"
                                        for c in ("p1", "p2", "tri", "noise")),
             '.segment "BANKDATA"']
        for c, text in chans.items():
            emit_stream(L, "mus_%s_%s" % (name, c), text, tempo, kinds[c])
        with open(os.path.join(ROOT, "src", "gen", "music_w%d.s" % w), "w") as f:
            f.write("\n".join(L) + "\n")
    h = ["/* GENERE PAR tools/music.py */", "#ifndef MUSIC_H", "#define MUSIC_H"]
    for i, (name, _, _) in enumerate(common):
        h.append("#define MUS_%s %d" % (name, i))
    h.append("#define MUS_WORLD 255   /* theme du monde en place */")
    for i, (name, _) in enumerate(SFX):
        h.append("#define SFX_%s %d" % (name, i))
    h.append("#endif")
    with open(os.path.join(ROOT, "src", "gen", "music.h"), "w") as f:
        f.write("\n".join(h) + "\n")

if __name__ == "__main__":
    main()
