# =====================================================================
#  TCHAK ! - LA GRANDE CATASTROPHE : construction de la ROM NES
#  Prerequis : cc65 (ca65, cc65, ld65) et Python 3 (+ Pillow pour les apercus)
# =====================================================================
CC65    = cc65
CA65    = ca65
LD65    = ld65
PYTHON  = python3
CFLAGS  = -t none --cpu 6502 -Oirs -Cl -I src
BUILD   = build
ROM     = $(BUILD)/tchak.nes

CSRC    = main video room objects enemies bidule tchak events progress menus gen/gfx_data gen/rooms
ASRC    = crt0 sound fast gen/music

COBJ    = $(addprefix $(BUILD)/,$(addsuffix .o,$(subst /,_,$(CSRC))))
AOBJ    = $(addprefix $(BUILD)/,$(addsuffix .o,$(subst /,_,$(ASRC))))

GEN     = src/gen/gfx.h src/gen/gfx_data.c src/gen/rooms.c src/gen/rooms.h \
          src/gen/music.s src/gen/music.h $(BUILD)/tchak.chr

all: $(ROM)

src/gen/gfx.h src/gen/gfx_data.c $(BUILD)/tchak.chr $(BUILD)/metatiles.txt: tools/gfx.py tools/art_bidule.py
	mkdir -p $(BUILD) src/gen
	$(PYTHON) tools/gfx.py

src/gen/rooms.c src/gen/rooms.h: tools/rooms.py rooms/*.txt $(BUILD)/metatiles.txt
	$(PYTHON) tools/rooms.py

src/gen/music.s src/gen/music.h: tools/music.py
	$(PYTHON) tools/music.py

# compilation C : src/x.c -> build/x.s -> build/x.o
$(BUILD)/gen_%.s: src/gen/%.c $(GEN) src/*.h
	$(CC65) $(CFLAGS) $< -o $@
$(BUILD)/%.s: src/%.c $(GEN) src/*.h
	$(CC65) $(CFLAGS) $< -o $@
$(BUILD)/%.o: $(BUILD)/%.s
	$(CA65) $< -o $@

$(BUILD)/gen_music.o: src/gen/music.s
	$(CA65) $< -o $@
$(BUILD)/crt0.o: src/crt0.s $(BUILD)/tchak.chr
	$(CA65) $< -o $@
$(BUILD)/sound.o: src/sound.s
	$(CA65) $< -o $@
$(BUILD)/fast.o: src/fast.s
	$(CA65) $< -o $@

$(ROM): $(AOBJ) $(COBJ) cfg/nrom.cfg
	$(LD65) -C cfg/nrom.cfg -o $@ -m $(BUILD)/tchak.map -Ln $(BUILD)/tchak.labels $(AOBJ) $(COBJ) none.lib
	@echo "ROM : $@ ($$(stat -c %s $@) octets)"

dist: $(ROM)
	cp $(ROM) dist/tchak_prototype.nes

test: $(ROM)
	$(PYTHON) tests/test_rom.py

clean:
	rm -f build/*.o build/*.s build/tchak.nes build/tchak.map build/tchak.labels

.PHONY: all dist test clean
.SECONDARY:
