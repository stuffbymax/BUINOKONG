DONKEY KONG (level 1) for the original MAKERbuino / Gamebuino Classic
=====================================================================
File: DONKEYK     Target: ATmega328P @16 MHz, 84x48 LCD, D-pad + A/B/C

STATUS - READ THIS FIRST
------------------------
* NOT tested on physical MAKERbuino hardware (none available to me).
* It WAS compiled with avr-gcc 7.3.0 against the real Gamebuino Classic 0.5.2
  library, and the resulting firmware was RUN in the simavr AVR emulator
  (UI navigation, gameplay, scoring, LCD output decoded from SPI). See
  test_evidence/. Emulation is strong evidence, not proof: please report
  anything odd on real hardware (sound levels, contrast, button feel).
* The INF format was verified against the loader's own source (Loader.ino in
  the Gamebuino library) and the official INVADERS.INF. I did NOT run the
  loader itself (it needs SD-card hardware), so "appears in the menu" is
  verified by format, not by observation.

INSTALL (SD card)
-----------------
1. SD card formatted FAT16/FAT32 that already has the MAKERbuino/Gamebuino
   loader set-up (LOADER.HEX etc. in the card ROOT). Do not touch these.
2. Copy SD_CARD/DONKEYK.HEX and SD_CARD/DONKEYK.INF to the ROOT of the card
   (same folder as the other games' .HEX/.INF files; there is no sub-folder
   convention on the Classic loader).
3. Put the card in, power on, pick "DONKEYK" in the loader's list, press A.
   The loader flashes the game, so it takes a few seconds. The ATmega328P
   cannot run a HEX straight from SD - the loader/bootloader flashes it.
4. Return to the loader any time: hold C then press & hold A. (Or press C on
   the title screen.)
Files: DONKEYK.HEX (firmware, 16,700 bytes of code), DONKEYK.INF (icon, name,
3 preview slides). DONKEYK.SAV is created by the loader itself.

CONTROLS (also in-game: B on the title screen)
----------------------------------------------
 Left/Right  walk              Up/Down   climb ladders (stand at the ladder)
 A or B      jump              C         pause (in pause: C resume, B quit)
 Hold C+A    return to loader  C+B       volume (library feature)
Goal: climb the 6 girders to Pauline at the top. Jump barrels (+100 each).
Rescue = 1000 + time bonus. Extra life at 7000. 3 lives. Each rescue starts
a harder level: faster barrels, more of them, more barrels choosing ladders.
High score is kept in EEPROM; the loader copies it to DONKEYK.SAV when you
leave via the loader, so exit with C+A / C-on-title to keep it.

WHAT IS AND ISN'T IN IT (honest list)
-------------------------------------
Included: Mario walk/jump/climb, gravity jump, ladders, DK throwing barrels
from a barrel stack, Pauline, rolling barrels that drop at girder ends and
sometimes ladders, oil drum, lives, score, high score, game over, title,
controls screen, pause, difficulty ramp, bonus timer, sound effects.
Not included: hammers, fireballs, rivets/elevators/conveyor stages, the
original's 6-girder portrait layout (re-drawn to fit 84x48), original
music. Girders are 1px with a 1px slope step. All art is original.

BUILD FROM SOURCE
-----------------
Arduino IDE 1.8.x:
  1. Library Manager: install "Gamebuino Classic" 0.5.2
  2. Open source/DONKEYK/DONKEYK.ino ; Board: Arduino/Genuino Uno
  3. Sketch > Export compiled Binary -> use DONKEYK.ino.standard.hex
     (NOT ...with_bootloader.hex); rename to DONKEYK.HEX.
Command line (what I used):  tools/build.sh  (needs avr-gcc/avr-libc,
  ArduinoCore-avr and the Gamebuino library checked out next to the project).
  tools/verify.sh runs build + all tests. Sprites: edit the ASCII art in
  tools/gen_sprites.py (DONKEYK.ino.in is the template; the .ino is generated).
  INF: tools/gen_inf.py.

DEPENDENCIES / VERSIONS USED
----------------------------
 Gamebuino Classic library 0.5.2 (git ab82e57)   LGPL-3.0
 Arduino AVR core 1.8.8 (git 11b9130), libraries SPI + EEPROM (bundled)
 avr-gcc 7.3.0, avr-libc 2.0.0 (flags copied from the core's platform.txt:
 -Os -flto -fno-exceptions, gnu++11)
 Test-only: simavr 1.6, g++, Python 3 + Pillow.  No other dependencies.

MEMORY (measured)
-----------------
 Flash 16,700 B of 30,592 B usable by loader games (limit = 0x7780, the
        loader's settings page) -> 55% of the limit, 13.8 KB free.
 RAM   776 B static of 2,048 B (38%); stack peak 72 B observed over a full
        bot-played level -> ~1,270 B free. Game's own state < 120 B.
 No malloc, no big RAM arrays; all sprites in PROGMEM.
 EEPROM: bytes 0-2 only (magic + 16-bit high score).
 Speed: runs at 25 fps. Profiling in the emulator puts game code (draw +
        logic) at ~7 ms/frame; real-SPI LCD refresh ~2 ms; ~25% CPU load
        ESTIMATED. (simavr charges a fixed ~51 ms per LCD refresh regardless
        of SPI clock - I proved this with a micro-benchmark - so the
        emulator's own "frame time" figure in the log is not meaningful.)

HOW IT WAS CHECKED (details: test_evidence/verify.log)
------------------------------------------------------
 - 27 automated checks on the real sketch source (mock Gamebuino API):
   geometry (Mario fits between every girder pair), jump arc (7 px, 0.64 s),
   barrels always travel 5->0 and never stick, level-1 trip time, difficulty
   ramp, every level reachable by ladder, rescue scoring, death/lives/game
   over, EEPROM hi-score, pause/resume, C->loader.
 - Real firmware in simavr: boots, title -> controls -> back -> start ->
   READY -> PLAY; a bot using only button presses climbed to Pauline and
   scored 1910 (jump-overs counted).
 - Caveat: a bot that jumps perfectly clears level 1 easily; real humans on
   a tiny screen will find later levels hard. Tuning constants are at the top
   of the .ino (WALK_SPD, JUMP_V, spawnGap(), barrelSpeed()).

LICENSE / CREDITS
-----------------
Game code & art: original, LGPL-3.0-or-later (matches the Gamebuino library).
Fan homage; "Donkey Kong" is a Nintendo trademark, not affiliated.
