#
/*
 * converts standard vector plot data into raster lines
 *
 * Author- Rob Clayton, Dept. of Geophysics, Stanford Univ.
 */

/* Size parameters
 *	To set, fix NYW and NBLOCK, then:
 *		NXW=	(NBLOCK-2)*256/NYW
 *		NSEC=	NBLOCK
 *		YMAX=	NYW*16 -1
 *		XMAX=	(NXW-1)*NSEC -1
 */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#define NBLOCK 52	/* memory size in blocks of 512 bytes */
#define NXW 100		/* raster lines per sector */
#define NYW 128		/* width of raster lines in words */
#define NSEC 52		/* number of sectors */
#define XMAX 5147	/* maximum x value */
#define XMIN 0		/* minimum x value */
#define YMAX 2047	/* maximum y value */
#define YMIN 0		/* minimum y value */
#define INCHDOT	0.005	/* inches per raster dot */
#define STDSIZ	16384	/* # of pixels in standard size plot */
#define STDSCL	8	/* = STDSIZ/(YMAX+1) */
#define STDCW	5	/* standard text size */
#define TWMUL	64	/* # of pixels in standard text width size spec. */
#define THMUL	96	/* 3/2*TWMUL */
#define PLTCMD	0200	/* flag bit for plot commands */
#define CMDMSK	0177	/* mask for value of plot commands */

#define NERASE 100	/* number of blank line ejects on erase */
#define STATOFF 2	/* static offset of plot in words */

#define ENDSEC -1	/* end of sector flag */
#define ENDRAS -2	/* end of raster data flag */
#define BEGRAS -3	/* beginning of raster data flag */

#define PLOTTER "/dev/mp"	/* plotter name */
#define ATTEMPT 60	/* number of attempts at opening plotter */
#define SLEEPTIME 30	/* sleep time between attempted openings */
#define TSIZE 1024	/* buffer size for moving data to plotter */

int mem[ NBLOCK*256 ];	/* main memory for sector buffers and rasterizing */
int grkflag;		/* flag to return ascii chars */
int *secbase &mem[512];	/* sector starting position pointer */
int tbuf[256];		/* read back from temp file buffer */
int *wrtptr;		/* write pointer for sector packing */
int head[NSEC];		/* pointers to first disk block in each sector */
int nextav;		/* next available disk block */
int diskr, diskw;	/* temp file read-write descriptors */
int plotr, plotw;	/* plot file read-write descriptors */
int newplot;		/* new plot flag */
int nodd 0;		/* counter non-blocked part of plotfile writes */
int dotsize;		/* # of standard pixels per dot in raster mode */
unsigned tsize,tangle;	/* text size and angle */
char tfont;		/* font type (in future) */
int fatbase,fat;	/* line thickness parameters */
int xmax 4095;		/* plot window parameters defaulted */
int xmin XMIN;		/* to maximum size 		*/
int ymax YMAX;
int ymin YMIN;
int xnew,ynew;		/* new pen location */
int xold,yold;		/* old pen location */
int xorigin,yorigin;	/* origin for text mode */
int scale;		/* current scaling factor */
struct secbuf	/* sector buffer structure */
   {
	int buf[252];	/* buffer */
	int link;	/* forward link to next block */
	int assign;	/* current block assignment */
	int nused;	/* number of buffer slots used */
	int junk;	/* not used */
   };
char inbuf[BUFSIZ];	/* input buffer -used by getc/w */
int erase;		/* erase flag */
int purge,npurge;	/* purge plotfile counters */
int tmpflag 0;		/* temp file exists flag */
int pltflag 0;		/* plot file exists flag */
int plotit;		/* make plot flag */
int plotter;
char tmpdir[]	"/tmp/mp";	/* generic leader for temp files */
char tmp[25];		/* temp file name */
char plt[25];		/* plot file name */
FILE *pltin, *prinout;
struct inode
  {	int dev;
	int inum;
	int flags;
  };



