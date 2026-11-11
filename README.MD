# DONKEY KONG (Level 1)
### Original MAKERbuino / Gamebuino Classic

**File:** `DONKEYK`  
**Target:** ATmega328P @ 16 MHz, 84×48 LCD, D-pad + A/B/C

---

## STATUS — READ THIS FIRST

- **Not tested on physical MAKERbuino hardware** (none available to me).
- It **was compiled** with `avr-gcc 7.3.0` against the real Gamebuino Classic 0.5.2 library, and the resulting firmware was **run in the simavr AVR emulator** (UI navigation, gameplay, scoring, LCD output decoded from SPI). See `test_evidence/`.
- Emulation is strong evidence, not proof. Please report anything odd on real hardware (sound levels, contrast, button feel).
- The INF format was verified against the loader's own source (`Loader.ino` in the Gamebuino library) and the official `INVADERS.INF`. I did **not** run the loader itself (it needs SD-card hardware), so "appears in the menu" is verified by format, not by observation.

## INSTALL (SD CARD)

1. Use an SD card formatted FAT16/FAT32 that already has the MAKERbuino/Gamebuino loader setup (`LOADER.HEX` etc. in the card **ROOT**). Do not touch these files.
2. Copy the following files to the **ROOT** of the card:
   - `SD_CARD/DONKEYK.HEX`
   - `SD_CARD/DONKEYK.INF`

   Put them in the same folder as the other games' `.HEX`/`.INF` files. There is no sub-folder convention on the Classic loader.
3. Insert the card, power on, pick **DONKEYK** in the loader's list, and press **A**.

   The loader flashes the game, so it takes a few seconds. The ATmega328P cannot run a HEX straight from SD — the loader/bootloader flashes it.
4. Return to the loader any time:
   - Hold **C**, then press and hold **A**.
   - Or press **C** on the title screen.

**Files:**

| File | Description |
|---|---|
| `DONKEYK.HEX` | Firmware (16,700 bytes of code) |
| `DONKEYK.INF` | Icon, name, 3 preview slides |
| `DONKEYK.SAV` | Created by the loader itself |

## CONTROLS

*(Also available in-game: press B on the title screen.)*

| Input | Action |
|---|---|
| Left / Right | Walk |
| Up / Down | Climb ladders (stand at the ladder) |
| A or B | Jump |
| C | Pause |
| C (while paused) | Resume |
| B (while paused) | Quit |
| Hold C + A | Return to loader |
| C + B | Volume (library feature) |

### Goal

Climb the 6 girders to Pauline at the top.

- Jump over barrels: **+100 points each**.
- Rescue bonus: **1000 + time bonus**.
- Extra life at **7000 points**.
- Start with **3 lives**.
- Each rescue starts a harder level:
  - Faster barrels.
  - More barrels.
  - More barrels choosing ladders.

The high score is kept in EEPROM. The loader copies it to `DONKEYK.SAV` when you leave via the loader, so exit with **C+A** or **C on the title screen** to keep it.

## WHAT IS AND ISN'T IN IT

### Included

- Mario walk, jump and climb.
- Gravity-based jumping.
- Ladders.
- Donkey Kong throwing barrels from a barrel stack.
- Pauline.
- Rolling barrels that drop at girder ends and sometimes use ladders.
- Oil drum.
- Lives and scoring.
- High score.
- Game over.
- Title screen.
- Controls screen.
- Pause.
- Difficulty ramp.
- Bonus timer.
- Sound effects.

### Not included

- Hammers.
- Fireballs.
- Rivets, elevators or conveyor stages.
- The original's 6-girder portrait layout (redrawn to fit 84×48).
- Original music.

**Visual notes:**
- Girders are 1 px with a 1 px slope step.
- All artwork is original.

## BUILD FROM SOURCE

### Arduino IDE 1.8.x

