/*
 * Rtpip: access an RT filesystem
 *
 * Jeffrey Kodosky
 * National Instruments
 *
 * usage:       rtpip -[xrldis[aev]] dev [file ...]
 *
 *      -x      extract named files from RT device, dev.  File
 *              args may use standard shell glob syntax (escaped
 *              so that the shell doesn't interpret it).  If no
 *              file args given, all files are extracted.
 *      -r      replace the named files on the RT device, dev.
 *      -l      list directory.
 *      -le     list directory and empty slots.
 *      -d      delete named files.
 *      -i      init RT filesystem: 1-st file arg is filesystem size in
 *              blocks (must be >=16); optional 2-nd file arg is number
 *              of directory segments to make (must be <= 31; if blank
 *              filesystem size/150 is used).
 *      -s      squeeze RT files to consolidate free space.
 *
 *           a  ASCII: with -x and -r the carriage returns are deleted or
 *              inserted as necessary and nulls are ignored.
 *           v  verbose: with -l the directory includes file length,
 *              starting block, creation date, and directory segment.
 */

struct dhdr {
	int     max, nxt, use, exb, bno; };

struct entry {
	int     stat, name[3], blen, job, date; };

#define DSIZE   1024
#define DHSIZ   sizeof(struct dhdr)
#define ESIZE   sizeof(struct entry)

#define TMP     0400
#define MTY     01000
#define PRM     02000
#define EOS     04000

char    *qual, **file;
int     nfile, dev, verbose, ascii, freeb;
int     lastseg, esize;
char    dirseg[DSIZE];

char    *lhdr=  "Name      Length      Date    Block (Seg)\n\n";
char    *lfmt=  "%-10s%6d  %11s%6d  (%d)\n";


main(argc,argv) char **argv;{
	int list(), extract(), delete();
	register int i, c;

	if(argc<3 || argv[1][0]!='-') goto bad;
	c= argv[1][1];
	if((dev=open(argv[2],(index(c,"isrd")>=0)?2:0))<0){
		printf("can't open %s\n",argv[2]);
		goto bad; }
	qual= &argv[1][2];
	verbose= index('v',qual)>=0;
	ascii= index('a',qual)>=0;
	nfile= argc-3;
	file= &argv[3];
	if(index(c,"lxd")>=0)
		for(i=0; i<nfile; i++)
			file[i]= mkpatt(globcvt(file[i]),0);
	switch(c){
		case 'i':
			if(nfile<=0 || (i=atoi(file[0]))<16){
				printf("1-st file is filesystem size?\n");
				goto bad; }
			rtinit(i);
			break;
		case 'l': if(verbose) printf(lhdr);
			freeb= 0;
			dscan(&list,1);
			printf("\n%d free blocks\n",freeb);
			break;
		case 'x': dscan(&extract,1); break;
		case 'r':
			if(nfile<=0){ printf("files?\n"); goto bad; }
			for(i=0; i<nfile; i++)
				replace(file[i]);
			break;
		case 'd':
			if(nfile<=0){ printf("files?\n"); goto bad; }
			dscan(&delete,0); break;
		case 's': squeeze(); break;
		default:printf("bad switch\n");
		bad:    printf("usage: rtpip -[xrldis[aev]] dev [file ...]\n");
			exit();
	}       }

rtinit(n){
	register char *d, *e;
	register int i;
	if(nfile<=1 || (i=atoi(file[1]))<=0) i= n/150;
	if(6+2*i>=n) i= (n-7)/2;
	if(i<=0) i= 1;
	else if(i>31) i= 31;
	d= dirseg;
	e= d+DHSIZ;
	d->max= i;
	d->nxt= 0;
	d->use= 1;
	d->exb= 0;
	d->bno= 6+ 2*i;
	e->stat= MTY;
	e->blen= n - (6+ 2*i);
	(e+ESIZE)->stat= EOS;
	putseg(1);
	seek(dev,n-1,3);
	write(dev, &dirseg[512], 512);
	if(verbose)
		printf("init: %d blocks total, %d directory segments, %d blocks usable\n",
			n,i,e->blen);
	}

list(e,bno,d,n) char *d, *e;{
	char *nm;
	if(e->stat&PRM){
		nm= ctoa(&e->name[0]);
		if(!ok(nm)) return 0; }
	else {  freeb=+ e->blen;
		if(index('e',qual)<0) return 0;
		e->date= 0;
		nm= "<empty>"; }
	if(verbose) printf(lfmt, nm, e->blen, cdate(e->date), bno, n);
	else printf("%s\n",nm);
	return 0; }

