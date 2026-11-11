// Host mock of the Gamebuino Classic API subset used by DONKEYK. Semantics copied
// from Gamebuino Classic 0.5.2 (Buttons.cpp / Display.cpp) so behaviour matches.
#pragma once
#include "Arduino.h"
#define LCDWIDTH 84
#define LCDHEIGHT 48
#define WHITE 0
#define BLACK 1
#define NOROT 0
#define NOFLIP 0
#define FLIPH 1
#define BTN_LEFT 0
#define BTN_UP 1
#define BTN_RIGHT 2
#define BTN_DOWN 3
#define BTN_A 4
#define BTN_B 5
#define BTN_C 6
#define NUM_BTN 7
extern const uint8_t font3x5[]; extern const uint8_t font5x7[];

struct Display : public Print {
  uint8_t px[LCDHEIGHT][LCDWIDTH]; uint8_t color=BLACK;
  const uint8_t* font; uint8_t fontWidth, fontHeight, fontSize=1; int8_t cursorX=0, cursorY=0;
  Display(){ setFont(font3x5); clear(); }
  void clear(){ memset(px,0,sizeof px); }
  void setColor(int8_t c){ color=c; }
  void setFont(const uint8_t*f){ font=f; fontWidth=pgm_read_byte(font)+1; fontHeight=pgm_read_byte(font+1)+1; font+=2; }
  void drawPixel(int8_t x,int8_t y){ if(x<0||y<0||x>=LCDWIDTH||y>=LCDHEIGHT) return; px[y][x]=(color==BLACK); }
  void drawFastHLine(int8_t x,int8_t y,int8_t w){ for(int8_t i=0;i<w;i++) drawPixel(x+i,y); }
  void drawFastVLine(int8_t x,int8_t y,int8_t h){ for(int8_t i=0;i<h;i++) drawPixel(x,y+i); }
  void drawRect(int8_t x,int8_t y,int8_t w,int8_t h){ drawFastHLine(x,y,w); drawFastHLine(x,y+h-1,w); drawFastVLine(x,y,h); drawFastVLine(x+w-1,y,h); }
  void fillRect(int8_t x,int8_t y,int8_t w,int8_t h){ for(int8_t i=y;i<y+h;i++) drawFastHLine(x,i,w); }
  bool getBitmapPixel(const uint8_t* b,uint8_t x,uint8_t y){ return pgm_read_byte(b+2+y*((pgm_read_byte(b)+7)/8)+(x>>3)) & (0x80>>(x%8)); }
  void drawBitmap(int8_t x,int8_t y,const uint8_t*b,uint8_t rot=0,uint8_t flip=0){
    int w=pgm_read_byte(b), h=pgm_read_byte(b+1);
    for(int j=0;j<h;j++) for(int i=0;i<w;i++) if(getBitmapPixel(b,i,j)){ int k=(flip&1)?w-i-1:i; if(color==BLACK) drawPixel(x+k,y+j); }
  }
  void drawChar(int8_t x,int8_t y,unsigned char c,uint8_t size){
    for(int8_t i=0;i<fontWidth;i++){ uint8_t line=(i==fontWidth-1)?0:pgm_read_byte(font+c*(fontWidth-1)+i);
      for(int8_t j=0;j<fontHeight;j++){ if(line&1){ if(size==1) drawPixel(x+i,y+j); else fillRect(x+i*size,y+j*size,size,size);} line>>=1; } }
  }
  size_t write(uint8_t c) override { if(c=='\n'){cursorY+=fontSize*fontHeight;cursorX=0;} else { drawChar(cursorX,cursorY,c,fontSize); cursorX+=fontSize*fontWidth; } return 1; }
};
struct Buttons {
  uint8_t states[NUM_BTN]={0};
  bool pressed(uint8_t b){ return states[b]==1; }
  bool released(uint8_t b){ return states[b]==0xFF; }
  bool held(uint8_t b,uint8_t t){ return states[b]==t+1; }
  bool repeat(uint8_t b,uint8_t p){ if(p<=1){ return states[b]!=0xFF && states[b]; } return states[b]!=0xFF && states[b]%p==1; }
  void feed(bool down,uint8_t b){            // exactly Buttons::update() logic
    if(down) states[b]++; else if(states[b]){ states[b]=(states[b]==0xFF)?0:0xFF; } }
};
struct Sound { int notes=0, lastPitch=-1; void playNote(uint8_t p,uint8_t d,uint8_t c){notes++; lastPitch=p;} void playOK(){notes++;} void playTick(){notes++;} };
struct Battery { bool show=true; };
struct Gamebuino {
  Display display; Buttons buttons; Sound sound; Battery battery; uint32_t frameCount=0; bool wantLoader=false;
  void begin(){} void setFrameRate(uint8_t){} void pickRandomSeed(){}
  bool update(){ display.clear(); frameCount++; return true; }
  void changeGame(){ wantLoader=true; }
};
extern Gamebuino gb;
