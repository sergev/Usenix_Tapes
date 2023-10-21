#
#include "tv.h"
#include "tdef.h"
#include "t.h"
/*
troff10.c

Video Comp 500 interface
*/

extern int vthck;
extern int hthck;
extern int dfact;
extern int rule;
extern int rf;
extern int *olinep;
extern int oline[];
extern int *pslp;
extern int back;
extern int xpts;
extern int mpts;
extern int po;
extern int line[];
extern int lss;
extern int xbitf;
extern char obuf[];
extern char *obufp;
extern int esct;
extern int trflg;
extern int cs;
extern int smnt;
extern int mfont;
extern int xfont;
extern int code;
extern int mcase;
extern int esc;
extern int lead;
extern int ptid;
extern int verm;
extern int escm;
extern char pstab[];
extern int *dip;
extern int dpn;
extern int ascii;
extern int nofeed;
extern int gflag;
extern int fontlab[];
extern int pfont;
extern int ppts;
extern int oldbits;
extern int bd;
extern int bdtab[];
extern int vflag;
extern int xxx;
extern int pagew;
extern int pagel;
extern int ptinitf;
extern char codetab[];
extern char cstab[];
extern int fontcode[];
extern int endpg;
extern int sbold;
int base;
int initlf; /* initial character on a line flag, see ptout */

