#!/usr/bin/env python3
"""Write BUINOKON.INF in the Gamebuino Classic loader format (verified against
Loader.ino from the Gamebuino library and the official INVADERS.INF):
  'gbinf',0x00 | version 0x01 | name[18]+NUL | icon 19x18 (3 B/row) | nslides |
  slides 84x32 (11 B/row, 352 B each).   Bit 1 = black pixel, MSB = leftmost."""
import sys, struct
def pbm(path):
    t=open(path).read().split(); w,h=int(t[1]),int(t[2]); v=t[3:]
    return [[int(v[y*w+x]) for x in range(w)] for y in range(h)]
def pack(rows,w):
    out=bytearray()
    for r in rows:
        for b in range((w+7)//8):
            v=0
            for k in range(8):
                x=b*8+k
                if x<w and r[x]: v|=0x80>>k
            out.append(v)
    return bytes(out)
HALF=[".....XXXXX","...XXXXXXX","..XXXXXXXX","..XXX.....","..XX..XX..","..XX......",
      "..XXX.XXXX","...XXX....",".XXXXXXXXX","XXXXXXXXXX","XXX.XXXXXX","XXX.XXXXXX",
      "XXXX..XXXX","XXXX..XXXX",".XXX..XXXX","..XX...XXX","..XXX.....",".........."]
def icon():
    rows=[]
    for h in HALF:
        full=h+h[:-1][::-1]; assert len(full)==19
        rows.append([1 if c=='X' else 0 for c in full])
    return pack(rows,19)
def main(out, name, slides):
    nm=name.encode()[:18]; hdr=b"gbinf\x00\x01"+nm+b"\x00"*(19-len(nm))
    data=hdr+icon()+bytes([len(slides)])
    for p,top in slides:
        img=pbm(p)
        for y in range(8):                      # blank the library's battery glyph (emulator artefact)
            for x in range(79,84): img[y][x]=0
        data+=pack(img[top:top+32],84)
    assert len(hdr)==26 and len(data)==81+352*len(slides)
    open(out,"wb").write(data); print(out,len(data),"bytes")
if __name__=="__main__":
    b="/home/claude/dk/build/"
    main(sys.argv[1],"BUINOKONG",[(b+"avr_title.pbm",0),(b+"avr_play.pbm",0),(b+"avr_help.pbm",0)])
