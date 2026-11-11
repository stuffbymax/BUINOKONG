// Runs the REAL compiled DONKEYK firmware in simavr (ATmega328P @16MHz), emulates the
// PCD8544 LCD from the SPI stream, drives the D-pad/A/B/C pins, reads game RAM for a bot.
// Build: gcc avrsim.c -o avrsim -lsimavr -lelf
#include <simavr/sim_avr.h>
#include <simavr/avr_ioport.h>
#include <simavr/avr_spi.h>
#include <simavr/sim_elf.h>
#include <simavr/avr_adc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
static avr_t* avr; static uint8_t lcd[504], shown[504]; static int bank,col,dc,cs=1; static long frames;
static uint64_t SYM_gb=0x800181;
static unsigned long symaddr(const char*elf,const char*n){ char c[256]; snprintf(c,sizeof c,"avr-nm %s | awk '$3==\"%s\"{print $1}'",elf,n);
  FILE*p=popen(c,"r"); char b[32]={0}; fgets(b,32,p); pclose(p); return strtoul(b,0,16)-0x800000; }
static void on_dc(struct avr_irq_t*i,uint32_t v,void*p){ dc=v; }
static void on_cs(struct avr_irq_t*i,uint32_t v,void*p){ if(v&&!cs){ /* rising: end of burst */ } cs=v; }
static void on_spi(struct avr_irq_t*i,uint32_t v,void*p){
  if(cs) return; uint8_t b=v;
  if(!dc){ if((b&0xF8)==0x40) bank=b&7; else if(b&0x80) col=b&0x7F; }
  else { if(bank<6&&col<84) lcd[bank*84+col]=b; col++; if(col>=84){col=0;} if(bank==5&&col==83) {} }
  if(!dc && b==0x40 && bank==0){ memcpy(shown,lcd,504); frames++; }   // last cmd of update() = SETYADDR|0
}
static void shot(const char*name){ char p[256]; snprintf(p,sizeof p,"/home/claude/dk/build/avr_%s.pbm",name); FILE*f=fopen(p,"w"); fprintf(f,"P1\n84 48\n");
  for(int y=0;y<48;y++){ for(int x=0;x<84;x++) fprintf(f,"%d ",(shown[(y/8)*84+x]>>(y%8))&1); fprintf(f,"\n"); } fclose(f); }
// button pins: L=D8(B0) U=D9(B1) R=D7(D7) D=D6(D6) A=D4(D4) B=D2(D2) C=A3(C3)
enum {K_L=1,K_R=2,K_U=4,K_D=8,K_A=16,K_B=32,K_C=64};
static const struct { char port; int pin; } BP[7]={{'B',0},{'D',7},{'B',1},{'D',6},{'D',4},{'D',2},{'C',3}};
static void setkeys(int mask){ const char P[3]={'B','C','D'};
  for(int k=0;k<3;k++){ avr_ioport_external_t ep; ep.name=P[k]; ep.mask=0; ep.value=0;
    for(int i=0;i<7;i++) if(BP[i].port==P[k]){ ep.mask|=1<<BP[i].pin; if(!(mask&(1<<i))) ep.value|=1<<BP[i].pin; }
    avr_ioctl(avr,AVR_IOCTL_IOPORT_SET_EXTERNAL(P[k]),&ep); } }