ptinit(){

	if(ascii || gflag)return;
	oput( JOBID);		oputw( v.pn);
	oput( FULFACE);
	oputw( pagew/ 10);	oputw( pagel/ 10);
	oput( ROT90);
	oputw( 0);		oputw( pagew/ 10);
	esct = 0;
	ptfont();
}
ptout(i)
int i;
{
	register j, *p;

	if((i & CMASK) != '\n'){
		*olinep++ = i;	/*  fill line buffer with char */
		return;
	}
	initlf++;
	if( endpg || ( !ptinitf)){
		if( ptinitf) termrec();		/* then endpg was true */
		else ptinitf++;		/* first time through */
		ptinit();
	}
	if(olinep == oline){
		lead =+ lss;	/* blank line  */
		return;
	}
	lead =+ dip->blss + lss;
	dip->blss = 0;
	esc= po;
	j= olinep- oline;		p= oline;		base= 0;
	while( j--) ptout0( *p++);
	if( rf) ptrule();  /* must put out rule if there was one */
	if( base){	/*  assures that 'cursor' is back at baseline before 
			    proceding to new line of output  */
		if (base<0) base = (-base) | NMOT;
		ptout0( base | MOT | VMOT );
	}
	olinep = oline;
	lead =+ dip->alss;
	dip->alss = 0;
}
ptout0(i)
int i;
{
	register j, k, w;
	int z, zmot, getcwf;

	if(i & MOT){
		j = i & ~MOTV;
		if(i & NMOT)j = -j;
		if( i & RMOT){
			if( !rule){
				if( lead || initlf) ptlead();
				if( esc) ptesc();
			}
			if( i & VMOT){
				if( rf & HRI ) ptrule();
				rf=| VRI;
				base =- j;
			}
			else{
				if( rf & VRI ) ptrule();
				rf=| HRI;
			}
			rule=+ j;
			return;
		}
		if(rf) ptrule();
		if(i & VMOT){
			lead =+ j;		base=- j;
		}
		else esc =+ j;
		return;
	}
	if( rf) ptrule();
	xbitf = 2;
	if((i>>BYTE) == oldbits){
		xfont = pfont;
		xpts = ppts;
		xbitf = 0;
	}else xbits(i);
	if((k = (i & CMASK)) < 040){
		return;
	}
	code = codetab[k - 32];		getcwf= 0;
		/*  want to minimize calls to getcw  */
	if(code & 0200)
		if( xfont == ( sbold -1 )) getcwf++;

	if((cs=cstab[xfont]) | !(j=code&0177) | (z=i&ZBIT) |
		(bd= bdtab[xfont]) | getcwf ) w=zmot=getcw(k-32);
	/*constant space, narrow space, bold, sbold, or zero width */

	if (!j) { 	/*narrow space*/
		if (z) return;	/*zero width*/
		if (cs) esc=+ cs;
		else esc=+ w;
		return;
	}
	if(cs){		/*  constant spacing  */
		if(bd)w =+ bd - 1;
		j = (cs-w)/2;
		if(bd) w =- (bd - 1);
	}else j = 0;
	if ( z ) {		/*zero width, zmot=actual char width*/
		if(cs) zmot=+ j;	/* left char spacing */
		if(bd) zmot=+ (bd - 1);	/* emboldening spacing */
	}else zmot = 0;
	esc =+ j;
	if( code & 0200 ) xfont= smnt -1;
	if(( xfont != mfont) || ( xpts != mpts)){
		ptfont();
	}
	if( lead || initlf)ptlead();
	if(esc)ptesc();
	oput( ( code & 0177 ) | 0200 );
/*
	*obufp++ = (code & 0177) | 0200;
	if(obufp == (obuf + OBUFSZ ))flusho();
*/
	if(bd){
		if(esc =- (w-bd+1))ptesc();	/* back up */
		oput((code & 0177) | 0200);	/* print again */
	}
	if( zmot){
		esc=- zmot;		zmot= 0;
	}
	else  if(cs) esc =+ j;	/* right char spacing */
	return;
}
ptfont(){
	register i, j, k;
	mfont= xfont;		mpts = xpts;
	oput(FONTFCH);
	oputw (fontcode[ mfont]);
	oputw( xpts *10);
}
ptlead(){
	register k, down;
	char i;

	if((k = lead) < 0){
		k = -k;			down = 1;
	}
	else down= 0;
	i=0;
	if( initlf){ /* initial character on a line flag */
		initlf= 0;
	}
	else {
		i++;			/*Must save horizontal position*/
					/*because ADVANCE causes the 'cursor'*/
					/*to move to the left margin.       */
		oput(SHTAB);	oput(i);
	}
	oput( ADCOM + down);		oputw( k);
	if(i) {oput(MHTAB);	oput(i);}
					/*return to previous horiz position*/
	lead = 0;
}
ptesc(){
	register com, bck, k;

	if(( k = esc) < 0){
		k= -k;	bck= 1;
	}
	else bck= 0;
	com= SFCOM + bck;
	for( ; ; ){
		oput( com);
		if( k < MAXESC){
			oputw( k * 5);
			break;
		}
		oputw( MESCHAR);
		k=- MAXESC;
	}
	esc = 0;
}
ptrule(){
	register int i;

	i= rule;
	if( rf & VRI ){
		lead= i;
		if(( i > 0) || initlf) ptlead();
		oput( SRCOM);
		oputw( (i > 0) ?  i : -i );
		oputw( vthck);
	}
	else{
		if( i < 0){
			esc= i;
			ptesc();
			i= -i;
		}
		oput( SRCOM);
		oputw( hthck);		oputw( i);
		esc= i;
	}
	rule= 0;	rf= 0;
}
dostop(){}
caseff()		/*  full face mode  */
{
	register i,j;
	if (skip()) return;
	j= pagew;
	dfact = INCH;
	i= inumb(&pagew);
	if(i<=0) i=j;
	pagew= (i<6480)? i : 6480;	/*max page width 9*720*/
	if (skip()) return;
	j=pagel;
	dfact= INCH;
	i= inumb(&pagel);
	if (i<=0) i=j;
	pagel = (i<7920) ? i : 7920;	/*max page length 11*720*/
}
oputw( w) int w;{
	register int i;

	i= w;
		/* swap bytes of word for compatability with target machine*/
	oput(( i >> BYTE ) & BMASK);
	oput( i & BMASK);
}
termrec(){

  register int j;
  register char *ep, *p;

	ptlead();
	oput(ENDPG);	/*  end of page  */
	endpg = 0;
	if( obufp== obuf) return;
	ep= obuf+ OBUFSZ;
	*obufp++= ENDREC;
	if( obufp != ep){
		j= ep- obufp;		p= obufp;
		while( j--) *p++= '\0';
		obufp= ep;
	}
	flusho();
}
