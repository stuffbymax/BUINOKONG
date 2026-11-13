BUINOKONG - arcade climber for the original MAKERbuino / Gamebuino Classic
=========================================================================
Files on the SD card: BUINOKON.HEX + BUINOKON.INF   (in-game name: BUINOKONG)

Why "BUINOKON" and not "BUINOKONG" on the card? The loader uses 8.3 file
names (max 8 characters; the stock games are truncated the same way, e.g.
SUPERCRA.HEX, ASTEROCK.HEX). The title screen and the loader's info page
show the full name BUINOKONG (it is stored inside the .INF).

STATUS - READ FIRST
-------------------
* NOT tested on physical hardware. Compiled with avr-gcc 7.3.0 against the
  real Gamebuino Classic 0.5.2 library and RUN in the simavr AVR emulator
  (menus, gameplay, LCD output decoded from SPI). Evidence: test_evidence/.
* INF format verified against the loader's own source and the official
  INVADERS.INF; I did not run the loader itself (needs SD hardware).

INSTALL
-------
1. SD card (FAT16/32) that already has the MAKERbuino loader (LOADER.HEX in
   the root). Do not touch it.
2. Copy SD_CARD/BUINOKON.HEX and SD_CARD/BUINOKON.INF to the ROOT of the card.
3. Boot, choose BUINOKON in the loader list, press A. The loader flashes the
   game (a few seconds); the ATmega328P can't run a HEX from SD by itself.
4. Back to the loader: hold C then press A (or press C on the title screen).
   BUINOKON.SAV (high score) is written by the loader when you leave this way.

CONTROLS
--------
 Left/Right  walk        Up/Down  climb ladders (stand on one)
 A           jump        B        HOLD to swing the hammer (once you carry one)
 C           pause (C resume, B quit)       Hold C+A  back to loader
Goal: climb the sloping girders to LUNA at the top before the BONUS box
counts down to 0 (-100 per second; at 0 Mario dies).
 * Barrels: jump them (+100; two in one jump +300; three +800).
 * Hammers lie on two girders: walk over one to pick it up, then HOLD B to
   swing it. Swinging smashes barrels (+300) and fireballs (+500) and makes
   you safe, but you can't jump or climb meanwhile; it has ~9 s of swinging.
   A hammer you carry but are not swinging does NOT protect you.
 * Wild (hollow) barrels are faster and always take ladders.
 * Fireballs leave the oil drum when barrels fall in; they climb after you.
Rescue = 1000 + remaining bonus. Extra life at 7000. Lives = icons top-left.

SCORE DISPLAY FIX: the lives used to be a digit next to the score, so
"3" + "00000" read as 30000. Lives are now icons and the score has no
leading zeros (it starts at 0).

LAYOUT (redrawn after the arcade look): 4 girders with alternating slopes
and truss dots, the gorilla beside a 2x2 barrel stack on the top girder,
Luna on a small ledge above, broken (decorative) ladders, oil drum bottom
left, BONUS box top right.
Not included: other stages, original music, the "carry away" cutscene.

MEMORY (measured)
-----------------
 Flash 19,742 B of 30,592 B usable by loader games (60%).
 RAM   806 B static of 2,048 B; stack peak 77 B in a full emulated level.
 No malloc, no big RAM arrays, sprites in PROGMEM, EEPROM bytes 0-2 only.
 Speed: 25 fps; est. ~30% CPU load (emulator's SPI timing is inflated, so
 its own frame-time figure is meaningless - see test_evidence/verify.log).

BUILD
-----
Arduino IDE 1.8.x: install library "Gamebuino Classic" 0.5.2; open
source/BUINOKON/BUINOKON.ino; board Arduino/Genuino Uno; Sketch > Export
compiled Binary; use the *.standard.hex (NOT with_bootloader), rename to
BUINOKON.HEX. Command line: tools/build.sh, full checks: tools/verify.sh.
Sprites: edit ASCII art in tools/gen_sprites.py (BUINOKON.ino.in is the
template). INF: tools/gen_inf.py.

DEPENDENCIES
------------
 Gamebuino Classic 0.5.2 (LGPL-3.0) | Arduino AVR core 1.8.8 (SPI, EEPROM)
 avr-gcc 7.3.0 / avr-libc 2.0.0 (-Os -flto, gnu++11)
 Test only: simavr 1.6, g++, Python 3 + Pillow.

CHECKS (test_evidence/verify.log, 43 passing)
---------------------------------------------
 geometry (Mario fits between every girder pair), jump arc (8 px), barrel
 routing, ladders reachable, rescue, death/lives/game over, EEPROM hi-score,
 pause, loader exit, hammer (needs B; carrying alone is unsafe), fireballs,
 time-up, combo, plus the real firmware in an AVR emulator.
 Difficulty: a bot with PERFECT jump timing that never uses the hammer
 rescues Luna on its first life in 21/40 runs at level 1, 14/40 at level 2,
 10/40 at level 3, 3/40 at level 4. I can't playtest as a human; tuning
 constants are at the top of the .ino (barrelSpeed, spawnGap, BONUS_START,
 HAMMER_FRAMES, fire chance).

LEGAL/CREDITS
-------------
Renamed BUINOKONG with Luna as the rescue target. All code and sprites are
original (LGPL-3.0-or-later, matching the Gamebuino library). Note: the
gameplay formula and the gorilla/barrel/hammer motifs are heavily inspired by
a classic arcade game; renaming the title and character is not legal advice -
if you plan to publish or sell it, check the trademark/copyright position
(and consider changing the gorilla design too).
