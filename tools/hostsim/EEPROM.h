#pragma once
#include "Arduino.h"
struct EEPROMClass { uint8_t mem[1024]; uint8_t read(int a){return mem[a];} void write(int a,uint8_t v){mem[a]=v;} };
extern EEPROMClass EEPROM;