main(argc,argv)
int argc; char **argv;
   {
	register char *cptr;
	int attempt,fileflag,i;
	int pltend();
	double fscale, atof();
	FILE *freopen(), *temp;
	if((signal(2,1)&1)==0) signal(2,&pltend);
	/*
	 * open temporary file
	 */
	tmpflag = 1;
	tempname(tmpdir,tmp);
	diskw= creat(tmp,0600);
	diskr= open(tmp,0);
	if( (diskw<0) || (diskr<0) )
		error("cannot creat temp file");
	/*
	 * open plot file
	 */
	fstat(1, tbuf);
	fstat(2, inbuf);
	if (tbuf->st_ino != inbuf->st_ino)	/* stdout redirected */
	   {
		plotw=1;
		plotit=0;
		prinout = stderr;
	   }
	 else
	   {
		pltflag = 1;
		tempname(tmpdir,plt);
		plotw= creat(plt,0600);
		plotr= open(plt,0);
		if( (plotw<0) || (plotr<0) )
			error("cannot create plotfile");
		plotit=1;
		prinout = stdout;
	   }

	pltin= stdin;
	setbuf(pltin,inbuf);
	purge= npurge= fileflag= 0;
	erase= -1;
	scale= STDSCL;
	do
	   {
		argc--; argv++;
		cptr= *argv;
		if( argc && (*cptr == '-') )
		   {
			cptr++;
			switch(*cptr)
			   {
				case 'p':
					if (*++cptr == '\0')
						npurge = -1;
					else
					  {	npurge=atoi(cptr);
						if(npurge<0) npurge=0;
					  }
					break;
				case 'f':
					fatbase=atoi(++cptr);
					if(fatbase<0) fatbase=0;
					fat= fatbase;
					break;
				default:	/* assume number */
					fscale= atof(cptr);
					if(fscale<=0.0)
					   {
						scale=STDSCL; break;
					   }
					while(*cptr) cptr++;
					switch(cptr[-1])
					   {
						case 'i':
						default:
							scale=STDSIZ*INCHDOT/fscale;
							break;
						case 'c':
							scale=STDSIZ*INCHDOT/(fscale/2.54);
							break;
					   }
					break;
			   }
			continue;
		   }
		if(argc)		/* file name arg */
			if((temp=freopen(cptr,"r",pltin)) == NULL)
			   {
				fprintf(stderr,"mp: cannot open %s\n",cptr);
				continue;
			   }
			  else	pltin= temp;
		/*
	 	* main loop
	 	*/
		newplot= 1;
		do
		   {
			pass1();
			sflush();
			pass2();
			if( (purge==npurge) && plotit ) cat();
		   } while(newplot);
	   } while(argc);

	if(tmpflag)
	   {
		unlink(tmp);
		tmpflag=0;
		close(diskr);
		close(diskw);
	   }
	if( plotit && (purge!=npurge) ) cat();
	pltend();
   }

pltend()
/*
 * destroy tmp and plt files
 */
   {
	if(tmpflag) unlink(tmp);
	if(pltflag) unlink(plt);
	close(plotter);
	exit();
   }


error(s)
char *s;
   {
	fprintf(stderr,"mp: %s\n",s);
	pltend();
   }



tempname(leader,tname)
register char *tname, *leader;
/*
 * generate unique name beginning with leader,
 * and place in tname.
 */
   {
	static seed ;
	while( *tname++ = *leader++ );
	tname[-1]= '.';
	convert( getpid(), tname);
	while( *tname++ );
	tname[-1]= '.';
	convert( seed++, tname);
   }
convert(num,s)
register int num;
register char *s;
   {
	register int div;
	int temp,zeropad;
	if( num == 0 ) *s++ = '0';
	if(num < 0)
	   {
		*s++= '-';
		num= -num;
	   }
	for(div=10000, zeropad=0; div; div=/ 10)
		if( temp= num/div )
		   {
			*s++= (temp + '0');
			num=% div;
			zeropad=1;
		   }
		 else if(zeropad) *s++= '0';
	*s= 0;
   }


