#include "tnt.h"
#include "tdef.h"
#include "t.h"
#include "tw.h"
#ifdef NROFF
#define GETCH gettch
#endif
#ifndef NROFF
#define GETCH getch
#endif

/*
 * troff7.c
 * Text adjusting and nofill mode
 * 
 * THIS CODE COMMON TO NROFF AND TROFF
 *
 * This code enhanced to add word markers in diversions (ENDWORD).
 * This was necessary to allow readjustment of text on dumping the
 * diversion.  Previously it was not possible to do this properly
 *
 * J. F. Ossana gets a dead fish for this one.
 *
 *					Joe Pistritto, JHU EE
 *
 */

extern int *dip;
extern int pl;
extern int trap;
extern int flss;
extern int npnflg;
extern int npn;
extern int stop;
extern int nflush;
extern int *ejl;
extern int ejf;
extern int ascii;
extern int donef;
extern int nc;
extern int wch;
extern int dpn;
extern int ndone;
extern int lss;
extern int pto;
extern int pfrom;
extern int print;
extern int nlist[NTRAP];
extern int mlist[NTRAP];
extern int *frame;
extern int *stk;
extern int *pnp;
extern int nb;
extern int ic;
extern int icf;
extern int ics;
extern int ne;
extern int ll;
extern int un;
extern int un1;
extern int in;
extern int ls;
extern int spread;
extern int totout;
extern int nwd;
extern int *pendw;
extern int *linep;
extern int line[];
extern int lastl;
extern int ch;
extern int ce;
extern int fi;
extern int nlflg;
extern int pendt;
extern int sps;
extern int adsp;
extern int pendnf;
extern int over;
extern int adrem;
extern int nel;
extern int ad;
extern int ohc;
extern int hyoff;
extern int nhyp;
extern int spflg;
extern int word[];
extern int *wordp;
extern int wne;
extern int chbits;
extern int cwidth;
extern int widthp;
extern int hyf;
extern int xbitf;
extern int vflag;
extern int ul;
extern int cu;
extern int font;
extern int sfont;
extern int it;
extern int itmac;
extern int *hyptr[NHYP];
extern int **hyp;
extern int *wdstart, *wdend;
extern int lnmod;
extern int admod;
extern int nn;
extern int nms;
extern int ndf;
extern int ni;
extern int nform;
extern int lnsize;
extern int po;
extern int ulbit;
extern int *vlist;
extern int nrbits;
extern int nmbits;
extern int xxx;
extern int nhcnt;
extern int nhzone;
int brflg;

/*
 *	Text break processor
 */
tbreak(){
	register *i, j, pad;

	trap = 0;
	/* ignore if in nobreak mode */
	if(nb)return;

	/* If not in diversion, no lines output, fake it! */
	if((dip->op == -1) && (v.nl == -1)){
		newline(1);
		return;
	}
	if(!nc){	/* no chars on line */
		setnel();	/* set up line parameters */
		if(!wch)return;
		if(pendw)getword(1);
		movword();
	}else if(pendw && !brflg){
		/* pendw = pending word */
		getword(1);
		movword();
	}
	*linep = dip->nls = 0;	/* null terminate */
#ifdef NROFF
	if(dip->op == -1)horiz(po);	/* do page offset in not in div. */
#endif
	if(lnmod)donum();
	lastl = ne;
	if(brflg != 1){
		totout = 0;
	}else if(ad){
		if((lastl = (ll - un)) < ne)lastl = ne;
	}
	if(admod && ad && (brflg != 2)){
		lastl = ne;
		adsp = adrem = 0;
#ifdef NROFF
		if(admod == 1)un =+  quant(nel/2,t.Adj);
#endif
#ifndef NROFF
		if(admod == 1)un =+ nel/2;
#endif NROFF
		else if(admod ==2)un =+ nel;
	}
	totout++;
	brflg = 0;
	/* update diversion max line length */
	if(lastl > dip->maxl)dip->maxl = lastl;
	horiz(un);	/* do page offset */

	/*
	 * in here we do line ADJUSTING
	 */
	for(i = line;nc > 0;){
		if(((j = *i++) & CMASK) == ' '){
			pad = 0;
			do{	/* in here we add spaces to pad */
				pad =+ width(j);
				nc--;
			  }while(((j = *i++) & CMASK) == ' ');
			i--;
			pad =+ adsp;
			if(adrem){
				if(adrem < 0){
#ifdef NROFF
					pad =- t.Adj;
					adrem =+ t.Adj;
				}else if((totout&01) ||
					((adrem/t.Adj)>=(--nwd))){
					pad =+ t.Adj;
					adrem =- t.Adj;
#endif
#ifndef NROFF
					pad--;
					adrem++;
				}else{
					pad++;
					adrem--;
#endif
				}
			}
			horiz(pad);
		}else{
			pchar(j);	/* actually put out char */
			nc--;
		}
	}
	if(ic){
		if((j = ll - un - lastl + ics) > 0)horiz(j);
		pchar(ic);
	}
	if(icf)icf++;
		else ic = 0;
	ne = nwd = 0;
	un = in;
	setnel();
	newline(0);
	if(dip->op > -1){if(dip->dnl > dip->hnl)dip->hnl = dip->dnl;}
	else{if(v.nl > dip->hnl)dip->hnl = v.nl;}
	for(j=ls-1; (j >0) && !trap; j--)newline(0);
	spread = 0;
}
donum(){
	register i, nw;
	extern pchar();

	nrbits = nmbits;
	nw = width('1' | nrbits);
	if(nn){
		nn--;
		goto d1;
	}
	if(v.ln%ndf){
		v.ln++;
	d1:
		un =+ nw*(3+nms+ni);
		return;
	}
	i = 0;
	if(v.ln<100)i++;
	if(v.ln<10)i++;
	horiz(nw*(ni+i));
	nform = 0;
	fnumb(v.ln,pchar);
	un =+ nw*nms;
	v.ln++;
}

