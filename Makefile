# =====================================================================
#  TCHAK ! - LA GRANDE CATASTROPHE : construction de la ROM NES (MMC1)
#  Prerequis : cc65 (ca65, cc65, ld65) et Python 3 (+ Pillow pour les apercus)
# =====================================================================
CC65    = cc65
CA65    = ca65
PYTHON  = python3
CFLAGS  = -t none --cpu 6502 -Ors -Cl -I src
BFLAGS  = $(CFLAGS) --code-name BANKCODE --rodata-name BANKDATA --bss-name BANKBSS --data-name BANKDATA
BUILD   = build
ROM     = $(BUILD)/tchak.nes

# moteur commun (identique dans toutes les banques)
SHARED_C = main video room objects enemies bidule tchak events progress screens gen/gfx_common
SHARED_S = crt0 sound fast libforce gen/music
SHARED   = $(addprefix $(BUILD)/,$(addsuffix .o,$(subst /,_,$(SHARED_S) $(SHARED_C))))

# banques : 0 = menus, 1 = monde 1, ...
BANK_MENU = $(BUILD)/bankhdr_menu.o $(BUILD)/gen_mt_w1.o $(BUILD)/gen_rooms_w1.o $(BUILD)/menus_bank.o
BANK_W1   = $(BUILD)/bankhdr_w1.o $(BUILD)/gen_mt_w1.o $(BUILD)/gen_rooms_w1.o
BANK_W2   = $(BUILD)/bankhdr_w2.o $(BUILD)/gen_mt_w2.o $(BUILD)/gen_rooms_w2.o $(BUILD)/world2_bank.o
CHRS      = $(BUILD)/chr_w1.chr $(BUILD)/chr_w2.chr

GEN = src/gen/gfx.h src/gen/gfx_common.c src/gen/mt_w1.c src/gen/mt_w2.c src/gen/rooms_w1.c \
      src/gen/rooms_w2.c src/gen/rooms.h \
      src/gen/music.s src/gen/music.h

all: $(ROM)

src/gen/gfx.h src/gen/gfx_common.c src/gen/mt_w1.c src/gen/mt_w2.c $(CHRS) $(BUILD)/metatiles_w1.txt $(BUILD)/metatiles_w2.txt: tools/gfx.py tools/art_bidule.py
	mkdir -p $(BUILD) src/gen
	$(PYTHON) tools/gfx.py

src/gen/rooms_w1.c src/gen/rooms_w2.c src/gen/rooms.h: tools/rooms.py rooms/*.txt $(BUILD)/metatiles_w1.txt $(BUILD)/metatiles_w2.txt
	$(PYTHON) tools/rooms.py

src/gen/music.s src/gen/music.h: tools/music.py
	$(PYTHON) tools/music.py

# C commun : src/x.c -> build/x.s -> build/x.o
$(BUILD)/gen_%.s: src/gen/%.c $(GEN) src/*.h
	$(CC65) $(CFLAGS) $< -o $@
$(BUILD)/%.s: src/%.c $(GEN) src/*.h
	$(CC65) $(CFLAGS) $< -o $@
$(BUILD)/%.o: $(BUILD)/%.s
	$(CA65) $< -o $@

# C de banque (segments BANKCODE / BANKDATA / BANKBSS)
$(BUILD)/menus_bank.s: src/menus.c $(GEN) src/*.h
	$(CC65) $(BFLAGS) $< -o $@
$(BUILD)/world2_bank.s: src/world2.c $(GEN) src/*.h
	$(CC65) $(BFLAGS) $< -o $@

# assembleur
$(BUILD)/gen_music.o: src/gen/music.s
	$(CA65) $< -o $@
$(BUILD)/%.o: src/%.s
	$(CA65) $< -o $@

$(ROM): $(SHARED) $(BANK_MENU) $(BANK_W1) $(BANK_W2) $(CHRS) cfg/mmc1.cfg tools/make_rom.py
	$(PYTHON) tools/make_rom.py $@ --shared $(SHARED) \
	  --bank menu:$(subst $(space),$(comma),$(BANK_MENU)) \
	  --bank w1:$(subst $(space),$(comma),$(BANK_W1)) \
	  --bank w2:$(subst $(space),$(comma),$(BANK_W2)) \
	  --chr $(CHRS)

comma := ,
empty :=
space := $(empty) $(empty)

dist: $(ROM)
	cp $(ROM) dist/tchak.nes

test: $(ROM)
	$(PYTHON) tests/test_rom.py

clean:
	rm -f build/*.o build/*.s build/*.bin build/*.map build/*.labels build/tchak.nes

.PHONY: all dist test clean
.SECONDARY:
