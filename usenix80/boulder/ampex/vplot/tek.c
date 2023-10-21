#
/*#define FAST 1 */
/*
 * vector plot filter for Tektronix 4012
 *
 * author- Rob Clayton, Dept. of Geophysics, Stanford Univ.
 * Modified for Version 7, 12/3/79
 * Mitchell Gart, Ampex Corp.
 */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#define BUFFER	1
#define CHARHT	22
#define PLTCMD	0200
#define CMDMSK	0177
#define INCHDOT	(8.0/1024.)
#define STDSIZ 16384	/* standard coordinate range */
#define STDSCL 	21	/* STDSIZ/780 to scale standard plot */
#define TWMUL 	64	/* # of pixels in standard text width size spec. */
#define THMUL 	96	/* 3/2*TWMUL */
#define STDCW 	5	/* standard text size (about 15 x 23) */
#define XMAX 	1023	/* maximum x value */
#define XMIN 	0		/* minimum x value */
#define YMAX 	780	/* maximum y value (manual says 779, but 780 fits) */
#define YMIN 	0		/* minimum y value */
#define ENDRAS 	-1	/* end of raster data flag */
#define ERASE	014
#define ESC	033
#define CSZ	032
#define GS	035
#define CUS	037
int gsreqd;		/* GS required on next graphics op */
int grkflag;		/* flag to return ascii chars */
int fatbase,fat;	/* line thickness parameters */
int xmax XMAX;		/* plot window parameters defaulted */
int xmin XMIN;		/* to maximum size 		*/
int ymax YMAX;
int ymin YMIN;
int xnew,ynew;		/* new pen location */
int xold,yold;		/* old pen location */
int xorigin,yorigin;	/* origin for text mode */
unsigned tangle, tsize;	/* new text angle, size */
char tfont;		/* future text font */
int scale;
int stop 3;		/* if 1, stop before 'e', 'b' */
int ty;			/* counter for y pos. of straight text */
int separate;		/* output redirected */
int intract, quitact;	/* old signal actions */
struct inode
  {	int dev;
	int inum;
	int flags;
  };
struct
   {
	char ispeed, ospeed;
	char erase, kill;
	int mode;
   } ttystat;
int fixtty();
FILE *pltout, *pltin, *prinout, *temp;
FILE *fopen();
char inbuf[BUFSIZ], outbuf[BUFSIZ];


main(argc,argv)
int argc; char **argv;
   {
	FILE *freopen();
	double atof();
	float fscale;
	register char *cptr;

	fstat(1, inbuf);
	fstat(2, outbuf);
	separate = (inbuf->st_ino == outbuf->st_ino)? 0: 1;
	pltin= stdin;
	pltout= stderr;
	prinout= stdout;
	setbuf(pltin,inbuf);
	if(BUFFER) setbuf(pltout,outbuf);
	scale= STDSCL;

	do	/* loop over arg list */
	   {
		argc--; argv++;
		cptr= *argv;
		if(argc && (*cptr == '-'))
		   {
			cptr++;
			switch(*cptr)
			  {
			  case 's':	/* no stop on erase */
				stop = 0;
				break;
			  case 'f':
				fatbase = atoi(++cptr);
				if (fatbase < 0)
					fatbase = 0;
				fat = fatbase;
				break;
			  default:
				fscale= atof(cptr);
				if(fscale<=0.0)
				   {
					scale= STDSCL; continue;
				   }
				while(*cptr) cptr++; /* get units */
				switch( cptr[-1] )
				  {
				  default:
				  case 'i':
					scale= STDSIZ*INCHDOT/fscale;
					break;
				  case 'c':
					scale= STDSIZ*INCHDOT/(fscale/2.54);
					break;
				  }
				break;
			  }
			continue;
		   }
		if(argc)
			if((temp=freopen(cptr,"r",pltin)) == NULL)
			   {
				fprintf(stderr,"tek:cannot open %s\n",cptr);
				fflush(stderr);
				continue;
			   }
			  else	pltin= temp;

		doplot();

	   } while(argc);
	exit();
   }

