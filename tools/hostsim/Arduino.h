// Minimal host-side stand-in for Arduino.h (testing only; NOT used for the real build)
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t*)(p))
#define pgm_read_word(p) (*(const uint16_t*)(p))
#define strlen_P strlen
typedef uint8_t byte; typedef bool boolean;
class __FlashStringHelper;
#define F(s) ((const __FlashStringHelper*)(s))
#define B10000000 0x80
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
inline long random(long n){ return rand() % n; }
inline void randomSeed(unsigned long s){ srand(s); }
struct Print {
  virtual size_t write(uint8_t)=0;
  size_t print(const char* s){ while(*s) write(*s++); return 0; }
  size_t print(const __FlashStringHelper* s){ return print((const char*)s); }
};