/*
 * All line adjusting and text input called in from here
 * returns at end on input line.
 */
text(){
	register i;
	static int spcnt;

	nflush++;
	/* if not diversion && no lines output - fake it out */
	if((dip->op == -1) && (v.nl == -1)){newline(1); return;}
	setnel();	/* set line param */
	if(ce || !fi){
		nofill();
		return;
	}
	if(pendw)goto t4;	/* go right out  get word */
	if(pendt)if(spcnt)goto t2; else goto t3;
	pendt++;
	if(spcnt)goto t2;
	/* count spaces at begin of line */
	while(((i = GETCH()) & CMASK) == ' ')spcnt++;
	if(nlflg){
t1:
		nflush = pendt = ch = spcnt = 0;
		callsp();
		return;
	}
	ch = i;		/* unread that last char */
	if(spcnt){
t2:
		tbreak();	/* stick in a break */
		if(nc || wch)goto rtn;	/* if chars left - return */
		un =+ spcnt*sps;	/* inc page offset */
		spcnt = 0;
		setnel();
		if(trap)goto rtn;
		if(nlflg)goto t1;
	}
t3:
	if(spread)goto t5;
	if(pendw || !wch) {
t4:
		if(getword(0))goto t6;	/* if no words left -> t6 */
	}

	if(!movword())goto t3;	/* get another word if not full */
t5:
	if(nlflg)pendt = 0;
	adsp = adrem = 0;
	if(ad){		/* if adjust on */
		adsp = nel/(nwd-1);	/* adjustment per word */
#ifdef NROFF
		adsp = (adsp/t.Adj)*t.Adj;	/* round off */
#endif
		adrem = nel - adsp*(nwd-1);	/* remainder */
	}
	brflg = 1;	/* this has the weirdest effects - see tbreak */
	tbreak();	/* break off this line */
	spread = 0;	/* stop spreading lines */
	if(!trap)goto t3;
	if(!nlflg)goto rtn;
t6:
	pendt = 0;
	ckul();
rtn:
	nflush = 0;
}
nofill(){
	register i, j;

	if(!pendnf){
		over = 0;
		tbreak();
		if(trap)goto rtn;
		if(nlflg){
			ch = nflush = 0;
			callsp();
			return;
		}
		adsp = adrem = 0;
		nwd = 10000;
	}
	while((j = ((i = GETCH()) & CMASK)) != '\n'){
		if(j == ohc)continue;
		if(j == CONT){
			pendnf++;
			nflush = 0;
			flushi();
			ckul();
			return;
		}
		/* word markers */
		if(j == ' ')
			storeline( ENDWORD, 0);
		storeline(i,-1);
	}
	if(ce){
		ce--;
		/* 'un' is page offset for this line */
		if((i=quant(nel/2,HOR)) > 0) un =+ i;
	}
	if(!nc)storeline(FILLER,0);
	brflg = 2;
	tbreak();
	ckul();
rtn:
	pendnf = nflush = 0;
}