extract(e,bno,d,n) char *e, *d;{
	char *nm, buf[512];
	int fd;
	register char *s, *t;
	register int i;

	if((e->stat&PRM)==0) return 0;
	nm= ctoa(&e->name[0]);
	if(!ok(nm)) return 0;
	if((fd= creat(nm,0666))<0){
		printf("can't create %s\n",nm);
		return -1; }
	if(verbose) printf("%s\n",nm);
	seek(dev,bno,3);
	for(i=e->blen; i-->0 && read(dev,buf,512)>0; )
		if(ascii){
			for(s=t=buf; s< &buf[512]; )
				if((*t= *s++) && *t!='\r') t++;
			write(fd,buf,t-buf); }
		else write(fd,buf,512);
	close(fd);
	return 0; }

delete(es,bno,d,n) char *es, *d;{
	register char *e, *ep;
	char *nm;

	for(e=es; e<d+DSIZE && (e->stat&EOS)==0; e=+ esize)
		if((e->stat&PRM) && ok(nm=ctoa(&e->name[0]))){
			e->stat= MTY;
			if(verbose) printf("%s\n",nm); }
	consolidate(d);
	return putseg(n); }

consolidate(d) char *d;{
	register char *e, *ep;

	for(e=d+DHSIZ; e<d+DSIZE && (e->stat&EOS)==0; e=+ esize)
		if((e->stat&PRM)==0){
			for(ep= e+esize; ep<d+DSIZE && (ep->stat&(PRM|EOS))==0; ep=+ esize)
				e->blen=+ ep->blen;
			copy(ep,e+esize,d+DSIZE-ep);
	}               }

squeeze(){
	int bmty, bno;
	register char *e, *d;
	register int n;
	for(bno=0, bmty=0, n=1; n>0 && (d=getseg(n)); n= d->nxt){
		if(bno==0) bno= 6+ 2*d->max;
		if(verbose) printf("seg %d: old start %d, new start %d\n",n,d->bno,bno);
		if(bmty!=d->bno-bno) printf("dir err seg %d start blk\n",n);
		d->bno= bno;
		for(e= d+DHSIZ; e<d+DSIZE && (e->stat&EOS)==0; e=+ esize){
			if((e->stat&PRM)==0) bmty=+ compress(e,bno,d,n,bmty);
			else scoot(bno,bmty,e->blen, &e->name[0]);
			if(e->stat&EOS) break;
			bno=+ e->blen; }
		if(d->nxt==0) break;
		putseg(n); }
	if(d){  if(e+esize+2 > d+DSIZE){
			if((n=newseg(d,n))==0 || (d=getseg(n))==0) return;
			d->bno= bno;
			e= d+DHSIZ; }
		e->stat= MTY;
		e->blen= bmty;
		e=+ esize;
		e->stat= EOS;
		putseg(n);
	}       }

compress(e,bno,d,n,bmty) char *d, *e;{
	register int bofst;
	register char *ep;
	for(bofst=0, ep=e; (ep->stat&(PRM|EOS))==0; bofst=+ ep->blen, ep=+ esize) ;
	copy(ep,e,d+DSIZE -ep);
	if((ep=e)->stat&PRM) scoot(bno,bofst+bmty,ep->blen, &ep->name[0]);
	return bofst; }

scoot(bno,bofst,n,name) char *name;{
	char buf[512];
	register int i;
	if(bofst==0) return;
	if(verbose) printf("\tmove %s up %d blocks\n", ctoa(name), bofst);
	for(i=0; i<n; i++){
		seek(dev,i+bno+bofst,3);
		read(dev,buf,512);
		seek(dev,i+bno,3);
		write(dev,buf,512);
	}       }

dscan(f,byentry) int (*f)();{
	register char *e, *d;
	register int bno;
	int n, x;

	for(n=1, bno=0; n>0 && (d=getseg(n)); n= d->nxt){
		if(bno==0) bno= 6+ 2*d->max;
		if(bno!=d->bno) printf("dir err seg %d start blk\n",n);
		bno= d->bno;
		if(byentry==0){
			if(x= (*f)(d+DHSIZ,bno,d,n)) return x;
			bno=+ dlen(d); }
		else for(e= d+DHSIZ; e<d+DSIZE && (e->stat&EOS)==0; e=+ esize){
				if(x= (*f)(e,bno,d,n)) return x;
				bno=+ e->blen;
		}               }
	return 0; }

ok(s) char *s;{
	register int i;
	if(nfile==0) return 1;
	for(i=0; i<nfile; i++)
		if(match(file[i],s,0)) return 1;
	return 0; }

ctoa(r) int r[3];{
	static char name[12];
	char *s;
	r50toa(r[0],&name[0]);
	r50toa(r[1],&name[3]);
	for(s= &name[6]; s>name && *(s-1)==' '; s--) ;
	if(r[2]){
		*s++= '.';
		r50toa(r[2],s);
		for(s=+ 3; *(s-1)==' '; s--) ; }
	*s= 0;
	return name; }