static unsigned minsp=0xFFFF; static unsigned hist[32768]; static int sampling=0; static uint64_t nextS=0;
static void runms(double ms){ uint64_t t=avr->cycle+(uint64_t)(ms*16000); while(avr->cycle<t){ int s=avr_run(avr); { unsigned sp=avr->data[0x5D]|(avr->data[0x5E]<<8); if(sp&&sp<minsp) minsp=sp; } if(sampling&&avr->cycle>=nextS){ nextS=avr->cycle+997; hist[(avr->pc>>1)&0x7fff]++; } if(s==cpu_Done||s==cpu_Crashed){printf("CPU stopped %d\n",s);exit(2);} } }
static unsigned long A_state,A_lives,A_score,A_lvl,A_m,A_bar,A_hi;
static uint8_t  r8(unsigned long a){return avr->data[a];}
static uint16_t r16(unsigned long a){return avr->data[a]|(avr->data[a+1]<<8);}
static int s8(unsigned long a){return (int8_t)avr->data[a];}
int main(int argc,char**argv){
  const char*elf=argv[1]; elf_firmware_t fw; memset(&fw,0,sizeof fw); elf_read_firmware(elf,&fw);
  strcpy(fw.mmcu,"atmega328p"); fw.frequency=16000000; avr=avr_make_mcu_by_name("atmega328p"); avr_init(avr); avr_load_firmware(avr,&fw); avr->frequency=16000000;
  avr_irq_register_notify(avr_io_getirq(avr,AVR_IOCTL_SPI_GETIRQ(0),SPI_IRQ_OUTPUT),on_spi,0);
  avr_irq_register_notify(avr_io_getirq(avr,AVR_IOCTL_IOPORT_GETIRQ('C'),2),on_dc,0);
  avr_irq_register_notify(avr_io_getirq(avr,AVR_IOCTL_IOPORT_GETIRQ('C'),1),on_cs,0);
  for(int ch=0;ch<8;ch++) avr_raise_irq(avr_io_getirq(avr,AVR_IOCTL_ADC_GETIRQ,ADC_IRQ_ADC0+ch),3300);
  setkeys(0);
  A_state=symaddr(elf,"state");A_lives=symaddr(elf,"lives");A_score=symaddr(elf,"score");A_lvl=symaddr(elf,"lvl");A_m=symaddr(elf,"m");A_bar=symaddr(elf,"bar");A_hi=symaddr(elf,"hiScore");
  unsigned long gb=symaddr(elf,"gb"); 
  // ---- scripted UI walk-through on the real firmware
  runms(2500); shot("title"); printf("title: frames drawn=%ld\n",frames);
  setkeys(K_B); runms(120); setkeys(0); runms(300); shot("help"); printf("after B: state=%d (1=HELP)\n",r8(A_state));
  setkeys(K_A); runms(120); setkeys(0); runms(200); printf("after A: state=%d (0=TITLE)\n",r8(A_state));
  setkeys(K_A); runms(120); setkeys(0); runms(600); shot("ready"); printf("after A: state=%d (2=READY) lives=%d\n",r8(A_state),r8(A_lives));
  runms(2000); printf("state=%d (3=PLAY)\n",r8(A_state));
  // ---- reactive bot on the real firmware (same strategy as host bot), measure frame time
  static const int LAD[11][2]={{0,62},{0,28},{1,16},{1,52},{2,66},{2,30},{3,14},{3,50},{4,62},{4,26},{5,36}};
  int maxdur=0; long sumdur=0,nd=0; int sawWin=0,deaths=0,prevLives=r8(A_lives); unsigned maxScore=0; int shots=0;
  sampling=1;
  for(int step=0; step<4000 && !sawWin; step++){          // 20 ms steps = 80 s emulated
    int st=r8(A_state);
    int mask=0;
    if(st==3){
      int x8=(int16_t)r16(A_m); int lev=r8(A_m+7), mode=r8(A_m+8); int c=(x8>>3)+2;
      if(mode==2) mask=K_U;
      else if(lev<6){ int best=-1,bd=999; for(int i=0;i<11;i++) if(LAD[i][0]==lev){ int d=abs(LAD[i][1]-c); if(d<bd){bd=d;best=i;} }
        int cx=LAD[best][1]; mask=(abs(cx-c)<=1)?K_U:((cx>c)?K_R:K_L);
        if(mode==0) for(int i=0;i<8;i++){ unsigned long b=A_bar+i*7; if(r8(b+4)==1 && r8(b+3)==lev){ int bc=(((int16_t)r16(b))>>3)+2; int d=bc-c; int dir=(lev&1)?1:-1; if(d*dir<0 && abs(d)<=7 && abs(d)>=4) mask|=K_A; } }
      } else mask=K_R;
      if(shots<2 && mode==1 && lev==1){ /* captured next frame */ }
    }
    setkeys(mask); runms(20);
    int dur=r16(gb+117); if(st==3){ if(dur>maxdur)maxdur=dur; sumdur+=dur; nd++; }
    if(r8(A_lives)<prevLives){deaths++;prevLives=r8(A_lives);}
    if(r8(A_state)==5) sawWin=1;
    if(r16(A_score)>maxScore) maxScore=r16(A_score);
    if(st==3 && shots==0 && step>400){ shot("play"); shots=1; }
  }
  setkeys(0); runms(300); if(sawWin) shot("win");
  printf("bot: reached Pauline=%d deaths=%d score=%u lvl=%d\n",sawWin,deaths,r16(A_score),r8(A_lvl));
  printf("frame busy time (library getCpuLoad basis): avg %.1f ms, max %.1f ms of 40 ms budget (25 fps)\n",sumdur/(double)nd/1000.0,maxdur/1000.0);
  { FILE*o=fopen("/home/claude/dk/build/prof.txt","w"); for(int i=0;i<32768;i++) if(hist[i]) fprintf(o,"%x %u\n",i*2,hist[i]); fclose(o); }
  printf("stack high-water: SP min=0x%04x -> peak stack use %u bytes (RAMEND 0x08FF)\n",minsp,0x8FF-minsp);
  printf("CPU still running, total emulated %.1f s, LCD frames %ld\n",avr->cycle/16e6,frames);
  return 0; }
