/*
 * RT simulator
 *
 * Jeffrey Kodosky
 * National Instruments
 *
 * ld -X r0.o r1.o r.o  -lc -la
 */

#define RMON    0134000
#define BLK     01000

struct savhdr {
	int     res0[14];
	int     trapv[2];
	char    *pc, *sp;
	int     jsw;
	char    *usr, *end;
	int     err;            /* root size */
	char    *rmonp;         /* stack size */
	int     ovlysize, relid, relblk;
	int     res1[94];
	char    bitmap[16];
	int     res[128];
	};

#define OVLY    01000   /* jsw bits */

extern int errno;

#define EBADF   9

int fd, debug, report;
char lbuf[90];
int swbuf[30];
int ufdes[16] { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
char fcrnam[16][12];    /* unix file name if enter'd; rad50 name at &fcrnam[i][2] if lookup'd */
struct chanstat {
	int     chstatus;
	int     startblk;
	int     length;
	int     used;
	int     devq;
	} chanstat[16];
#define HERR    1       /* channel status bits */
#define ENT     0200
#define EOF     020000
#define ACTIVE  0100000

char *exmsg= "emt %o -- exitting\n";
char *crmsg= "compl. routine -- exitting\n";

main(argc,argv) char **argv;{
	int emt(), iot(), trap();
	register int bit, i, j;
	char *p, *b;

	argc--; argv++;
	if(argc>0 && eqstr(*argv,"-d")){ debug++; argc--; argv++; }
	if(argc!=1 || (fd=open(*argv,0))<0 && (fd=open(sufx(*argv),0))<0 && (fd=open(prefx(*argv),0))<0){
		printf("usage: r [-d] savefile\n"); exit(); }
	p= 0;                   /* load header */
	read(fd,p,BLK);
	if(debug) printf("start pc= %o, sp= %o\n",p->pc,p->sp);
	if(p->end>RMON){
		printf("too big\n"); exit(); }
	if(p->jsw&OVLY){
		ufdes[15]= fd;
		chanstat[15].chstatus= ACTIVE;
		chanstat[15].length= fsize(fd);
		chanstat[15].used= chanstat[15].length; }
	seek(fd,0,0);
	for(i=0, b= &p->bitmap[0]; i<16; i++, b++)
		for(j=0, bit= 0200; j<8; j++, bit=>>1, p=+ BLK)
			if(*b&bit) read(fd,p,BLK);
	if(0->sp==0) 0->sp= 01000;
	0->rmonp= RMON;
	setwin(0,RMON);
	signal(12, &trap);
	signal(7, &emt);
	signal(6, &iot);
	}

#define MAXNM   80
char nbuf[MAXNM] { "/lib/rt/" };

prefx(s) register char *s;{
	register char *t;
	if(*s=='/') return s;
	for(t= &nbuf[8]; t< &nbuf[MAXNM-1]; )
		if((*t++= *s++)==0) break;
	*t= 0;
	return nbuf; }

sufx(s) register char *s;{
	register char *t;
	for(t= &nbuf[8]; t<&nbuf[MAXNM-5]; t++)
		if((*t= *s++)==0) break;
	copystr(".sav",t);
	return &nbuf[8]; }


#define R5      (-2)
#define R4      0
#define R3      1
#define R2      2
#define R1      3
#define R0      4
#define PC      5
#define PS      6

iot(r4,r3,r2,r1,r0,pc,ps,a1,a2,a3) int *pc; char *a1;{

	if(debug) printf("iot at PC= %o with R0= %o\n",pc,r0);
	exit();

	signal(6, &iot); }

trap(r4,r3,r2,r1,r0,pc,ps,a1,a2,a3) int *pc; char *a1;{
	register int e;

	e= *(pc-1)&0377;
	if(debug) printf("trap %o at PC= %o with R0= %o\n", e,pc,r0);
	sstuff(2, &pc, 034);
	ps= a2;
	signal(12, &trap); }

emt(r4,r3,r2,r1,r0,pc,ps,a1,a2,a3,a4) int *pc; char *a1;{
	register int *p, e, i;

	e= *(pc-1)&0377;
	ps=& ~1;
	0->err= 0;
	p= r0;
	if(debug) printf("emt %o at PC= %o with R0= %o\n",e,pc,p);
	switch(e){
		case 0375:      /* r0-> (code<<8 | chan), arg1, arg2, ... */
			if(debug) printf("\tr0-->\t%o\n\t\t%o\n\t\t%o\n\t\t%o\n",
					p[0],p[1],p[2],p[3]);
			e= (p[0]>>8)&0377;
			switch(e){
				case 02: sstuff(1, &a1, &p[2]); /* .enter */
					goto v3map;
				case 010:                       /* .readx */
				case 011:                       /* .writx */
					sstuff(3, &a1, &p[2]);
				case 01:                        /* .lookup */
				case 05:                        /* .savestatus */
				case 06:                        /* .reopen */
				v3map:  e= (p[0] | (p[0]>>4)) & 0377;
					r0= p= p[1];
					goto v1emt;
				case 020: job(p[1]); break;     /* .gtjb */
				case 021: datim(p[1]); break;   /* .gtim */
				case 034: r0= *(int *)(p[1]+RMON); /* .gval */
					break;
				case 03:                        /* .trpset */
				case 035: break;                /* .scca */
				default:  printf("emt375 code 0%o\n",e); exit();
				}
			break;
		case 0374:      /* r0= (code<<8 | chan) */
			e= (r0>>8)&0377;
			switch(e){
				case 0: e= 0240; goto v2map;    /* .wait */
				case 06: e= 0160;               /* .close */
				v2map:  e=| r0&017;
					goto v1emt;
				case 012: r0= datim(0); break;  /* .date */
				default:  printf("emt374 code 0%o\n",e); exit();
				}
			break;
		case 0340:      /* .ttyin */
			r0= getchar();
			break;
		case 0341:      /* .ttyout */
			putchar(p);
			break;
		case 0342:      /* .dstat */
			/* r0-> devname, sp-> statbuf */
			p= a1;
			*p++= 0100000;  /*rk05*/
			*p++= 100;      /*hdlr size*/
			*p++= RMON+0700;/*hdlr load pt*/
			*p++= 4000;     /*no. blocks*/
			sclear(1,&a1);
			break;
		case 0343:      /* .fetch, .release */
			/* r0-> devname, sp-> hdlrloc */
			r0= a1;
			sclear(1, &a1);
			break;
		case 0344:      /* .csigen */
		case 0345:      /* .gtlin, .csispc */
			/* sp-> string, defext, outspec [,linbuf] */
			if(debug) printf("\t%o %o %o %o\n",a1,a2,a3,a4);
			i= a3&1; p= a3&~1;
			if(a1) copystr(a1,lbuf);
			else if(getlin(p? "*\200" : a2)<0) rtexit();
			if(i) copystr(lbuf,a4);
			if(p && parse(a2,p)<0) ps=| 1;
			sclear(i+3,&a1);
			if(p) sstuff(swsize(), &a1, swbuf);
			r0= p;
			if(e==0344){
				report= 1;
				for(i=0; i<9; i++){
					if(ufdes[i]>=0){
						close(ufdes[i]);
						ufdes[i]= -1;
						chanstat[i].chstatus= 0; }
					if(p[0] && (i>=3? lookup(i,p): enter(i,p)) <0)
						ps=| 1;
					p=+ i>=3? 4:5; }
				report= 0; }
			break;
		case 0346:      /* .lock */
		case 0347:      /* .unlock */
			break;
		case 0350:      /* .exit */
			rtexit();
		case 0351:      /* .print */
			prints(p);
			break;
		case 0352:      /* .sreset */
			sreset();
			break;
		case 0353:      /* .qset */
			break;
		case 0354:      /* .settop */
			if(p >= RMON) p= RMON-2;
			0->end= p;
			r0= p;
			break;
		case 0355:      /* .rctrlo */
			break;
		default:if(e<0340)
v1emt:                  switch(e&~017){         /* V1 emts */
			case 020:       /* .lookup */
			case 0140:      /* .reopen */
				/* r0->dblk */
				if(lookup((e=& 017),r0)<0) ps=| 1;
				else r0= fsize(ufdes[e]);
				break;
			case 040:       /* .enter */
				/* r0->dblk, sp->len */
				if(enter(e&017,r0)<0) ps=| 1;
				else r0= 1000;
				sclear(1,&a1);
				break;
			case 0120:      /* .savestatus */
				/* r0->cblk */
				if(ufdes[e=& 017]<0 || fcrnam[e][0]) ps=| 1;
				else {  close(ufdes[e]);
					ufdes[e]= -1;
					chanstat[e].chstatus= 0;
					copy(&fcrnam[e][2],r0,8); }
				break;
			case 0160:      /* .close */
				close(ufdes[e=& 017]);
				ufdes[e]= -1;
				chanstat[e].chstatus= 0;
				perm(&fcrnam[e][0]);
				break;
			case 0200:      /* .readx */
				/* r0=relblk, sp->buf, wdcnt, compl.routine */
				if(debug) printf("\t%d wds at %o\n",a2,a1);
				if(a3 && (a3&1)==0){ printf(crmsg); exit(); }
				seek(ufdes[e=& 017],p,3);
				for(i=a2, p=a1; i-->0; *p++= 0) ;
				if((i=read(ufdes[e],a1,a2*2))<=0){
					if(debug) printf("\terror %d (errno=%d)\n",i,errno);
					r0= 0; ps=| 1;
					chanstat[e].chstatus=| i?HERR:EOF;
					if(i) 0->err= errno==EBADF? 2:1; }
				else {  i= ((i+BLK-1)&~(BLK-1))/2;
					r0= i<a2? i:a2; }
				if(debug) printf("\tret'd %d\n",r0);
				sclear(3, &a1);
				break;
			case 0220:      /* .writx */
				/* r0=relblk, sp->buf, wdcnt, compl.routine */
				if(debug) printf("\t%d wds at %o\n",a2,a1);
				if(a3 && (a3&1)==0){ printf(crmsg); exit(); }
				seek(ufdes[e=& 017],p,3);
				if((i=write(ufdes[e],a1,a2*2))!=a2*2){
					if(debug) printf("\terror %d (errno=%d)\n",i,errno);
					r0= 0; ps=| 1;
					chanstat[e].chstatus=| HERR;
					0->err= errno==EBADF?2:1; }
				else r0= (i+1)/2;
				i= p;
				if((chanstat[e].chstatus&ENT)==0 && (i=+ (r0+255)>>8) > chanstat[e].used)
					chanstat[e].used= i;
				if(debug) printf("\tret'd %d\n",r0);
				sclear(3, &a1);
				break;
			case 0240:      /* .wait */
				if(ufdes[e&017]<0) ps=| 1;
				break;
			case 0:         /* .delete */
			case 0100:      /* .rename */
			default:printf(exmsg,e);
				exit();
			}
			else { printf(exmsg,e); exit(); }
		}
	if(debug) printf("\n");
	signal(7, &emt); }

job(p) register int *p;{
	register int i;
	for(i=0; i<8; i++) p[i]= 0;
	p[1]= RMON-2;
	p[3]= &chanstat[0]; }

datim(lp) register int *lp;{
	static int tv[2];
	register int *ct;
	time(tv);
	ct= localtime(tv);
	if(lp){ for(lp[0]=lp[1]=0; ct[2]>0; ct[2]--) ladd(3,19392,lp);
		for( ; ct[1]>0; ct[1]--) ladd(0,3600,lp);
		ladd(0, ct[0]*60, lp); }
	return ((ct[4]+1)<<10) | (ct[3]<<5) | (ct[5]-72); }

#define R50TT 0100040
#define R50LP 046600
#define R50DK 015270
#define R500  036

lookup(e,p) register int *p;{
	register char *nm;
	nm= ctoa(&p[1]);
	if(debug) printf("opening %s\n",nm);
	errno= 0;
	if(ufdes[e]>=0) goto err;
	copy(p, &fcrnam[e][2],8);
	if(dev(*p,R50LP))
		if((ufdes[e]=open("/dev/lp",1))<0) goto err;
		else goto ok;
	if(dev(*p,R50TT))
		if((ufdes[e]=dup(2))<0) goto err;
		else goto ok;
	if((ufdes[e]=open(nm,2))<0 && (ufdes[e]=open(prefx(nm),0))<0) goto err;
ok:     chanstat[e].chstatus= ACTIVE;
	chanstat[e].length= fsize(ufdes[e]);
	chanstat[e].used= chanstat[e].length;
	return 0;
err:    if(debug || report) printf("can't open %s (errno= %d)\n",nm,errno);
	return -1; }

enter(e,p) register int *p;{
	register char *nm, *s;
	errno= 0;
	nm= ctoa(p+1);
	for(s=nm; *s; s++) ;
	if(s!=nm){ *s++= 'T'; *s= 0; }
	if(ufdes[e]>=0) goto err;
	copystr(nm, &fcrnam[e][0]);
	if(debug) printf("creating %s\n",nm);
	if(dev(*p,R50LP))
		if((ufdes[e]=open("/dev/lp",1))<0) goto err;
		else goto ok;
	if(dev(*p,R50TT))
		if((ufdes[e]=dup(2))<0) goto err;
		else goto ok;
	if(close(creat(nm,0666))<0 || (ufdes[e]=open(nm,2))<0) goto err;
ok:     chanstat[e].chstatus= ACTIVE|ENT;
	chanstat[e].length= 1000;
	chanstat[e].used= 0;
	return 0;
err:    if(debug || report) printf("can't create %s (errno= %d)\n",nm,errno);
	fcrnam[e][0]= 0;
	return -1; }

dev(d,r) register unsigned d, r;{
	if(d==r || d>=r+R500 && d<r+R500+8) return 1;
	return 0; }

perm(nm) char *nm;{
	register char *t, *s;
	static char tmp[12];
	for(t=tmp, s=nm; *t= *s++; t++) ;
	if(t>tmp && *--t=='T'){
		*t= 0;
		unlink(tmp);
		link(nm,tmp);
		unlink(nm); }
	nm[0]= 0; }

rtexit(){
	sreset();
	exit(); }

sreset(){
	register int i;
	for(i=0; i<15; i++){
		close(ufdes[i]);
		ufdes[i]= -1;
		chanstat[i].chstatus= 0;
		if(fcrnam[i][0]) unlink(&fcrnam[i][0]);
		fcrnam[i][0]= 0;
	}       }

fsize(fd){
	static char st[36];
	fstat(fd,st);
	st[10]= st[11]; st[11]= st[9];
	return *((int *)&st[10])/2; }

prints(s) char *s;{
	while(*s&0177) putchar(*s++);
	if(!*s) putchar ('\n');
	}

getlin(p) char *p;{
	register char *s;
	prints(p);
	for(s=lbuf; (*s=getchar())!='\n'; s++)
		if(*s==0) return -1;
	if(s== &lbuf[1] && lbuf[0]=='~') return -1;
	*s= 0;
	return (s-lbuf); }

swsize(){
	register int i, *p;
	for(p= &swbuf[1], i=swbuf[0]; i; i--)
		if(*p++<0) p++;
	return p-swbuf; }

char iside, state, *str;
char nxt[11][8] {
	0,      1,      2,      023,    023,    -1,     -1,     0147,
	8,      8,      8,      0105,   0105,   0164,   046,    0147,
	8,      8,      8,      063,    063,    -1,     -1,     0147,
	8,      8,      8,      0105,   0105,   0164,   -1,     0147,
	8,      8,      8,      -1,     -1,     0164,   -1,     0147,
	-1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
	8,      8,      8,      -1,     -1,     0164,   -1,     -1,
	-1,     -1,     -1,     0211,   -1,     -1,     -1,     -1,
	8,      8,      8,      -1,     -1,     0164,   0232,   -1,
	-1,     -1,     -1,     0253,   0254,   -1,     -1,     -1,
	8,      8,      8,      -1,     -1,     0164,   0235,   -1
	};
parse(defext,ospec) int *defext, *ospec;{
	register int *sp, *sw, fnum;
	for(sp=ospec; sp< &ospec[39]; ) *sp++= 0;
	iside= index('=',lbuf)<0;
	state= 0;
	str= lbuf;
	sp= iside? &ospec[15] : &ospec[0];
	sw= swbuf;
	*sw++= 0;
	fnum= iside? 3:0;
	for(;;) switch(move()){
			case 0: return 0;
			case 1: if(!iside && fnum>=3 || fnum>=9) return -1;
				fnum++;
				sp=+ 5-iside;
				str++;
				break;
			case 2: if(iside++ || fnum>=3) return -1;
				fnum= 3;
				sp= &ospec[15];
				str++;
				break;
			case 3: if(sp[0]==0) sp[0]= R50DK;
				sp[1]= getr50();
				sp[3]= defext[fnum>=3? 0:fnum+1];
				break;
			case 4: str++;
				break;
			case 5: sp[2]= getr50();
				break;
			case 6: sp[0]= sp[1];
				sp[1]= 0;
				str++;
				break;
			case 7: str++;
				sp[3]= getr50();
				break;
			case 8: break;
			case 9: *sw++= (*str++&~040) | (fnum<<8);
				swbuf[0]=+ 1;
				break;
			case 10:sw[-1]=| 0100000;
				str++;
				break;
			case 11:*sw++= getr50();
				break;
			case 12:*sw++= getnum();
				break;
			case 13:*sw= sw[-2];
				sw++;
				swbuf[0]=+ 1;
				str++;
				break;
			default:return -1;
	}               }

move(){ register int os, cl;
	os= state;
	if((cl=class(*str))<0) return -1;
	state= (nxt[os][cl]>>4) &017;
	return (nxt[os][cl] &017); }

char chr[]= " abcdefghijklmnopqrstuvwxyz$.?0123456789";
getr50(){
	register int i, c, r;
	for(i=0, r=0; i<3; i++, r= r*050 + c)
		if((c=class(*str))==3 || c==4)
			c= index(*str++|040, chr);
		else c= 0;
	return r; }

getnum(){
	register char *s;
	register int base, i;
	for(s=str; class(*str)==4; str++);
	if(*str=='.'){ base= 10; str++; }
	else base= 8;
	for(i=0; class(*s)==4; s++)
		i= i*base + *s-'0';
	return i; }

class(c){
	switch(c){
		case 0: return 0;
		case ',': return 1;
		case '=': return 2;
		case '/': return 5;
		case ':': return 6;
		case '.': return 7;
		default:if(c>='A' && c<='Z') return 3;
			if(c>='a' && c<='z') return 3;
			if(c>='0' && c<='9') return 4;
			return -1;
	}       }

ctoa(r) register int *r;{
	static char name[12];
	register char *s;
	r50toa(r[0],&name[0]);
	r50toa(r[1],&name[3]);
	for(s= &name[6]; s>name && *(s-1)==' '; s--) ;
	if(r[2]){
		*s++= '.';
		r50toa(r[2],s);
		for(s=+ 3; *(s-1)==' '; s--) ; }
	*s= 0;
	return name; }

r50toa(r,s) register unsigned r; register char *s;{
	register int i;
	for(i=3, s=+ 3; i>0; i--, r=/ 050)
		*--s= chr[r%050];
	}