callsp(){
	register i;

	if(flss)i = flss; else i = lss;
	flss = 0;
	casesp(i);
}

/* check for underlining */
ckul(){
	if(ul && (--ul == 0)){
			/* check underlining */
			cu = 0;
			font = sfont;
			mchbits();
	}
	if(it && (--it == 0) && itmac)control(itmac,0);
}

/*
 * puts char 'c' into line buffer with no fiddling
 */
storeline(c,w){
	register i;

	if((c & CMASK) == JREG){
		if((i=findr(c>>BYTE)) != -1)vlist[i] = ne;
		return;
	}
	if(linep >= (line + lnsize - 1)){
		if(!over){	/* too long */
			/* tell him first time */
			prstrfl("Line overflow.\n");
			over++;	/* but not again */
			c = 0343;	/* error char (left hand) */
			w = -1;		/* set no width */
			goto s1;
		}
		return;
	}
s1:
	if(w == -1)w = width(c);	/* if no width - get one */
	ne =+ w;		/* w used up */
	nel =- w;		/* less left over */
	*linep++ = c;		/* add c to line buffer (finally) */
	nc++;			/* one more char */
}		/* this works line storeword() */

newline(a)
int a;
{
	register i, j, nlss;
	int opn;

	if(a)goto nl1;
	if(dip->op > -1){
		j = lss;
		pchar1(FLSS);
		if(flss)lss = flss;
		i = lss + dip->blss;
		dip->dnl =+ i;
		pchar1(i);
		pchar1('\n');
		lss = j;
		dip->blss = flss = 0;
		if(dip->alss){
			pchar1(FLSS);
			pchar1(dip->alss);
			pchar1('\n');
			dip->dnl =+ dip->alss;
			dip->alss = 0;
		}
		if(dip->ditrap && !dip->ditf &&
			(dip->dnl >= dip->ditrap) && dip->dimac)

			/* diversion traps go off - ditf flag was
			   formerly set here and not reset as the
			   more global trap flag is.  Diversion traps
			   were thus trippable only once.  ditf isn't
			   referenced anywhere anyway. */
			if(control(dip->dimac,0))trap++;
						/* dip->ditf++; */
		return;
	}
	j = lss;
	if(flss)lss = flss;
	nlss = dip->alss + dip->blss + lss;
	v.nl =+ nlss;
	/* count lines in the no hyphenation zone */
	nhcnt++;
#ifndef NROFF
	if(ascii){dip->alss = dip->blss = 0;}
#endif
	pchar1('\n');
	flss = 0;
	lss = j;
	if(v.nl < pl)goto nl2;
nl1:
	/* begin a new logical page */
	ejf = dip->hnl = v.nl = nhcnt =0;
	ejl = frame;
	if(donef){
		if((!nc && !wch) || ndone)done1(0);
		ndone++;
		donef = 0;
		if(frame == stk)nflush++;
	}
	opn = v.pn;
	v.pn++;
	if(npnflg){
		v.pn = npn;
		npn = npnflg = 0;
	}
nlpn:
	if(v.pn == pfrom){
		print++;
		pfrom = -1;
	}else if(opn == pto){
		print = 0;
		opn = -1;
		chkpn();
		goto nlpn;
		}
	if(stop && print){
		dpn++;
		if(dpn >= stop){
			dpn = 0;
			dostop();
		}
	}
nl2:
	trap = 0;
	if(v.nl == 0){
		if((j = findn(0)) != NTRAP)
			trap = control(mlist[j],0);
	} else if((i = findt(v.nl-nlss)) <= nlss){
		if((j = findn1(v.nl-nlss+i)) == NTRAP){
			prstrfl("Trap botch.\n");
			done2(-5);
		}
		trap = control(mlist[j],0);
	}
}

findn1(a)
int a;
{
	register i, j;

	for(i=0; i<NTRAP; i++){
		if(mlist[i]){
			if((j = nlist[i]) < 0)j =+ pl;
			if(j == a)break;
		}
	}
	return(i);
}

chkpn(){
	pto = *(pnp++);
	pfrom = pto & ~MOT;
	if(pto == -1){
		flusho();
		done1(0);
	}
	if(pto & MOT){
		pto =& ~MOT;
		print++;
		pfrom = 0;
	}
}

