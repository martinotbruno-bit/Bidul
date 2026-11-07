#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Assemble la ROM MMC1 de TCHAK ! a partir des banques liees separement.

Chaque banque de 32 Ko = moteur commun ($8000-$DFFF) + partie propre ($E000-).
Le script lie chaque banque avec ld65, VERIFIE que le moteur commun et les
vecteurs sont identiques octet pour octet dans toutes les banques (sinon un
changement de banque ferait sauter le processeur au milieu de n'importe quoi),
puis ecrit l'en-tete iNES (mapper 1) suivi des banques PRG et CHR.

Usage : make_rom.py SORTIE --shared a.o b.o ... --bank nom:x.o,y.o ... --chr f.chr ...
"""
import os, subprocess, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD = os.path.join(ROOT, "build")
SHARED_END = 0xE000 - 0x8000

def main():
    args = sys.argv[1:]
    out = args.pop(0)
    shared, banks, chrs, mode = [], [], [], None
    for a in args:
        if a in ("--shared", "--bank", "--chr"):
            mode = a
            continue
        if mode == "--shared":
            shared.append(a)
        elif mode == "--bank":
            name, objs = a.split(":")
            banks.append((name, objs.split(",")))
        else:
            chrs.append(a)

    images = []
    for n, (name, objs) in enumerate(banks):
        binf = os.path.join(BUILD, "bank%d_%s.bin" % (n, name))
        cmd = ["ld65", "-C", os.path.join(ROOT, "cfg", "mmc1.cfg"), "-o", binf,
               "-m", binf.replace(".bin", ".map"), "-Ln", binf.replace(".bin", ".labels")]
        cmd += shared + objs + ["none.lib"]
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0 or "Warning" in r.stderr:
            sys.stderr.write(r.stderr)
            if r.returncode != 0:
                sys.exit(1)
        data = open(binf, "rb").read()
        assert len(data) == 0x8000, (binf, len(data))
        images.append((name, data))

    ref_name, ref = images[0]
    for name, data in images[1:]:
        if data[:SHARED_END] != ref[:SHARED_END] or data[-6:] != ref[-6:]:
            diff = next(i for i in range(SHARED_END) if data[i] != ref[i]) if data[:SHARED_END] != ref[:SHARED_END] else -1
            sys.exit("ERREUR : le moteur commun differe entre les banques '%s' et '%s' (adresse $%04X)."
                     " Une banque utilise sans doute une routine de la bibliotheque cc65 que le moteur"
                     " n'utilise pas : l'ajouter a src/libforce.s." % (ref_name, name, 0x8000 + diff))

    # remplissage a une puissance de 2 (4 ou 8 banques)
    nb = 4 if len(images) <= 4 else 8
    prg = b"".join(d for _, d in images) + images[0][1] * (nb - len(images))
    chr_data = b"".join(open(c, "rb").read() for c in chrs)
    nchr = len(chr_data) // 8192
    while nchr & (nchr - 1):
        chr_data += chrs and open(chrs[0], "rb").read()
        nchr += 1
    header = bytes([0x4E, 0x45, 0x53, 0x1A, nb * 2, nchr, 0x10 | 0x01, 0x00]) + bytes(8)
    with open(out, "wb") as f:
        f.write(header + prg + chr_data)
    # etiquettes communes (pour les tests et les debogueurs) : celles de la banque 1
    lab = os.path.join(BUILD, "bank%d_%s.labels" % (min(1, len(images) - 1), images[min(1, len(images) - 1)][0]))
    with open(os.path.join(BUILD, "tchak.labels"), "w") as f:
        f.write(open(lab).read())
    free = [(name, 0x1FFA - (len(d[SHARED_END:-6].rstrip(b"\x00")))) for name, d in images]
    used_shared = len(ref[:SHARED_END].rstrip(b"\x00"))
    print("ROM %s : %d banques PRG de 32 Ko, %d banques CHR ; moteur commun %d/%d octets ; "
          "place libre par banque : %s" % (out, nb, nchr, used_shared, SHARED_END,
                                           ", ".join("%s %d" % f for f in free)))

if __name__ == "__main__":
    main()