char chr[]= " abcdefghijklmnopqrstuvwxyz$.?0123456789";

r50toa(r,s) unsigned r; char *s;{
	int i;
	for(i=3, s=+ 3; i>0; i--, r=/ 050)
		*--s= chr[r%050];
	}

ctor(s) register char *s;{
	static int nm[3];
	char buf[9];
	register int i;
	register char *t;
	for(t= &s[strlen(s)]; t>s && *(t-1)!='/'; t--) ;
	for(s=t; *s && *s!='.'; s++) ;
	for(i=0; i<6; i++)
		buf[i]= t<s? (*t++|040) : ' ';
	if(*s=='.') s++;
	for(; i<9; i++)
		buf[i]= *s? (*s++|040) : ' ';
	nm[0]= ator50(&buf[0]);
	nm[1]= ator50(&buf[3]);
	nm[2]= ator50(&buf[6]);
	return nm; }

ator50(s) register char *s;{
	register int i, j;
	unsigned r;
	for(i=0, r=0; i<3; r= r*050 + j)
		if((j=index(s[i++],chr))<0) j=index('?',chr);
	return r; }

cdate(d){
	static char dstr[32];
	register char *s, *t;
	register int i;

	t= dstr;
	i= (d>>10)&017;
	for(s= &"   JanFebMarAprMayJunJulAugSepOctNovDec"[3*i], i=3; i-->0; )
		*t++= *s++;
	*t++= ' ';
	i= (d>>5)&037;
	*t++= i>=10? i/10 + '0': ' ';
	*t++= i>0? i%10 + '0': ' ';
	*t++= ' ';
	i= d&037;
	if(d){ *t++= '1'; *t++= '9'; *t++= (i+72)/10 + '0'; *t++= (i+72)%10 + '0'; }
	else for(i=0; i<4; i++) *t++= ' ';
	*t= 0;
	return dstr; }

datec(){ int t[2], *l;
	time(t);
	l= localtime(t);
	return ((l[4]+1)<<10) | (l[3]<<5) | (l[5]-72); }

dlen(d) char *d;{
	register char *e;
	register int i;

	for(i=0, e=d+DHSIZ; e<d+DSIZE && (e->stat&EOS)==0; e=+ esize)
		i=+ e->blen;
	return i; }


int     srchlen, *srchnm;
int     sseg[2], sent[2], sbno[2], slen[2];

replace(s) char *s;{
	register int i;
	register char *t, *e;
	int n, fd, locate();
	long len;
	char buf[1024];
	if((fd=open(s,0))<0){
		printf("can't open %s\n", s); return -1; }
	if(ascii){
		for(len=0; (i=read(fd,buf,1024))>0; len=+ i)
			for(t= &buf[i]; t>buf; )
				if(*--t=='\n') len++;
		seek(fd,0,0); }
	else {  fstat(fd,buf);
		buf[8]= buf[9]; buf[9]= 0;
		len= *((long *)(&buf[8])); }
	srchlen= (len+511)>>9;
	srchnm= ctor(s);
	sseg[0]= 0; sseg[1]= 0; sbno[0]= 0;
	dscan(&locate,1);
	if(sseg[1]==0){
		printf("no room for %s\n",s); return -1; }
	if(verbose) printf("%s%s",s,sbno[0]?(sseg[0]?" (removing old)\n":" (overwriting old)\n"):"\n");
	seek(dev,sbno[1],3);
	for(n=srchlen; n-->0 && (i=read(fd,buf,512))>0; )
		if(ascii){
			for(t= &buf[1024]; i>0; )
				if((*--t= buf[--i])=='\n') *--t= '\r';
			bwrite(dev,t, &buf[1024] -t); }
		else write(dev,buf,i);
	if(ascii) bwrite(dev,0,0);
	if(sseg[0]){
		if((t=getseg(sseg[0]))==0) goto out;
		e= t+sent[0];
		e->stat= MTY;
		if(sseg[0]!=sseg[1]){
			consolidate(t);
			putseg(sseg[0]);
			if((t=getseg(sseg[1]))==0) goto out;
		}       }
	else if((t=getseg(sseg[1]))==0) goto out;
	mkent(t+sent[1],sbno[1],t,sseg[1]);
out:    close(fd); }

bwrite(fd,b,n) char *b;{
	static char buf[512];
	static int next= 0;
	while(n-->0){
		buf[next++]= *b++;
		if(next>=512){ write(fd,buf,512); next= 0;
		}       }
	if(b==0 && next>0){
		while(next<512) buf[next++]= 0;
		write(fd,buf,512); next= 0;
	}       }

