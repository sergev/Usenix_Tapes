#include <stdio.h>
#define d define
#d G getchar()
#d I if
#d H(n) ((C*)h)[n]
#d C char
#d U unsigned long
#d u unsigned short
#d R register C*
#d B(n) (1<<(n))
#d V(a,b) (a&(B(b)-1))
#d E {fprintf(stderr,"Bad COMPRESS file format\n");exit(1);}
int i,q,b,n,k=16,e=128,j=0,O=0,S=0;u t[69001];long o,v,c,m,M=B(16),f=0;U h[
69001];C D[16];long g(){R p=D;I(j>0||O>=S||f>m){I(f>m)m= ++n==k?M:B(n)-1;I(j>0)
m=B(n=9)-1,j=0;S=fread(D,1,n,stdin);I(S<=0)return-1;O=0;S=(S<<3)-n+1;}q=O;b=n;p
+=q>>3;q&=7;c=V(*p++>>q,8-q);q=8-q;b-=q;I(b>=8)c|=(*p++&255)<<q,q+=8,b-=8;c|=V(
*p,b)<<q;O+=n;return c;}main(){I(G!=31||G!=157)E;k=G;e=k&128;k&=31;M=B(k);I(k>
16)E;z();exit(0);}z(){long w;R s;m=B(n=9)-1;for(w=255;w>=0;w--)t[w]=0,H(w)=(C)w
;f=e?257:256;i=o=g();I(o==-1)return;putchar((C)i);s=(C*)&H(B(16));while((w=g())
>-1){I(w==256&&e){for(w=255;w>=0;w--)t[w]=0;j=1;f=256;I((w=g())==-1)break;}v=w;
I(w>=f)*s++=i,w=o;while((U)w>=(U)256)*s++=H(w),w=t[w];*s++=i=H(w);do putchar(*
--s);while(s>&H(B(16)));I((w=f)<M)t[w]=(u)o,H(w)=i,f=w+1;o=v;}}