findt(a)
int a;
{
	register i, j, k;

	k = 32767;
	if(dip->op > -1){
		if(dip->dimac && ((i = dip->ditrap -a) > 0))k = i;
		return(k);
	}
	for(i=0; i<NTRAP; i++){
		if(mlist[i]){
			if((j = nlist[i]) < 0)j =+ pl;
			if((j =- a)  <=  0)continue;
			if(j < k)k = j;
		}
	}
	i = pl - a;
	if(k > i)k = i;
	return(k);
}

findt1(){
	register i;

	if(dip->op > -1)i = dip->dnl;
		else i = v.nl;
	return(findt(i));
}

eject(a)
int *a;
{
	register savlss;

	if(dip->op > -1)return;
	ejf++;
	if(a)ejl = a;
		else ejl = frame;
	if(trap)return;
e1:
	savlss = lss;
	lss = findt(v.nl);
	newline(0);
	lss = savlss;
	if(v.nl && !trap)goto e1;
}

/* moves word into line buffer - returns 0 normally, 1 at end of line */
movword(){
	register i, w, *wp;
	int savwch, hys;

	over = 0;	/* set no overflow */
	wp = wordp;	/* wp - temp word ptr */
	if(!nwd){
		while(((i = *wp++) & CMASK) == ' '){	/* delete spaces */
			wch--;
			wne =- width(i);
		}
		wp--;	/* back up one char */
	}
	if((wne > nel) &&
	   !hyoff && hyf &&	/* hyf - hyphenation flag */
	   (!nwd || (nel > 3*sps)) &&		/* > 3 spaces left */
	   (!(hyf & 02) || (findt1() > lss)) &&  /* not a last line */
	   (nhcnt > nhzone)  /* beyond no hyphenation zone */
	  )hyphen(wp);		/* we can hyphenate here */
	savwch = wch;
	hyp = hyptr;
	nhyp = 0;
	while(*hyp && (*hyp <= wp))hyp++;
	while(wch){		/* while we have chars left */
		if((hyoff != 1) && (*hyp == wp)){
			hyp++;	/* get next hyphenation point */
			if(!wdstart ||
			   ((wp > (wdstart+1)) &&
			    (wp < wdend) &&
			    (!(hyf & 04) || (wp < (wdend-1))) &&
			    (!(hyf & 010) || (wp > (wdstart+2)))
			   )	/* we can hyphenate here! */
			  ){
				nhyp++;	/* inc # of hyphen pts */

				/* place holder for hyphen */
				storeline(IMP,0);
			}
		}
		i = *wp++;	/* store next char */
		w = width(i);	/* w - width need for this char */
		wne =- w;	/* dec width needed by w */
		wch--;		/* one less char to worry about */
		storeline(i,w);	/* put it away! */
	}
	if(nel >= 0){	/* space left over */
		nwd++;	/* inc word count */
		return(0);
	}
	xbitf = 1;
	hys = width(0200); /*hyphen*/
m1:
	if(!nhyp){	/* no hyphenation points this word */
		if(!nwd)goto m3;	/* set nwd; return */
		if(wch == savwch)goto m4;
	}
	if(*--linep != IMP)goto m5;	/* if not hyphen pt */
	if(!(--nhyp))	/* if this is last hyphen oppurtunity */
		if(!nwd)goto m2;	/* hyphenate */
	if(nel < hys){	/* if not enough space for hyphen */
		nc--;	/* back up 1 char */
		goto m1;
	}
m2:
	/*
	 * This actually sticks in a hyphen
	 */
	if(((i = *(linep-1) & CMASK) != '-') && (i != 0203)) {
		*linep = (*(linep-1) & ~CMASK) | 0200;	/* this is it! */
		nhcnt = 0;  /* start no hyphenation zone */
		w = width(*linep);	/* get new width */
		nel =- w;	/* and adjust variables appropriately */
		ne =+ w;
		linep++;	/* next char  */
	/*
		hsend();
	*/
	}
m3:
	nwd++;	/* nwd - need another word */
m4:
	wordp = wp;
	return(1);
m5:
	nc--;	/* less 1 char */
	w = width(*linep);
	ne =- w;	/* adjust appropriately */
	nel =+ w;
	wne =+ w;
	wch++;
	wp--;
	goto m1;
}

/* does horizontal movement  i = horizontal offset */
horiz(i)
int i;
{
	vflag = 0;
	if(i)pchar(makem(i));
}

/* set default line parameters */
setnel(){
	if(!nc){
		linep = line;
		if(un1 >= 0){
			un = un1;
			un1 = -1;
		}
		nel = ll - un;
		ne = adsp = adrem = 0;
	}
}

/*
 * Here we actually get a word from the input
 * returns 1 on no word gotten, 0 otherwise
 */