cat()
/*
 * move plot to plotter
 */
   {
	register int attempt,nread;
	attempt=ATTEMPT;
	while(((plotter=open(PLOTTER,1))<0)&&(attempt--))
		sleep(SLEEPTIME);
	if(attempt) /* move plot to plotter */
		do
		   {
			nread=read(plotr,mem,TSIZE);
			if(nread <= 0) break;
			write(plotter,mem,nread);
		   } while(nread==TSIZE);
	 else error("cannot open the plotter");
	purge=0;
	/*
	* truncate plot file to zero length
	*/
	close(plotw);
	close(plotr);
	plotw= creat(plt,0600);
	plotr= open(plt,0);
	if( (plotw<0) || (plotr<0) )
		error("cannot reopen temp file");
	close(plotter);
	nodd=0;
   }



pass1()
/*
 * read in plot commands and sector them
 */
   {
	register int i;
	register int c;
	register struct secbuf *psec;
	psec=mem;
	for(i=0;i<NSEC;i++)
	   {
		head[i]= -1;
		(psec++)->nused=0;
	   }
	nextav=0;
	while((c=getc(pltin))!= EOF)
	   {
		if ((c&PLTCMD) == 0 || ((c&CMDMSK)<'a' || (c&CMDMSK)>'z'))
		   {
			do
				putc(c,prinout);
			while(((c=getc(pltin))&PLTCMD)==0);
			if( c == EOF ) break;
			ungetc(c,pltin);
			continue;
		   }
		switch(c&CMDMSK)	/* c is lower-case letter with PLTCMD on */
		   {
			case 'm':		/* move */
				if (xmax > 4095)
					xold= xorigin= ((unsigned) getw(pltin))/scale;
				else
					xold= xorigin= getw(pltin)/scale;
				yold= yorigin= getw(pltin)/scale;
				break;
			case 'd':		/* draw */
				if (xmax > 4095)
					xnew = ((unsigned) getw(pltin))/scale;
				else
					xnew = getw(pltin)/scale;
				ynew=getw(pltin)/scale;
				if(fat)
				   {
					if(abs(xnew-xold) >= abs(ynew-yold))
						for(i= -(fat/2);i<=(fat+1)/2;i++)
						  dump(xold,yold+i,xnew,ynew+i);
					  else
						for(i= -(fat/2);i<=(fat+1)/2;i++)
						  dump(xold+i,yold,xnew+i,ynew);
				   }
				  else  dump(xold,yold,xnew,ynew);
				xold=xnew;
				yold=ynew;
				break;
			case 's':	/* set angle and size for text */
				tsize= getc(pltin)&0377;
				tfont= getc(pltin)&0377;
				tangle= getw(pltin);
				break;
			case 't':		/* text */
				putsym();
				break;
			case 'b':		/* break */
				if(erase == -1) /* ignore first 'e' or 'b' */
				   {
					erase=0;
					break;
				   }
				newplot=1;
				erase=0;
				if(npurge) purge++;
				return;
			case 'e':		/* erase */
				if(erase == -1) /* ignore first 'e' or 'b' */
				   {
					erase=0;
					break;
				   }
				newplot=1;
				erase=1;
				if(npurge) purge++;
				return;
			case 'f':		/* fat */
				fat= fatbase+ getw(pltin);
				break;
			case 'r':		/* raster data */
				putras();
				break;
			case 'w':		/* window */
				xmin=((unsigned)getw(pltin))/scale;
				if(xmin<XMIN) xmin=XMIN;
				xmax=((unsigned)getw(pltin))/scale;
				if(xmax>XMAX) xmax=XMAX;
				ymin=((unsigned)getw(pltin))/scale;
				if(ymin<YMIN) ymin=YMIN;
				ymax=((unsigned)getw(pltin))/scale;
				if(ymax>YMAX) ymax=YMAX;
				break;
			case 'x':		/* display X-hairs */
			case 'p':		/* purge plot buffers */
			case 'n':		/* no op */
				break;
			default: 		/* error */
				fprintf(stderr,"mp: invalid plot command %c\n",c&CMDMSK);
				sflush();
				pass2();
				if (plotit) cat();
				pltend();	/* error is fatal */
		   }
		erase = 0;
	   }
	newplot=0;
	erase=0;
   }


