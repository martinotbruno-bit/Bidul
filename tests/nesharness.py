# -*- coding: utf-8 -*-
"""Petit banc d'essai autour de l'emulateur headless 'cynes'.

Lit les adresses des variables dans build/tchak.labels (genere par ld65)
pour pouvoir inspecter l'etat du jeu depuis les tests.
"""
import os
import cynes
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
ROM = os.path.join(ROOT, "build", "tchak.nes")

A, B, SELECT, START = cynes.NES_INPUT_A, cynes.NES_INPUT_B, cynes.NES_INPUT_SELECT, cynes.NES_INPUT_START
UP, DOWN, LEFT, RIGHT = cynes.NES_INPUT_UP, cynes.NES_INPUT_DOWN, cynes.NES_INPUT_LEFT, cynes.NES_INPUT_RIGHT


def load_labels():
    labels = {}
    with open(os.path.join(ROOT, "build", "tchak.labels")) as f:
        for line in f:
            p = line.split()
            if len(p) == 3 and p[0] == "al":
                labels[p[2].lstrip(".")] = int(p[1], 16)
    return labels


class Game:
    def __init__(self, rom=ROM):
        self.nes = cynes.NES(rom)
        self.L = load_labels()
        self.frame = None
        self.frames = 0

    # ---------------------------------------------------------- memoire
    def u8(self, name, off=0):
        return self.nes[self.L["_" + name] + off]

    def s8(self, name, off=0):
        v = self.u8(name, off)
        return v - 256 if v >= 128 else v

    def u16(self, name, off=0):
        a = self.L["_" + name] + off * 2
        return self.nes[a] | (self.nes[a + 1] << 8)

    def poke(self, name, value, off=0):
        self.nes[self.L["_" + name] + off] = value

    # ---------------------------------------------------------- bidule
    def bx(self):
        return self.u16("b_x") >> 4

    def by(self):
        return self.u16("b_y") >> 4

    def obj(self, i):
        return dict(type=self.u8("o_type", i), x=self.u16("o_x", i) >> 4, y=self.u16("o_y", i) >> 4,
                    vx=self.s8("o_vx", i), vy=self.s8("o_vy", i), flags=self.u8("o_flags", i),
                    state=self.u8("o_state", i))

    # ---------------------------------------------------------- entrees
    def run(self, n=1, buttons=0):
        self.nes.controller = buttons
        for _ in range(n):
            self.frame = self.nes.step(1)
            self.frames += 1
            if self.nes.has_crashed:
                raise RuntimeError("le CPU a plante (opcode illegal)")
        self.nes.controller = 0
        return self.frame

    def press(self, buttons, hold=2, after=2):
        self.run(hold, buttons)
        self.run(after, 0)

    def until(self, cond, limit=2000, buttons=0):
        for _ in range(limit):
            if cond():
                return True
            self.run(1, buttons)
        return cond()

    def screenshot(self, path, scale=2):
        img = Image.fromarray(self.frame)
        if scale != 1:
            img = img.resize((img.width * scale, img.height * scale), Image.NEAREST)
        img.save(path)

    # ---------------------------------------------------------- navigation
    def wait_room(self, idx, limit=1500):
        """Attend que la salle idx soit chargee ET que sa boucle de jeu tourne."""
        ok = self.until(lambda: self.u8("game_state") == 4 and self.u8("room_idx") == idx
                        and self.u8("room_timer") == 0 and self.u8("room_result") == 0, limit)
        ok = ok and self.until(lambda: self.u8("room_timer") >= 2, 60)
        if not ok:
            raise AssertionError("la salle %d n'a pas demarre" % idx)

    def boot_to_room1(self):
        self.run(500)
        self.press(START, 2, 20)
        self.press(START, 2, 10)
        self.wait_room(0)

    def skip_room(self, idx):
        self.poke("room_result", 1)
        self.wait_room(idx + 1)