1. Open Library Manager and install **Gamebuino Classic 0.5.2**.
2. Open `source/DONKEYK/DONKEYK.ino`.
3. Select board: **Arduino/Genuino Uno**.
4. Go to **Sketch → Export compiled Binary**.
5. Use `DONKEYK.ino.standard.hex` (NOT `...with_bootloader.hex`).
6. Rename it to `DONKEYK.HEX`.

### Command line

The build command used:

```bash
tools/build.sh
```

Requirements:

- `avr-gcc`
- `avr-libc`
- `ArduinoCore-avr`
- Gamebuino library checked out next to the project.

Run all tests with:

```bash
tools/verify.sh
```

**Sprites:** Edit the ASCII art in `tools/gen_sprites.py`. The `DONKEYK.ino.in` file is the template; the `.ino` is generated.

**INF:** Generated using:

```bash
tools/gen_inf.py
```

## DEPENDENCIES / VERSIONS USED

| Dependency | Version / Details |
|---|---|
| Gamebuino Classic library | 0.5.2 (git `ab82e57`), LGPL-3.0 |
| Arduino AVR core | 1.8.8 (git `11b9130`) |
| SPI | Bundled with Arduino AVR core |
| EEPROM | Bundled with Arduino AVR core |
| avr-gcc | 7.3.0 |
| avr-libc | 2.0.0 |
| simavr | 1.6 (test only) |
| g++ | Test only |
| Python 3 + Pillow | Test only |

Compiler flags copied from the core's `platform.txt`:

```text
-Os -flto -fno-exceptions, gnu++11
```

No other dependencies.

## MEMORY (MEASURED)

| Resource | Usage | Details |
|---|---:|---|
| Flash | 16,700 B / 30,592 B | 55% of loader limit; 13.8 KB free |
| RAM | 776 B / 2,048 B | 38% static usage |
| Stack | 72 B peak | Observed over a full bot-played level; approximately 1,270 B free |
| EEPROM | Bytes 0–2 | Magic + 16-bit high score |

### Additional details

- Flash limit: `0x7780` (the loader's settings page).
- Game's own state: less than 120 B.
- No `malloc`.
- No large RAM arrays.
- All sprites stored in `PROGMEM`.

### Performance

- Runs at **25 FPS**.
- Emulator profiling puts game code (draw + logic) at approximately **7 ms/frame**.
- Real-SPI LCD refresh: approximately **2 ms**.
- Estimated CPU load: approximately **25%**.

**Emulator timing caveat:**

`simavr` charges a fixed ~51 ms per LCD refresh regardless of SPI clock. This was verified with a micro-benchmark, so the emulator's own "frame time" figure in the log is not meaningful.

## HOW IT WAS CHECKED

Details: `test_evidence/verify.log`

### Automated tests

27 automated checks on the real sketch source using a mock Gamebuino API:

- Geometry (Mario fits between every girder pair).
- Jump arc (7 px, 0.64 s).
- Barrels always travel 5 → 0 and never stick.
- Level 1 trip time.
- Difficulty ramp.
- Every level reachable by ladder.
- Rescue scoring.
- Death, lives and game over.
- EEPROM high score.
- Pause and resume.
- C → loader.

### Firmware emulation

Real firmware was run in simavr.

Verified sequence:

1. Boot.
2. Title screen.
3. Controls screen.
4. Return.
5. Start.
6. READY.
7. PLAY.

A bot using only button presses climbed to Pauline and scored **1910 points** (jump-overs counted).

### Known caveat

A bot that jumps perfectly clears level 1 easily. Real humans on a tiny screen will find later levels hard.

Tuning constants are at the top of the `.ino` file:

- `WALK_SPD`
- `JUMP_V`
- `spawnGap()`
- `barrelSpeed()`

## LICENSE / CREDITS

**Game code and artwork:** Original, LGPL-3.0-or-later (matches the Gamebuino library).

Fan homage. **Donkey Kong** is a Nintendo trademark. This project is not affiliated with or endorsed by Nintendo.
