#!/usr/bin/env python3
"""Independent INF parser using the loader's constants; renders icon + slides."""
import sys
from PIL import Image
TOK,NAMEL,IW,IH,SW,SH=5,18,19,18,84,32
VO=TOK+1; NO=VO+1; IO=NO+NAMEL+1; ILEN=((IW+7)//8)*IH; SNO=IO+ILEN; SO=SNO+1; SL=((SW+7)//8)*SH
def parse(p):
    d=open(p,"rb").read(); assert d[:5]==b"gbinf" and d[5]==0, "bad token"
    n=d[SNO]; assert len(d)==SO+SL*n, "size mismatch %d vs %d"%(len(d),SO+SL*n)
    return dict(version=d[VO],name=d[NO:NO+NAMEL+1].split(b"\0")[0].decode(),slides=n,data=d)
def bmp(d,off,w,h,S=4):
    im=Image.new("RGB",(w*S,h*S),(158,190,150)); bw=(w+7)//8
    for y in range(h):
        for x in range(w):
            if d[off+y*bw+x//8]&(0x80>>(x%8)):
                for a in range(S-1):
                    for b in range(S-1): im.putpixel((x*S+a,y*S+b),(25,35,25))
    return im
for p in sys.argv[1:]:
    i=parse(p); print(p,"-> version",i["version"],"name",repr(i["name"]),"slides",i["slides"],"size",len(i["data"]),"OK")
last=parse(sys.argv[-1]); first=parse(sys.argv[1])
sheet=Image.new("RGB",(700,380),(235,235,235))
x=5
for k,i in enumerate((first,last)):
    sheet.paste(bmp(i["data"],IO,IW,IH),(5+k*180,5)); 
    sheet.paste(bmp(i["data"],SO,SW,SH,2),(5+k*180,100))
sheet.save("/tmp/inf_check.png")