doplot()
   {
	register int i;
	register int c;
	gsreqd= 1;
	ty= YMAX-CHARHT;
	while((c=getc(pltin))!= EOF)
	   {
		if ((c&PLTCMD) == 0 || ((c&CMDMSK) < 'a' || (c&CMDMSK) > 'z'))
		   {
			if(!separate)
			   {
				plot(0, ty, 1);
				putc(CUS,pltout);
				fflush(pltout);
				stop =& 1;
			   }
			do
			   {
				if( c == '\n') ty =- CHARHT;
				putc(c,prinout);
			   }  while(((c=getc(pltin))&PLTCMD) == 0);
			if( c == EOF ) break;
			ungetc(c,pltin);
			gsreqd= 1;
			continue;
		   }
		switch (c& CMDMSK)	/* command list */
		   {
			case 'm':		/* move */
				xold= xorigin= getw(pltin)/scale;
				yold= yorigin= getw(pltin)/scale;
				break;
			case 'd':		/* draw */
				xnew=getw(pltin)/scale;
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
			case 's':		/* set up Text modes */
				tsize = getc(pltin)&0377;
				tfont = getc(pltin)&0377;
				tangle = getw(pltin);
				break;
			case 't':		/* text */
				putsym();
				break;
			case 'b':		/* break */
			case 'e':		/* erase */
				fflush(pltout);
				if (stop == 1)
					read(2, outbuf, BUFSIZ);
				putc(ESC,pltout);
				putc(ERASE,pltout);
				ty= YMAX - CHARHT;
				gsreqd = 1;
				break;
			case 'f':		/* fat */
				fat= fatbase+ getw(pltin);
				break;
			case 'r':		/* raster data */
				putras();
				break;
			case 'w':		/* window */
				xmin=((unsigned) getw(pltin))/scale;
				if(xmin<XMIN) xmin=XMIN;
				xmax=((unsigned) getw(pltin))/scale;
				if(xmax>XMAX) xmax=XMAX;
				ymin=((unsigned) getw(pltin))/scale;
				if(ymin<YMIN) ymin=YMIN;
				ymax=((unsigned) getw(pltin))/scale;
				if(ymax>YMAX) ymax=YMAX;
				break;
			case 'x':		/* display x-hairs */
				fflush(pltout);
				gtty(fileno(pltout),&ttystat);
/* used to be:	ttystat.mode =| 040;	/* raw mode */
				if ((intract = (signal(2, 1))&1) == 0)
					signal(2, &fixtty);	/* interrupts */
				if ((quitact = (signal(3, 1))&1) == 0)
					signal(3, &fixtty);	/* quits */
				ttystat.mode =& ~010;	/* no echo */
				stty(fileno(pltout),&ttystat);
			    	do
				  {	putc(ESC,pltout);
					putc(CSZ,pltout);
					fflush(pltout);
					c= getc(pltin);	/* get next cmd */
				  } while (c == ('x'|PLTCMD));
/* used to be:	putc('\r', pltout);	/* get out of funny mode */
				fflush(pltout);
				ungetc(c,pltin);
		 		ttystat.mode =& ~040;	/* turn off raw */
				ttystat.mode =| 010;
				stty(fileno(pltout),&ttystat);
				signal(2, intract);
				signal(3, quitact);
				gsreqd= 1;
				break;
			case 'p':		/* purge pltout buffers */
				fflush(pltout);
				gsreqd= 1;
				break;
			case 'n':		/* no op */
				break;
			default: 		/* error */
				fprintf(stderr,"\ntek: invalid plot command %c\n",c);
				fflush(stderr);
				exit();
			}
		stop =& 1;	/* no stop if erase is first command */
		}
/*
 * Add delay to allow last draw command 
 * to finnish before terminal jumps out
 * of graphics mode 
 */
	dump(xold,yold,xold,yold);
	dump(xold,yold,xold,yold);
	putc('\n',pltout);
	fflush(pltout);
   }