dump(x1,y1,x2,y2)
int x1,y1,x2,y2;
/*
 * place segments of the line defined by the endpoints
 * (x1,y1) and (x2,y2) int the appropriate sectors
 */
   {
	int istart,iend,i,xst,yst,xstore,ystore;
	register int *ptr, xbndry;
	register struct secbuf *psec;

	if( clip(&x1,&y1,&x2,&y2) ) return;
	if(x1 > x2)	/* make x interval positive */
	   {
		xstore=x1; x1=x2; x2=xstore;
		ystore=y1; y1=y2; y2=ystore;
	   }
	istart= x1/(NXW-1);
	iend= x2/(NXW-1);
	psec= mem;
	psec=+ istart;
	xst= x1%(NXW-1);
	yst= y1;
	xbndry= (istart+1)*(NXW-1);
	xstore= NXW-1;
	for(i=istart;i<=iend;i++)
	   {
		if(i==iend)
		   {
			xstore= x2%(NXW-1);
			ystore= y2;
		   }
		 else  ystore=solve(xbndry,x1,y1,x2,y2);
		if(psec->nused ==252)
		   {
			if(head[i]== -1)
				head[i]= psec->assign= nextav++;
			psec->link= nextav++;
			lseek(diskw, 512L*psec->assign, 0);
			write(diskw,psec,512);
			psec->assign= psec->link;
			psec->nused= 0;
		   }
		ptr= psec->buf + psec->nused;
		*ptr++= xst;
		*ptr++= yst;
		*ptr++= xstore;
		*ptr  = ystore;
		(psec++)->nused=+ 4;
		xst=0;
		yst=ystore;
		xbndry=+ (NXW-1);
	   }
	return;
   }


sflush()
/*
 * flush sector buffers
 */
   {
	register int i;
	register struct secbuf *p;
	for(i=0,p=mem; i<NSEC; i++,p++)
	   {
		if(head[i]== -1)
			if(p->nused)
				head[i]= p->assign= nextav++;
			 else   continue;
		p->buf[p->nused]= ENDSEC;
		lseek(diskw, 512L*p->assign, 0);
		write(diskw,p,512);
	   }
   }



pass2()
/*
 * rasterize line segments in all sectors
 */
   {
	int nlast,zap,i,nrows;
	register int *p, *ptr;
	register struct secbuf *pbuf;
	wrtptr=mem;
	pbuf= tbuf;
	if(erase) wrtsec( -NERASE );
	for(nlast=NSEC-1; (nlast>0)&&(head[nlast]== -1); nlast--);
	if(head[nlast]== -1) /* empty plot */
		goto finish;
	for(p=secbase+(NXW-1)*NYW; p< secbase+NXW*NYW; *p++=0);
	zap=1;
	for(i=0; i<=nlast; i++)  /* loop over sectors */
	   {
		for(p= secbase, ptr= secbase+(NXW-1)*NYW;
			p< secbase+NYW; *p++= *ptr++);
		if(zap) for(p= secbase+NYW; p< secbase+NXW*NYW; *p++=0);
		if(head[i]== -1) /* sector empty */
		   {
			wrtsec( -(NXW-1) );
			for(p= secbase+(NXW-1)*NYW; p< secbase+NXW*NYW; *p++=0);
			zap=0;
			continue;
		   }
		lseek(diskr, 512L*head[i], 0);
		read(diskr,pbuf,512);
		ptr= pbuf->buf;
		pbuf->nused= 0;
		for(;;)
		   {
			if(pbuf->nused== 252)
			   {
				if(pbuf->link== ENDSEC) break;
				lseek(diskr, 512L*pbuf->link, 0);
				read(diskr,pbuf,512);
				pbuf->nused= 0;
				ptr= pbuf->buf;
			   }
			if(*ptr == BEGRAS)
			   {
				getras();
				ptr= pbuf->buf +pbuf->nused;
				continue;
			   }
			if(*ptr == ENDSEC) break;
			lplot(ptr);
			ptr=+ 4;
			pbuf->nused=+ 4;
		   }
		if(i == nlast)
		   {
			for(p= secbase+(NXW-1)*NYW-1;
				(p>secbase)&&( !(*p) ); p--);
			nrows= (p-secbase)/NYW;
			wrtsec( nrows );
		   }
		 else  wrtsec( NXW-2 );
		zap=1;
	   }
    finish:
	nodd= wrtptr-mem;
	if(nodd) write(plotw,mem,nodd*2);
   }