mkent(e,bno,d,n) char *d, *e;{
	register char *ep;
	register int i, mt;

	ep= e;
	mt= ep->blen - srchlen;
	ep->stat= PRM;
	for(i=0; i<3; i++) ep->name[i]= srchnm[i];
	ep->date= datec();
	ep=+ esize;
	if(mt>0 && (ep->stat&(PRM|EOS)) && insert(ep,d)==0){
		if(split(&e,&d,&n)==0) return;
		ep= e+esize;
		insert(ep,d); }
	ep->blen=+ mt;
	e->blen=- mt;
	putseg(n); }

insert(e,d) char *e, *d;{
	register char *ep, *tp;

	for(ep=e; ep<d+DSIZE && (ep->stat&EOS)==0; ep=+ esize) ;
	if(ep>=d+DSIZE-esize) return 0;
	ep=+ 2;
	for(tp=ep+esize; ep>e; ) *--tp= *--ep;
	ep->stat= MTY;
	ep->blen= 0;
	return 1; }

split(ep,dp,np) char **ep, **dp; int *np;{
	register char *e, *d;
	register int x;
	int n, bno, nxt, ofst;

	e= *ep; d= *dp; n= *np;
	nxt= d->nxt;
	ofst= e-d;
	if(newseg(d,n)==0 || (d=getseg(n))==0) return 0;
	for(bno= d->bno, e= d+DHSIZ; e<d+DSIZE/2; e=+ esize)
		bno=+ e->blen;
	x= e->stat;
	e->stat= EOS;
	putseg(n);
	e->stat= x;
	n= d->nxt;
	d->nxt= nxt;
	d->bno= bno;
	copy(e, d+DHSIZ, d+DSIZE-e);
	if(e-d>=ofst){
		*ep= d +ofst -(e-d) +DHSIZ;
		*dp= d;
		*np= n; }
	else {  putseg(n);
		if((d=getseg(n= *np))==0) return 0;
		*ep= d +ofst;
		*dp= d; }
	return 1; }

locate(e,bno,d,n) char *d, *e;{
	register int i;
	if(e->stat&PRM){
		for(i=0; i<3; i++)
			if(e->name[i]!=srchnm[i]) return 0;
		sseg[0]= n; sent[0]= e-d; sbno[0]= bno; slen[0]= e->blen;
		if(e->blen<srchlen) return 0;
		sseg[1]= sseg[0]; sent[1]= sent[0];     /* stop looking if old */
		sbno[1]= sbno[0]; slen[1]= slen[0];     /* file >= new one     */
		sseg[0]= 0;
		return 1; }
	if(sseg[1]==0 && e->blen>=srchlen){
		sseg[1]= n; sent[1]= e-d; sbno[1]= bno; slen[1]= e->blen; }
	return 0; }

newseg(d,n) char *d;{
	register int nxt;

	if(lastseg>=d->max) goto out;
	lastseg++;
	nxt= d->nxt;
	d->nxt= lastseg;
	if(putseg(n)>=0){
		d->nxt= nxt;
		(d+DHSIZ)->stat= EOS;
		if(putseg(lastseg)>=0 && (d=getseg(1))){
			d->use= lastseg;
			if(putseg(1)>=0) return lastseg;
		}       }
out:    printf("can't make new dir seg\n");
	return 0; }

#define MAXPATT 128
globcvt(s) register char *s;{
	static char buf[MAXPATT];
	register char *p, c;

	p= buf;
	for(*p++= '^'; p<&buf[MAXPATT-2] && (c= *s++); *p++= c)
		switch(c){
			case '*': if(p<&buf[MAXPATT-3])
					*p++= '.';
				break;
			case '?': c= '.'; break;
			case '\\': if((c= *s++)=='?') break;
				if(c==0) s--;
			case '.': if(p<&buf[MAXPATT-3])
					*p++= '\\';
				break;
			case '[':
				for(*p++= c; p<&buf[MAXPATT-2] && (c= *s++); *p++= c)
					if(c==']') break;
				if(c==0) s--;
				break;
			}
	*p++= '$';
	*p= 0;
	return buf; }

getseg(n){
	extern int errno;
	if(seek(dev, (n-1)*2 +6, 3)<0 || read(dev,dirseg,DSIZE)!=DSIZE){
		printf("dir %d rd err (%d)\n",n,errno); return 0; }
	if(n==1){ lastseg= dirseg->use; esize= ESIZE + dirseg->exb; }
	return dirseg; }
putseg(n){
	extern int errno;
	if(seek(dev, (n-1)*2 +6, 3)<0 || write(dev,dirseg,DSIZE)!=DSIZE){
		printf("dir %d wt err (%d)\n",n,errno); return -1; }
	return 0; }


