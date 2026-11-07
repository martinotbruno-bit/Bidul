#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Compilateur de salles : rooms/*.txt -> src/gen/rooms.c

Verifie la coherence (dimensions, caracteres, cibles des actions) et
produit des tables C compactes (208 octets de carte par salle).
"""
import os, sys, glob

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
W, H = 16, 13

OBJ_TYPES = {"BALL": 1, "CRATE": 2, "TROU": 3, "SOCK": 4, "KING": 5, "ARMCHAIR": 6,
             "LAMP": 7, "DRESSER": 8, "CHEST": 9, "COOKIE": 10, "WARDROBE": 11}
TRIG_KINDS = {"TARGET": 1, "PLATE": 2, "LEVER": 3, "BOSS": 4}
ACTIONS = {"NONE": 0, "DOOR_OPEN": 1, "DOOR_CLOSE": 2, "CONV_RIGHT": 3, "CONV_LEFT": 4,
           "CONV_STOP": 5, "CONV_REV": 6, "HATCH_OPEN": 7, "HATCH_CLOSE": 8, "WAKE": 9, "REVEAL": 10, "HATCH_PULSE": 11, "LOCK": 12}
ACT_ARGS = {"DOOR_OPEN": 2, "DOOR_CLOSE": 2, "HATCH_OPEN": 3, "HATCH_CLOSE": 3, "HATCH_PULSE": 3, "LOCK": 2, "WAKE": 1, "REVEAL": 1}

def load_metatiles(world):
    table = {}
    with open(os.path.join(ROOT, "build", "metatiles_w%d.txt" % world)) as f:
        for line in f:
            i, name, ch = line.split()
            if ch != "-":
                table[ch] = (int(i), name)
    return table

GLOBAL = {}

def parse(path, mts):
    rooms, cur, in_map = [], None, False
    for ln, raw in enumerate(open(path, encoding="utf-8"), 1):
        line = raw.rstrip("\n")
        if in_map:
            if len(cur["map"]) < H:
                assert len(line) == W, "%s:%d ligne de carte de %d car." % (path, ln, len(line))
                cur["map"].append(line)
                if len(cur["map"]) == H:
                    in_map = False
                continue
        s = line.strip()
        if not s or s.startswith("#") and cur is None:
            continue
        tok = s.split()
        if cur is None and tok[0] == "PALETTE":
            GLOBAL["palette"] = [int(v, 16) for v in tok[1:]]
            continue
        if cur is None and tok[0] == "MUSIC":
            GLOBAL["music"] = int(tok[1])
            continue
        if tok[0] == "ROOM":
            name = s[s.index('"') + 1:s.rindex('"')]
            cur = {"name": name, "map": [], "objs": [], "trigs": [], "conv": 0, "bidule": None}
        elif tok[0] == "WORLD":
            cur["world"], cur["num"] = int(tok[1]), int(tok[3])
        elif tok[0] == "CONV":
            cur["conv"] = int(tok[1])
        elif tok[0] == "MAP":
            in_map = True
        elif tok[0] == "BIDULE":
            cur["bidule"] = (int(tok[1]), int(tok[2]))
        elif tok[0] == "OBJ":
            fl = 2 if "HIDDEN" in tok else 0
            cur["objs"].append((OBJ_TYPES[tok[1]], int(tok[2]), int(tok[3]), fl))
        elif tok[0] == "TRIG":
            kind = TRIG_KINDS[tok[1]]
            x, y, w, h = map(int, tok[2:6])
            on, off, i = [], [], 6
            while i < len(tok):
                which = tok[i]; act = tok[i + 1]; i += 2
                n = ACT_ARGS.get(act, 0)
                args = [int(v) & 0xFF for v in tok[i:i + n]]; i += n
                if act.startswith("HATCH"):          # x y largeur -> y | (largeur-1) << 4
                    args = [args[0], args[1] | ((args[2] - 1) << 4)]
                args += [0] * (2 - len(args))
                (on if which == "ON" else off).append([ACTIONS[act]] + args)
            assert len(on) <= 2 and len(off) <= 1, "%s:%d trop d'actions" % (path, ln)
            while len(on) < 2: on.append([0, 0, 0])
            if not off: off.append([0, 0, 0])
            cur["trigs"].append([kind, x, y, w, h] + on[0] + on[1] + off[0])
        elif tok[0] == "END":
            check(cur, mts, path)
            rooms.append(cur); cur = None
        else:
            raise SystemExit("%s:%d mot-cle inconnu %s" % (path, ln, tok[0]))
    return rooms

def check(r, mts, path):
    assert len(r["map"]) == H, r["name"]
    for y, row in enumerate(r["map"]):
        for x, ch in enumerate(row):
            assert ch in mts, "%s : caractere '%s' inconnu (%d,%d)" % (r["name"], ch, x, y)
    assert r["bidule"], r["name"]
    names = {mts[ch][1] for row in r["map"] for ch in row}
    for t in r["trigs"]:
        for a, x, y in (t[5:8], t[8:11], t[11:14]):
            if a in (1, 2):
                assert r["map"][y][x] in "DE", "%s : DOOR (%d,%d) n'est pas une porte" % (r["name"], x, y)
            if a in (7, 8, 11):
                y &= 15
                assert r["map"][y][x] in "h~", "%s : HATCH (%d,%d) n'est pas une trappe" % (r["name"], x, y)
    assert len(r["objs"]) <= 8, r["name"]
    assert len(r["trigs"]) <= 8, r["name"]
    # chaque salle doit avoir une sortie
    assert any(ch in "DE" for row in r["map"] for ch in row), "%s sans sortie" % r["name"]

def main():
    files = sorted(glob.glob(os.path.join(ROOT, "rooms", "world*.txt")))
    total = 0
    for path in files:
        world = int(os.path.basename(path)[5:-4])
        GLOBAL.clear()
        mts = load_metatiles(world)
        rooms = parse(path, mts)
        assert 1 <= len(rooms) <= 8, path
        pal = GLOBAL.get("palette")
        assert pal and len(pal) == 32, "%s : PALETTE (32 couleurs) manquante" % path
        c = ["/* GENERE PAR tools/rooms.py : salles du monde %d - NE PAS MODIFIER */" % world,
             '#include "../defs.h"', '#include "gfx.h"', '#include "rooms.h"',
             '#pragma rodata-name(push, "BANKDATA")', ""]
        for n, r in enumerate(rooms):
            cells = [mts[ch][0] for row in r["map"] for ch in row]
            c.append("static const u8 map%d[MAP_SIZE] = {" % n)
            for y in range(H):
                c.append("  " + ",".join("%2d" % v for v in cells[y * W:(y + 1) * W]) + ",")
            c.append("};")
            objs = []
            for t, x, y, fl in r["objs"]:
                objs += [t, x * 16, 32 + y * 16, fl]
            c.append("static const u8 obj%d[] = {%s};" % (n, ",".join(map(str, objs)) or "0"))
            tr = [v for t in r["trigs"] for v in t]
            c.append("static const u8 trig%d[] = {%s};" % (n, ",".join(map(str, tr)) or "0"))
            c.append('static const char name%d[] = "%s";' % (n, r["name"]))
        c.append('#pragma rodata-name(pop)')
        c.append('#pragma rodata-name(push, "BANKHDR")')
        c.append("const Room rooms[8] = {")
        for k in range(8):
            n = k if k < len(rooms) else 0
            r = rooms[n]
            bx, by = r["bidule"]
            c.append("  { map%d, name%d, %d, %d, %d, obj%d, %d, trig%d, %d, %d, %d }," % (
                n, n, bx * 16, 32 + by * 16 + 16 - 24, len(r["objs"]), n, len(r["trigs"]), n,
                r["conv"], r["world"], r["num"]))
        c.append("};")
        c.append("const u8 pal_world[32] = {" + ",".join("0x%02X" % v for v in pal) + "};")
        c.append("const u8 world_music = %d;" % GLOBAL.get("music", 1))
        c.append("const u8 world_rooms = %d;" % len(rooms))
        c.append('#pragma rodata-name(pop)')
        with open(os.path.join(ROOT, "src", "gen", "rooms_w%d.c" % world), "w") as f:
            f.write("\n".join(c) + "\n")
        total += 1
        print("monde %d : %d salles compilees" % (world, len(rooms)))
    with open(os.path.join(ROOT, "src", "gen", "rooms.h"), "w") as f:
        f.write("/* GENERE PAR tools/rooms.py */\n#ifndef ROOMS_H\n#define ROOMS_H\n"
                "#define WORLDS_DONE %d\n#define TOTAL_ROOMS (WORLDS_DONE * 8)\n"
                "/* en-tete de la banque du monde courant */\n"
                "extern const Room rooms[8];\nextern const u8 pal_world[32];\n"
                "extern const u8 world_music;\nextern const u8 world_rooms;\n#endif\n" % total)

if __name__ == "__main__":
    main()