wrtsec(n)
int n;
/*
 * write out sector. First pack the sector by eliminating leading
 * and trailing zero in each raster line. Negative n means put
 * |n| blank raster lines.
 */
   {
	int *p1,*p2,offset,header,nwrite,i;
	register int *p,*pmem,count;

	pmem= wrtptr;
	if(n < 0)
	   {
		count=n;
		while(count++) *pmem++= 0;
		goto bypass;
	   }
	p1= secbase;
	p2= secbase+(NYW-1);
	for(i=0;i<=n;i++)
	   {
		for(p=p2; (p>p1) && (!*p); p--);
		if(*p)
		   {
			count= (p-p1)+1;
			for(p=p1; !*p; p++);
			offset= p-p1;
			count=- offset;
			header= ((offset+STATOFF)<<8) +count;
		   }
		 else  count= header= 0;
		*pmem++= header;
		while(count--) *pmem++ = *p++;
		p1=+ NYW;
		p2=+ NYW;
	   }
    bypass:
	count= pmem-mem;
	p=mem;
	if(nodd)
	   {
		nwrite= 256-nodd;
		if(nwrite>count) nwrite=count;
		if(nwrite) write(plotw,p,nwrite*2);
		count=- nwrite;
		nodd= (nodd+nwrite)%256;
		p=+ nwrite;
	   }
	if( (nwrite= count/256) )
	   {
		nwrite=* 256;
		write(plotw,p,nwrite*2);
		count=- nwrite;
		p=+ nwrite;
	   }
	pmem= mem;
	while(count--) *pmem++= *p++;
	wrtptr= pmem;
   }

int bit[16]		/* bit-set bits */
	{0100000,040000,020000,010000,04000,02000,
	 01000,0400,0200,0100,040,020,010,04,02,01};
int *bit15 &bit[15];

lplot(ptr)  /* plot a straight line */
int *ptr;
   {
	int dx,dy,dx2,dy2,flag;
	int x1,y1,x2,y2;
	register int *pbuf,test,*pbit;

	pbuf= ptr;
	x1= *pbuf++;
	y1= *pbuf++;
	x2= *pbuf++;
	y2= *pbuf;
	dx=(x2>x1?x2-x1:x1-x2); dy=(y2>y1?y2-y1:y1-y2);
	dx2=dx<<1; dy2=dy<<1;
	if(((dx<dy)&&(y1>y2))||((dx>=dy)&&(x1>x2)))
	   {
		test=x1;x1=x2;x2=test;  /* swap endpoints to make */
		test=y1;y1=y2;y2=test;  /* intervals positive  */
	   }
	pbuf= secbase +(x1*NYW+(y1>>4));
	pbit= &bit[y1&017];
	*pbuf=| *pbit;  /* plot first point */
	if(dx>=dy)  /* plot y = f(x)  */
	   {
		flag= y2>y1; test=dx-dy2;
		while(dx--)
		   {
			pbuf=+ NYW;
			if(test <= 0) /* test for y increment */
			   {
				if(flag)
				   {
					if(++pbit>bit15){pbuf++;pbit=bit;}
				   }
				 else
				   {
					if(--pbit<bit) {pbuf--;pbit=bit15;}
				   }
				test=+ dx2;
			   }
			*pbuf=| *pbit;
			test=- dy2;
		   }
	   }
	 else  /*  plot x = f(y)  */
	   {
		flag= x2>x1; test=dy-dx2;
		while(dy--)
		   {
			if(test <= 0) /* test for x increment */
			   {
				if(flag) pbuf=+ NYW;
				 else    pbuf=- NYW;
				test=+ dy2;
			   }
			if(++pbit>bit15) {pbuf++;pbit=bit;}
			*pbuf=| *pbit;
			test=- dx2;
		   }
	   }
   }