getword(x)
int x;
{
	register i, j, swp;
	int noword;
	int endword;

	/*
	 *	'chbits' has characteristics bits for this char.
	 */
	noword = 0;
	endword = 0;
	if(x)if(pendw){
		*pendw = 0;
		goto rtn;
	}
	if(wordp = pendw)goto g1;
	hyp = hyptr;
	wordp = word;
	over = wne = wch = 0;
	hyoff = 0;
	while(1){
		j = (i = GETCH()) & CMASK;
		if(j == '\n'){
			wne = wch = 0;
			noword = 1;	/* didn't get one */
			goto rtn;
		}
		if(j == ohc){
			hyoff = 1;	/* turn off hypenation */
			continue;
		}
		if(j == ' '){
			storeword(i,cwidth);	/* put it away */
			continue;
		}
		break;
	}
	swp = widthp;	/* swp - temp var. for widthp */
	storeword(' ' | chbits, -1);	/* insert 1 space */
	if(spflg){
		storeword(' ' | chbits, -1);	/* insert another space */
		spflg = 0;
	}
	widthp = swp;	/* reset widthp */
g0:
	if(j == CONT){		/* Control U */
		pendw = wordp;
		nflush = 0;
		flushi();	/* Eat rest of line */
		return(1);	/* didn't get word */
	}
	/* hyphenation section - pass 1 */
	/* processes optional hyphenation chars (ohc)'s */
	if(hyoff != 1){
		if(j == ohc){	/* ohc = ^t (default */
			hyoff = 2;	/* set automatic hyphen mode */
			*hyp++ = wordp;	/* set hyphen point */
			if(hyp > (hyptr+NHYP-1))hyp = hyptr+NHYP-1;
			goto g1;	/* don't store the ohc */
		}
		if((j == '-') ||
		   (j == 0203) /*3/4 Em dash*/
		  )if(wordp > word+1){	/* if > 2 chars into word */
			hyoff = 2;	/* set automatic hyphen mode */
			*hyp++ = wordp + 1;	/* set hyphen point */
			if(hyp > (hyptr+NHYP-1))hyp = hyptr+NHYP-1;
			/* check to see not too many hyphens */
		}
	}
	storeword(i,cwidth);	/* add char to word */
g1:
	j = (i = GETCH()) & CMASK;	/* next char please */
	/*
	 * space, or a word marker (ENDWORD = 0777) ends the word
	 * this was the fix to the George Toth diversion bug
	 * 30 Nov 1977 by JCP 
	 */


/***************** Bell Labs Original:
	if( j != ' ' ) {			/* original version */
/*****************/

	if((j != ' ') && !endword) {
		if(j == (ENDWORD & CMASK))
			endword = 1;
		if(j != '\n')goto g0;	/* get another char */
		j = *(wordp-1) & CMASK;
		/* any of following also end word */
		if((j == '.') ||
		   (j == '!') ||
		   (j == '?'))spflg++;
	}
	/*
	 * further diversion bug stuff
	 */
	if(j == ' ')
		storeword( ENDWORD, 0);

	*wordp = 0;	/* Null terminate */
rtn:
	wdstart = 0;
	wordp = word;	/* reset to begin of word */
	pendw = 0;
	*hyp++ = 0;
	setnel();
	return(noword);	/* see return comments at start of func. */
}

/*
 * This actually stores away a char in the word buffer
 * Works a lot like storeline()
 */
storeword(c,w)
int c, w;
{

	if(wordp >= &word[WDSIZE - 1]){	/* You blew it! */
		if(!over){
			/* print first time */
			prstrfl("Word overflow.\n");
			over++;		/* but not again */
			c = 0343;	/* error char (left hand) */
			w = -1;		/* set no width */
		goto s1;
		}
		return;
	}
s1:
	if(w == -1)w = width(c);	/* if no width - get one */
	wne =+ w;	/* need this much more width */
	*wordp++ = c;	/* add char to word buffer */
	wch++;		/* inc char count this word */
}

#ifdef NROFF
extern char trtab[];
gettch(){
	register int i, j;

	if(!((i = getch()) & MOT) && (i & ulbit)){
		j = i&CMASK;
		if(cu && (trtab[j] == ' '))
			i = ((i & ~ulbit)& ~CMASK) | '_';
		if(!cu && (j>32) && (j<0370) && !(*t.codetab[j-32] & 0200))
			i =& ~ulbit;
	}
	return(i);
}