dump(x1,y1,x2,y2)
int x1,y1,x2,y2;
   {
	static int xlst, ylst;
	if( clip(&x1,&y1,&x2,&y2) ) return;
	if( (x1 != xlst) || (y1 != ylst) || gsreqd )
		plot(x1,y1,1);
	plot(x2,y2,0);
	xlst= x2;
	ylst= y2;
	gsreqd= 0;
   }


plot(x,y,icode)
int x,y,icode;
   {
	/*
	 * this code takes advantage of
	 * vector addresses.
	 */
	register char tempx, tempy;
	static char xhi,yhi,ylo;

	if( icode )	/* move pen */
	   {
		putc(GS,pltout);
		putc((y>>5)|040,pltout); 	/* y/32+32 hi y byte */
		putc((y&037)|0140,pltout); 	/* y%32+96 low y byte */
		putc((x>>5)|040,pltout); 	/* x/32+32 hi x byte */
		putc((x&037)|0100,pltout); 	/* x%32+64 low x byte */
	   }
	 else
#ifndef FAST
	   {
		putc((y>>5)|040,pltout); 	/* y/32+32 hi y byte */
		putc((y&037)|0140,pltout); 	/* y%32+96 low y byte */
		putc((x>>5)|040,pltout); 	/* x/32+32 hi x byte */
		putc((x&037)|0100,pltout); 	/* x%32+64 low x byte */
	   }
#else
	   {
		tempy= (y>>5)|040;	/* y/32+32 hi y byte */
		if(tempy != yhi)
		   {
			putc(tempy,pltout);
			yhi= tempy;
		   }
		tempy= (y&037)|0140;	/* y%32+96 low y byte */
		tempx= (x>>5)|040;	/* x/32+32 hi x byte */
		if(tempx != xhi)
		   {
			putc(tempy,pltout);
			putc(tempx,pltout);
			ylo= tempy;
			xhi= tempx;
		   }
		  else if (tempy != ylo)
		   {
			putc(tempy,pltout);
			ylo= tempy;
		   }
		putc( (x&037)|0100,pltout);	/* x%32+64 low x byte */
	   }
#endif
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


/*
 * plot maximum enclosing box for raster data
 */
putras()
   {
	register int header,count,y;
	int rxmin, rymin, rxmax, rymax, offset, dotsize, raster;

	dotsize = getc(pltin);
	if (dotsize == 0)
		dotsize = STDSIZ/2048;	/* mp resolution */
	raster = 0;
	rxmin= rxmax= xold;
	rymin= 32767;
	rymax= yold;
	while( (header=getw(pltin)) != ENDRAS )
	  {
		count= header&0377;
		offset= (header>>8)&0377;
		if (count == 0377)	/* escape */
		  {	switch (offset)
			  {
			  case 't':
			  case 'T':
				for (count=0; getc(pltin)>0; count++)
					;
				if (count&1)
					getc(pltin);
				break;
			  }
			continue;
		  }
		y = yold + (offset*16*dotsize)/STDSCL;
		if(rymin > y) rymin=y;
		y =+ (count*16*dotsize)/STDSCL;
		if(rymax < y) rymax=y;
		while(count--) getw(pltin);
		raster++;
	  }
	rxmax =+ (raster*dotsize)/STDSCL + 1;
	dump(rxmin,rymin,rxmin,rymax);
	dump(rxmin,rymax,rxmax,rymax);
	dump(rxmax,rymax,rxmax,rymin);
	dump(rxmax,rymin,rxmin,rymin);
   }

/*
 * Restore tty mode and exit.
 */
fixtty()
  {
	ttystat.mode =& ~040;	/* turn off raw */
	ttystat.mode =| 010;	/* turn on echo */
	stty(fileno(pltout),&ttystat);
	exit(1);
  }

#include "putsym.h"