putras()
/*
 * put raster data into temp file
 */
   {
	int x,yold16,ymax16,header,eject,isec,i;
	int count,offset;
	register struct secbuf *pbuf;
	register int len, *ptr;

	dotsize = getc(pltin)&0377;
	x=xold;
	header= BEGRAS;
	while(x < xmin)
	   {
		while((header=getw(pltin)) != -1)
		   {
			if (escr(header)) continue;
			len= header&0377;
			while(len--) getw(pltin);
			x++;
		   }
	   }
	if(header == -1) return;
	isec= x/(NXW-1);
	pbuf= mem;
	pbuf=+ isec;
	yold16= yold/16;
	ymax16= ymax/16;
	dumpr(BEGRAS,pbuf);
	dumpr(x%(NXW-1),pbuf);
	dumpr(yold,pbuf);
	while((header=getw(pltin)) != ENDRAS)
	   {
		if (escr(header)) continue;
		count= header&0377;
		offset= (header>>8)&0377;
		eject=0;
		if((len=ymax16-yold16-offset) < count)
		   {
			if(len<=0)
			   {
				eject=count;
				count=0;
				header=0;
			   }
			 else
			   {
				eject= count-len;
				count=len;
				header= count +(offset<<8);
			   }
		   }
		dumpr(header,pbuf);
	   loop:
		len= 252- pbuf->nused;
		if(len > count) len=count;
		ptr= pbuf->buf + pbuf->nused;
		count=- len;
		pbuf->nused=+ len;
		while(len--) *ptr++= getw(pltin);
		if( count && (pbuf->nused==252) )
		   {
			dumpr(getw(pltin),pbuf);
			count--;
		   }
		if(count) goto loop;
		while(eject--) getw(pltin);
		x++;
		if(x > xmax)
		   {
			while((header=getw(pltin)) != -1)
			   {
				if (escr(header)) continue;
				count= header&0377;
				while(count--) getw(pltin);
			   }
			return;
		   }
		if( x%(NXW-1) == 0)
		   {
			dumpr(ENDRAS,pbuf);
			while(pbuf->nused&03) pbuf->nused++;
			pbuf++;
			dumpr(BEGRAS,pbuf);
			dumpr(0,pbuf);
			dumpr(yold,pbuf);
		   }
	   }
	dumpr(ENDRAS,pbuf);
	while(pbuf->nused&03) pbuf->nused++;
   }


int escr(header)
/*
 * return true if rplot header is an escape sequence
 * return false otherwise
 * eat entire escape sequence if one
 */
  {
	register int i;

	if ((header&0377) == 0377)	/* escape */
	  {	switch ((header>>8)&0377)		/* escape code */
		  {
		  case 't':
		  case 'T':
			for (i = 0; getc(pltin) > 0; i++)
				;
			if (i&1)
				getc(pltin);
			break;
		  }
		return(1);
	  }
	else
		return(0);
  }

