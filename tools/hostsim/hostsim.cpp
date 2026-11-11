#include <vector>
#include <string>
#include <algorithm>
#include <sys/stat.h>
// Host-side verification harness for DONKEYK (NOT part of the firmware).
// Compiles the *actual sketch source* against a mock of the Gamebuino API.
#include "Gamebuino.h"
#include "EEPROM.h"
EEPROMClass EEPROM;
#include "/home/claude/Gamebuino/utility/font3x5.c"
#include "/home/claude/Gamebuino/utility/font5x7.c"
#include "../../DONKEYK/DONKEYK.ino"
#include <vector>
#include <string>
#include <sys/stat.h>

enum { K_L=1,K_R=2,K_U=4,K_D=8,K_A=16,K_B=32,K_C=64 };
static void step(int mask=0){
  const uint8_t map[7]={BTN_LEFT,BTN_RIGHT,BTN_UP,BTN_DOWN,BTN_A,BTN_B,BTN_C};
  for(int i=0;i<7;i++) gb.buttons.feed(mask&(1<<i), map[i]);
  loop();
}
static void shot(const char* name){
  mkdir("/home/claude/dk/build/shots",0755);
  std::string p=std::string("/home/claude/dk/build/shots/")+name+".pbm";
  FILE*f=fopen(p.c_str(),"w"); fprintf(f,"P1\n84 48\n");
  for(int y=0;y<48;y++){ for(int x=0;x<84;x++) fprintf(f,"%d ",gb.display.px[y][x]); fprintf(f,"\n"); } fclose(f);
}
static int fails=0;
#define CHECK(c,msg) do{ if(!(c)){ printf("  FAIL: %s\n",msg); fails++; } else printf("  ok:   %s\n",msg); }while(0)
static void reset(){ memset(&gb.buttons,0,sizeof gb.buttons); }

