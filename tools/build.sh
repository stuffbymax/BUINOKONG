#!/usr/bin/env bash
# Build DONKEYK.HEX exactly like the Arduino IDE would (avr-gcc 7.3, -Os, LTO)
# Needs: avr-gcc/avr-libc, ArduinoCore-avr, Gamebuino Classic library checked out.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CORE=${CORE:-$ROOT/../ArduinoCore-avr}
GB=${GB:-$ROOT/../Gamebuino}
OUT=$ROOT/build; rm -rf "$OUT"; mkdir -p "$OUT/obj" "$OUT/sketch"

# 1. assemble the sketch (sprites generated from ASCII art)
python3 "$ROOT/tools/gen_sprites.py" > "$OUT/sprites.inc"
python3 - "$ROOT/DONKEYK.ino.in" "$OUT/sprites.inc" "$ROOT/DONKEYK/DONKEYK.ino" <<'PY'
import sys; t=open(sys.argv[1]).read(); s=open(sys.argv[2]).read()
import os; os.makedirs(os.path.dirname(sys.argv[3]),exist_ok=True)
open(sys.argv[3],"w").write(t.replace("//@@SPRITES@@",s))
PY
cp "$ROOT/DONKEYK/DONKEYK.ino" "$OUT/sketch/DONKEYK.cpp"
sed -i '1i #include <Arduino.h>' "$OUT/sketch/DONKEYK.cpp"

DEF="-mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10813 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR"
INC="-I$CORE/cores/arduino -I$CORE/variants/standard -I$GB -I$GB/utility -I$CORE/libraries/SPI/src -I$CORE/libraries/EEPROM/src"
CXXF="-c -g -Os -Wall -Wextra -Wno-unused-parameter -std=gnu++11 -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -flto"
CF="-c -g -Os -w -std=gnu11 -ffunction-sections -fdata-sections -flto -fno-fat-lto-objects"
objs=()
build(){ local f=$1 o="$OUT/obj/$(echo "$1" | md5sum | cut -c1-8)_$(basename "$1").o"
  case $f in *.c) avr-gcc $CF $DEF $INC "$f" -o "$o";; *.S) avr-gcc -c -g -x assembler-with-cpp $DEF $INC "$f" -o "$o";; *) avr-g++ $CXXF $DEF $INC "$f" -o "$o";; esac
  objs+=("$o"); }
for f in "$OUT/sketch/DONKEYK.cpp" "$GB/Gamebuino.cpp" "$GB"/utility/*.cpp "$GB"/utility/*.c \
         "$CORE/libraries/SPI/src/SPI.cpp" ; do build "$f"; done
# core library
core=()
for f in "$CORE"/cores/arduino/*.c "$CORE"/cores/arduino/*.cpp "$CORE"/cores/arduino/*.S; do
  o="$OUT/obj/core_$(basename "$f").o"
  case $f in *.c) avr-gcc $CF $DEF $INC "$f" -o "$o";; *.S) avr-gcc -c -g -x assembler-with-cpp $DEF $INC "$f" -o "$o";; *) avr-g++ $CXXF $DEF $INC "$f" -o "$o";; esac
  avr-gcc-ar rcs "$OUT/core.a" "$o"
done
avr-gcc -w -Os -g -flto -fuse-linker-plugin -Wl,--gc-sections -mmcu=atmega328p \
  -o "$OUT/DONKEYK.elf" "${objs[@]}" "$OUT/core.a" -lm
avr-objcopy -O ihex -R .eeprom "$OUT/DONKEYK.elf" "$OUT/DONKEYK.HEX"
tr -d "\r" < "$OUT/DONKEYK.HEX" > "$OUT/H.tmp" && mv "$OUT/H.tmp" "$OUT/DONKEYK.HEX"   # LF endings like the stock games
avr-size -C --mcu=atmega328p "$OUT/DONKEYK.elf"
ls -l "$OUT/DONKEYK.HEX"