dumpr(val,pbuf)
register int val;
register struct secbuf *pbuf;
   {
	register int *headi;
	struct secbuf *pbufbase;
	if(pbuf->nused == 252)
	   {
		pbufbase= mem;
		headi= head + (pbuf-pbufbase);
		if( *headi == -1)
			*headi= pbuf->assign= nextav++;
		pbuf->link= nextav++;
		lseek(diskw, 512L*pbuf->assign, 0);
		write(diskw,pbuf,512);
		pbuf->assign= pbuf->link;
		pbuf->nused= 0;
	   }
	pbuf->buf[pbuf->nused++]= val;
   }


getras()
/*
 * get raster data from the temp file
 */
   {
	int xbase,ybase,count,header,*base;
	register int len,*psec,*ptr;
	struct secbuf *pbuf;

	pbuf= tbuf;
	pbuf->nused++;
	ptr= pbuf->buf +pbuf->nused;
	xbase= *ptr++;
	ybase= *ptr++;
	pbuf->nused=+ 2;
	base= secbase +(xbase*NYW +(ybase>>4));
	while((header= *ptr++) != ENDRAS)
	   {
		pbuf->nused++;
		count= header&0377;
		psec= base +( (header>>8)&0377 );
	  loop:
		len= 252 - pbuf->nused;
		if(len > count) len=count;
		count=- len;
		pbuf->nused=+ len;
		while(len--) *psec++ =| *ptr++;
		if(pbuf->nused == 252)
		   {
			lseek(diskr, 512L*pbuf->link, 0);
			read(diskr,pbuf,512);
			pbuf->nused=0;
			ptr= pbuf->buf;
		   }
		if(count) goto loop;
		base=+ NYW;
	   }
	pbuf->nused++;
	while(pbuf->nused&03) pbuf->nused++;
   }


/* code for clip */
#define code(x,y) (x<xmin?1:(x>xmax?2:0))|(y<ymin?4:(y>ymax?8:0))

clip(x1,y1,x2,y2)	/* window the plot */
int *x1,*y1,*x2,*y2;
   {
	register int c1,c2,temp;
	int swap;
	c1=code(*x1,*y1);
	c2=code(*x2,*y2);
	swap=0;
	if(!(c1||c2)) return(0); /* line completely in bounds */
	while(c1|c2)
	   {
		if( c1&c2 ) return(1);  /* line completely out of bounds */
		if(!c1)	/* interchange endpoints */
		   {
			temp= *x1;*x1= *x2;*x2=temp;
			temp= *y1;*y1= *y2;*y2=temp;
			temp=c1;c1=c2;c2=temp;
			swap= ~swap;
		   }
		if(c1<4)	/* move endpoint in x */
		   {
			temp=(c1&2?xmax:xmin);
			*y1= solve(temp,*x1,*y1,*x2,*y2);
			*x1=temp;
		   }
		  else		/* move endpoint in y */
		   {
			temp=(c1&8?ymax:ymin);
			*x1= solve(temp,*y1,*x1,*y2,*x2);
			*y1=temp;
		   }
		c1=code(*x1,*y1);
	   }
	if( swap )	/* put endpoints in order */
	   {
		temp= *x1; *x1= *x2; *x2=temp;
		temp= *y1; *y1= *y2; *y2=temp;
	   }
	return(0);
   }




solve(pnot,p1,q1,p2,q2) /* solve linear eqn in integers */
int pnot,p1,q1,p2,q2;
   {
	register int pmid,qmid;
	if(p1>p2)
	   {
		pmid=p1;p1=p2;p2=pmid;
		qmid=q1;q1=q2;q2=qmid;
	   }
	if(pnot<=p1) return(q1);
	if(pnot>=p2) return(q2);
	while(1)  /* iterate until convergence */
	   {
		pmid= (p1+p2)>>1;
		qmid= (q1+q2)>>1;
		if(pmid<pnot) {p1=pmid;q1=qmid;}
		 else if(pmid>pnot) {p2=pmid;q2=qmid;}
			else return(qmid);
	   }
   }
#include	"putsym.h"