int main(){
  srand(1); setup();

  printf("[1] Geometry\n");
  int mind=99;
  for(int k=0;k<5;k++) for(int x=0;x<=83;x++){
    if(x<xmin(k)||x>xmax(k)||x<xmin(k+1)||x>xmax(k+1)) continue;
    int d=surf(k,x)-surf(k+1,x); if(d<mind) mind=d; }
  printf("  min girder-to-girder spacing in overlap = %d px (Mario needs %d+1)\n",mind,MARIO_H);
  CHECK(mind>=MARIO_H+1,"Mario fits between every pair of girders");
  bool ladOk=true; int minL=99,maxL=0;
  for(int i=0;i<NLAD;i++){ int g=ladG(i),cx=ladX(i);
    if(cx-2<xmin(g)||cx+2>xmax(g)||cx-2<xmin(g+1)||cx+2>xmax(g+1)) ladOk=false;
    int len=surf(g,cx)-surf(g+1,cx); if(len<minL)minL=len; if(len>maxL)maxL=len; }
  CHECK(ladOk,"all ladders lie fully on both girders they join");
  printf("  ladder lengths %d..%d px\n",minL,maxL);
  bool cont=true; for(int k=0;k<6;k++) for(int x=1;x<84;x++) if(abs(surf(k,x)-surf(k,x-1))>1) cont=false;
  CHECK(cont,"girder surfaces never step more than 1 px");

  printf("[2] Title / help / controls screens\n");
  for(int i=0;i<40;i++) step(); shot("01_title");
  for(int i=0;i<30;i++) step(); shot("01b_title_hop");
  step(K_B); step(); shot("02_help"); CHECK(state==S_HELP,"B opens controls screen");
  step(K_A); step(); CHECK(state==S_TITLE,"A returns from controls");
  reset();
  step(K_A); step(); CHECK(state==S_READY,"A starts a game"); shot("03_ready");
  for(int i=0;i<51;i++) step(); CHECK(state==S_PLAY,"READY -> PLAY after 2 s");

  printf("[3] Jump physics\n");
  for(int i=0;i<5;i++) step();
  int y0=m.y8>>3,apex=0,air=0; step(K_A);
  while(m.mode==M_JUMP && air<100){ int h=y0-(m.y8>>3); if(h>apex)apex=h; air++; step(); }
  printf("  apex = %d px, air time = %d frames (%.2f s)\n",apex,air+1,(air+1)/25.0);
  CHECK(apex>=5&&apex<=8,"apex 5-8 px (clears a 4 px barrel)");
  CHECK(m.mode==M_WALK&&(m.y8>>3)==y0,"lands exactly back on girder");

  printf("[4] Barrel flow (no Mario)\n");
  {
    for(int i=0;i<MAXB;i++) bar[i].st=B_OFF;
    lvl=1; int maxAlive=0; std::vector<int> life(MAXB,0); int done=0; bool order=true; int stuck=0;
    int minLife=99999,maxLife=0, ladderDescents=0; static int maxLife1=0;
    std::vector<int> lastLev(MAXB,9);
    for(int f=0;f<6000;f++){
      if(f%100==0) spawnBarrel();
      updateBarrels();
      int alive=0;
      for(int i=0;i<MAXB;i++){
        if(bar[i].st==B_OFF){ if(life[i]){ done++; if(life[i]<minLife)minLife=life[i]; if(life[i]>maxLife)maxLife=life[i]; life[i]=0; lastLev[i]=9; } continue; }
        alive++; life[i]++;
        if(bar[i].st==B_LADDER && lastLev[i]!=-1 && false) ladderDescents++;
        if(bar[i].lev>lastLev[i]) order=false; lastLev[i]=bar[i].lev;
        if(life[i]>3000) stuck++;
      }
      if(alive>maxAlive) maxAlive=alive;
    }
    printf("  level 1 (100-frame spacing): %d barrels completed route, life %d..%d frames, max alive %d\n",done,minLife,maxLife,maxAlive);
    CHECK(done>=55,"barrels reach the oil drum and disappear");
    CHECK(order,"barrels only ever move downward (5->0)");
    CHECK(stuck==0,"no barrel gets stuck");
    maxLife1=maxLife; CHECK(maxLife<1200,"barrel trip time sane at level 1");
    lvl=8; for(int i=0;i<MAXB;i++) bar[i].st=B_OFF; maxAlive=0; done=0; minLife=99999;maxLife=0; life.assign(MAXB,0);
    for(int f=0;f<3000;f++){ if(f%30==0) spawnBarrel(); updateBarrels(); int alive=0;
      for(int i=0;i<MAXB;i++){ if(bar[i].st==B_OFF){ if(life[i]){done++; if(life[i]<minLife)minLife=life[i]; if(life[i]>maxLife)maxLife=life[i]; life[i]=0;} continue;} alive++; life[i]++; }
      if(alive>maxAlive)maxAlive=alive; }
    printf("  level 8 (30-frame spacing): trip %d..%d frames, max alive %d (cap %d)\n",minLife,maxLife,maxAlive,MAXB);
    CHECK(maxLife<maxLife1,"level 8 barrels are faster (shorter worst-case trip)");
  }

  printf("[5] Route to Pauline (no barrels), ladder climbing\n");
  {
    newGame(); for(int i=0;i<60;i++) step(); 
    int f=0; int shots=0;
    while(state!=S_WIN && f<2000){
      for(int i=0;i<MAXB;i++) bar[i].st=B_OFF; for(int i=0;i<MAXF;i++) fire[i].st=F_OFF; spawnT=200;     // keep the level empty
      int mask=0;
      if(m.mode==M_CLIMB){ mask=K_U; }
      else if(m.lev<LEDGE){
        // nearest ladder going up from this level
        int best=-1,bd=999; for(int i=0;i<NLAD;i++) if(ladG(i)==m.lev){ int d=abs(ladX(i)-marioCx()); if(d<bd){bd=d;best=i;} }
        int cx=ladX(best); int c=marioCx();
        if(abs(cx-c)<=1) mask=K_U; else mask=(cx>c)?K_R:K_L;
      } else mask=K_R;
      step(mask); f++;
      if(m.mode==M_CLIMB && shots==0 && m.lev==2){ shot("04_climbing"); shots=1; }
    }
    printf("  reached Pauline in %d frames (%.1f s)\n",f,f/25.0);
    CHECK(state==S_WIN,"every level reachable by ladders; touching Pauline wins");
    shot("05_win"); CHECK(score>=1000,"rescue awards 1000 + bonus");
    printf("  score after rescue = %u (bonus %u)\n",score,winBonus);
    for(int i=0;i<80;i++) step(); CHECK(state==S_READY&&lvl==2,"next level starts after rescue, difficulty up");
  }

  printf("[6] Death / lives / game over / high score\n");
  {
    reset(); newGame(); lvl=1; for(int i=0;i<60;i++) step();
    EEPROM.mem[0]=0; hiScore=0; addScore(300);
    int deaths=0; int f=0; uint8_t prevLives=lives;
    while(state!=S_OVER && f<20000){ step(0); f++; if(lives<prevLives){deaths++;prevLives=lives;} 
      if(state==S_PLAY && deaths==0 && f%97==0) shot("06_play"); }
    printf("  standing still: died %d times, game over after %d frames (%.0f s)\n",deaths+0,f,f/25.0);
    CHECK(state==S_OVER,"idle player eventually loses all lives");
    CHECK(deaths>=2,"three lives consumed (2 mid-game + final)");
    shot("07_gameover");
    CHECK(hiScore==300&&score==300,"high score updated to 300");
    CHECK(EEPROM.mem[0]==EE_MAGIC,"high score written to EEPROM (magic present)");
    for(int i=0;i<50;i++) step(); step(K_A); step(); CHECK(state==S_READY,"A after game over restarts");
  }

  printf("[7] Pause + exit paths\n");
  {
    for(int i=0;i<60;i++) step(); reset();
    step(K_C); step(); CHECK(state==S_PAUSE,"C pauses"); shot("08_pause");
    for(int i=0;i<10;i++) step(); step(K_C); step(); CHECK(state==S_PLAY,"C resumes");
    reset(); state=S_TITLE; step(); step(K_C); CHECK(gb.wantLoader,"C on title calls changeGame() -> loader");
  }

  printf("[8] Autoplay bot (perfect-timing barrel jumper, never uses hammer) by level\n");
  for(int L=1;L<=4;L+=1){
    int wins=0,tot=0,deathsAll=0; unsigned best=0;
    for(int run=0;run<40;run++){
      srand(100+run+L*1000); reset(); newGame(); lvl=L; startLife(); int f=0; uint8_t startLives=lives;
      while(state!=S_OVER && state!=S_WIN && f<8000 && lives==startLives){
        int mask=0;
        if(state==S_PLAY){
          if(m.mode==M_CLIMB) mask=K_U;
          else if(m.lev<LEDGE){
            int best_=-1,bd=999; for(int i=0;i<NLAD;i++) if(ladG(i)==m.lev){ int d=abs(ladX(i)-marioCx()); if(d<bd){bd=d;best_=i;} }
            int cx=ladX(best_),c=marioCx(); int dirx=(cx>c)?1:-1;
            if(abs(cx-c)<=1) mask=K_U; else mask=(dirx>0)?K_R:K_L;
            if(m.mode==M_WALK){ for(int i=0;i<MAXB;i++) if(bar[i].st==B_ROLL&&bar[i].lev==m.lev){
                int d=((bar[i].x8>>3)+2)-c; int bd2=rollDir(bar[i].lev);
                if(d*bd2<0 && abs(d)<=7 && abs(d)>=4) mask|=K_A; }
              for(int i=0;i<MAXF;i++) if(fire[i].st==F_WALK&&fire[i].lev==m.lev){ int d=((fire[i].x8>>3)+2)-c; if(abs(d)>=4&&abs(d)<=7) mask|=K_A; } }
          } else mask=K_R;
        }
        step(mask); f++;
      }
      tot++; if(state==S_WIN) wins++; if(lives<startLives||state==S_OVER) deathsAll++; if(score>best)best=score;
    }
    printf("  level %d: bot rescued Pauline on its FIRST life in %2d/%d runs (died %d)\n",L,wins,tot,deathsAll);
    if(L==1) CHECK(wins<tot,"level 1 is no longer a guaranteed clear for a perfect-timing bot");
    if(L==4) CHECK(wins<=tot/2,"by level 4 a perfect-timing bot fails at least half the time");
  }

  printf("[9] Hammer\n");
  {
    reset(); newGame(); lvl=2; startLife(); for(int i=0;i<60;i++) step();
    // teleport Mario next to hammer 0 (level 2, x=78)
    m.lev=2; m.x8=76*8; m.y8=surf(2,78)*8; m.mode=M_WALK;
    for(int i=0;i<MAXB;i++) bar[i].st=B_OFF; spawnT=250;
    step(); CHECK(m.ham>0,"walking over a hammer picks it up"); CHECK(hamTaken&1,"hammer item is consumed");
    // put a barrel in front of Mario and let him smash it
    unsigned sc0=score; bar[0].st=B_ROLL; bar[0].lev=2; bar[0].wild=0; bar[0].scored=0; bar[0].x8=(((m.x8>>3)+10))*8; bar[0].y=surf(2,88)-4;   // ahead of Mario, rolling towards him (level 2 rolls left)
    int died=0; for(int i=0;i<30;i++){ step(); if(state!=S_PLAY) died=1; }
    CHECK(!died,"Mario survives while swinging the hammer"); CHECK(bar[0].st==B_OFF&&score>=sc0+300,"hammer smashes a barrel for +300");
    // can't jump or climb with hammer
    uint8_t ml=m.mode; step(K_A); CHECK(m.mode==ml&&m.mode!=M_JUMP,"no jumping while holding the hammer");
    for(int i=0;i<HAMMER_FRAMES+5;i++){ spawnT=250; for(int k=0;k<MAXB;k++) bar[k].st=B_OFF; step(); }
    CHECK(m.ham==0,"hammer runs out");
  }
  printf("[10] Fireballs\n");
  {
    reset(); newGame(); lvl=3; startLife(); for(int i=0;i<60;i++) step();
    for(int i=0;i<MAXB;i++) bar[i].st=B_OFF; spawnT=250;
    srand(7); int spawned=0; for(int t=0;t<200&&!spawned;t++){ maybeSpawnFire(); for(int i=0;i<MAXF;i++) if(fire[i].st) spawned=1; }
    CHECK(spawned,"drum can release a fireball");
    // Mario parked on level 3: fire should come up to meet him
    m.lev=3; m.x8=40*8; m.y8=surf(3,42)*8; m.mode=M_WALK; int reached=0, maxlev=0; int x8min=9999,x8max=0;
    for(int f=0;f<2500;f++){ spawnT=250; for(int k=0;k<MAXB;k++) bar[k].st=B_OFF; m.mode=M_WALK; m.lev=3; m.x8=40*8; m.y8=surf(3,42)*8; m.ham=0;
      gb.frameCount++; updateFires(); for(int i=0;i<MAXF;i++) if(fire[i].st){ if(fire[i].lev>maxlev)maxlev=fire[i].lev; if(fire[i].lev==3)reached=1; int x=fire[i].x8>>3; if(x<x8min)x8min=x; if(x>x8max)x8max=x; } }
    printf("  fire reached level %d (max), x-range %d..%d\n",maxlev,x8min,x8max);
    CHECK(reached,"fireball climbs ladders towards Mario");
    CHECK(maxlev<LEDGE,"fireball never climbs to Pauline's ledge");
    CHECK(x8min>=0&&x8max<=84,"fireball stays on screen");
  }
  printf("[11] Time-up and combo scoring\n");
  {
    reset(); newGame(); lvl=1; startLife(); for(int i=0;i<60;i++) step();
    for(int i=0;i<MAXB;i++) bar[i].st=B_OFF; for(int i=0;i<MAXF;i++) fire[i].st=F_OFF; spawnT=250; bonus=200;
    uint8_t l0=lives; int f=0; while(state==S_PLAY&&f<200){ spawnT=250; step(); f++; }
    CHECK(state==S_DYING&&timeUp,"bonus timer reaching 0 kills Mario");
    for(int i=0;i<70;i++) step(); CHECK(lives==l0-1,"time-up costs a life");
    // combo: 100 then 300
    reset(); newGame(); lvl=1; startLife(); for(int i=0;i<60;i++) step(); score=0;
    m.combo=0; m.mode=M_JUMP; m.lev=0; m.y8=(surf(0,30)-6)*8; m.x8=28*8; m.vy=0;
    bar[0].st=B_ROLL; bar[0].lev=0; bar[0].scored=0; bar[0].x8=28*8; bar[0].y=surf(0,30)-4;
    bar[1].st=B_ROLL; bar[1].lev=0; bar[1].scored=0; bar[1].x8=30*8; bar[1].y=surf(0,30)-4;
    scoreJumps(); CHECK(score==100+300,"jumping two barrels in one jump scores 100 + 300");
  }
  { // staged frame for a visual check
    reset(); newGame(); lvl=3; startLife(); for(int i=0;i<60;i++) step();
    for(int i=0;i<MAXB;i++) bar[i].st=B_OFF; spawnT=250; bonus=2100;
    m.lev=2; m.x8=60*8; m.y8=surf(2,62)*8; m.mode=M_WALK; m.ham=150; m.left=0;
    bar[0]=Barrel{ (int16_t)(40*8),(int8_t)(surf(3,42)-4),3,B_ROLL,0,0,1 };
    bar[1]=Barrel{ (int16_t)(20*8),(int8_t)(surf(1,22)-4),1,B_ROLL,0,0,0 };
    bar[2]=Barrel{ (int16_t)(66*8),(int8_t)(surf(4,66)-4),4,B_ROLL,0,0,0 };
    fire[0].st=F_WALK; fire[0].lev=0; fire[0].x8=30*8; fire[0].dir=1; fire[0].y=surf(0,32)-5;
    gb.frameCount=64+8; gb.display.clear(); drawScene(true); shot("10_action");
  }
  printf("\n%s (%d failures)\n",fails?"SOME TESTS FAILED":"ALL TESTS PASSED",fails);
  return fails?1:0;
}
