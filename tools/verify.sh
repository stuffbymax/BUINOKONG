#!/usr/bin/env bash
# Full verification pipeline: build -> size gate -> host logic tests -> real-firmware emulation -> INF
set -euo pipefail; cd "$(dirname "$0")/.."
./tools/build.sh | tee /tmp/size.txt
FL=$(avr-size -A build/BUINOKON.elf | awk '/^\.text|^\.data/{s+=$2} END{print s}')
[ "$FL" -le 30592 ] && echo "FLASH GATE OK: $FL <= 30592 (loader settings page at 0x7780)"
g++ -std=c++17 -O1 -Itools/hostsim -fpermissive -w tools/hostsim/hostsim.cpp -o build/hostsim && ./build/hostsim
gcc tools/avrsim.c -o build/avrsim -lsimavr -lelf -lm && ./build/avrsim build/BUINOKON.elf
python3 tools/gen_inf.py build/BUINOKON.INF && python3 tools/check_inf.py /home/claude/Gamebuino-Games-Compilation/INVADERS.INF build/BUINOKON.INF
