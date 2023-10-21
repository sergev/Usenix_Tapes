#include "tdef.h"
#include "t.h"
#include "tw.h"

/*
 * troff3.c
 * 
 * macro and string routines, storage allocation
 *
 *		R E V I S I O N   H I S T O R Y
 *
 *	-Who-	-When-		-What-
 *
 *	MJM	8/19/77		Additions for JHU debugging feature
 */

extern int bug;



extern int ch;
extern int ibf;
extern int nextb;
extern char *enda;
extern int lgf;
extern int copyf;
extern int ch0;
extern int ip;
extern int iseek;
extern int app;
extern int ds;
extern int nlflg;
extern int *nxf;
extern int *argtop;
extern int *ap;
extern int nchar;
extern int *frame;
extern int *stk;
extern int pendt;
extern int rchar;
extern int dilev;
extern int *dip;
extern int nonumb;
extern int lt;
extern int nrbits;
extern int nform;
extern int fmt[];
extern int oldmn;
extern int newmn;
extern int macerr;
extern int apptr;
extern int apseek;
extern int oseek;
extern int offset;
extern int aplnk;
extern int diflg;
extern int woff;
extern int roff;
extern int rseek;
extern int wbfi;
extern int po;
extern int *cp;
extern int xxx;
int pagech '%';
int strflg;
extern struct contab {
	int rq;
	int (*f)();
}contab[NM];
int blist[NBLIST];
int wbuf[BLK];
int rbuf[BLK];



/* Ignore input lines */
caseig(){
	register i;

	/* characters read are not written anywhere */
	offset = -1;
	/* lines are read in copy mode and discarded by copyb()
	   until the request signalling end-of-ignore is encountered.
	   This request (if not the default "..") is executed. */
	if((i = copyb()) != '.')control(i,1);
}


/* Rename a request */
casern(){
	register i,j;

	lgf++;  /* ligatures inappropriate */
	skip();  /* to old name */
	/* no name given or no old request by the name given */
	if(((i=getrq())==0) || ((oldmn=findmn(i)) < 0))return;
	skip();  /* to new name */
	/* remove any old definition for the new name given */
	clrmn(findmn(j = getrq()));
	/* place the new name over the old name with
	   the old name's macro-indicator bit setting */
	if(j)contab[oldmn].rq = (contab[oldmn].rq & MMASK) | j;
}


/* Remove a request */
caserm(){
	lgf++;  /* ligatures inappropriate */
	skip();  /* to request name */
	/* remove name from table and free temp file blocks */
	clrmn(findmn(getrq()));
}


/* Append to string */
caseas(){
	app++;			/* set append flag */
	caseds();
}


/* Define string */
caseds(){
	ds++;			/* set define string flag */
	casede();
}


/* Append to macro */
caseam(){
	app++;			/* set append flag */
	casede();
}


/* Define a macro/string */
casede(){
	register i, savoff, req;
	int savosk;

	/* write last of any diversion in progress */
	if(dip->op > -1)wbfl();
	req = '.';  /* default end-of-copy request name */
	lgf++;  /* ligatures inappropriate */
	skip();  /* to macro name */
	if((i=getrq())==0)goto de1;  /* no name given */
	if(finds(i,&oseek,&offset) == -1)goto de1;
	if(ds)copys();  /* copy string to temp file */
		else req = copyb();  /* copy macro to temp file */
	wbfl();  /* flush last of definition to temp file */
	clrmn(oldmn);  /* wipe out old name and definition (if any) */
	/* insert new name with macro-indicator bit on
	   into the slot allocated by finds() */
	if(newmn)contab[newmn].rq = i | MMASK;

	if(apptr > -1){
		/* appending */
		/* save the write offset which points at the
		   end of the characters just appended */
		savosk = oseek;
		savoff = offset;
		oseek = apseek;
		offset = apptr;  /* write where apptr points */
		/* write impossible char over the 0 which marks
		   the end of the original definition */
		wbt(IMP);
		oseek = savosk;
		offset = savoff;
	}
	/* restore write ptr to position before .de encountered */
	oseek = dip->os;
	offset = dip->op;
	/* execute the end-of-copy request returned by copyb()
	   if not the default ".."  */
	if(req != '.')control(req,1);
de1:
	ds = app = 0;  /* reset string and append flags */
	return;
}


/* Find a macro name in table */
/*	return:		index of name in table
			-1 on name not found
*/
findmn(i)
int i;  /* request name to look up */
{
	register j;

	/* lookup name - ignore macro bit */
	for(j=0;j<NM;j++){
		if(i == (contab[j].rq & ~MMASK))break;  /* found it */
	}
	if(j==NM)j = -1;  /* didn't find it */
	return(j);
}


/* Clear table entry and deallocate temp file blocks
   associated with a macro/string */
clrmn(i)
int i;  /* index of macro/string into name table */
{
	/* if name exists */
	if(i >= 0){
		/* if a macro/string then free temp file space */
		if(contab[i].rq & MMASK)free(contab[i].f);
		contab[i].rq = 0;  /* clear name */
		contab[i].f = -1;  /* clear ptr to def on temp file */
	}
}


/* Find temp file space for a string/macro definition */
/*	return:		number of block found
			-1 on no blocks available

			*sk = seeks to space found
			*of = offset from seek pt to space found
*/
finds(mn,sk,of)
int mn,		/* name of macro/string */
    *sk,	/* return seeks into temp file */
    *of;	/* return offset from seek point */
{
	register i, savip, savisk;
	int nblok;

	/* oldmn is index into contab for an already
	   existing mn */
	oldmn = findmn(mn);
	apptr = -1;
	newmn = aplnk = 0;
	/* if appending, mn already exists, and is a macro/string */
	if(app && (oldmn >= 0) && (contab[oldmn].rq & MMASK)){
			/* append mode */
			savisk = iseek;
			savip = ip;  /* save input ptr */
			boff(contab[oldmn].f,&iseek,&ip);
			nblok = contab[oldmn].f;
			/* when appending, zap the old name so the
			   thing we're appending to doesn't get wiped out */
			oldmn = -1;
			while((i=rbf()) != 0);  /* read to end of definition */
			apseek = iseek;
			apptr = ip;  /* set append ptr here (points at 0) */
			/* ip points at next char after the 0 */
			if(!diflg) incoff(&iseek,&ip);
			/* return ptr to char after 0 */
			*sk = iseek;
			*of = ip;
			ip = savip;  /* restore input ptr */
			iseek = savisk;
	}else{
		/* definition mode */
		/* find empty slot in name table */
		for(i=0;i<NM;i++){
			if(contab[i].rq == 0)break;  /* found one */
		}
		if((i==NM) ||  /* no names available or */
		   (nblok = alloc()) == -1){  /* no space on temp file */
			app = 0;
			if(macerr++ > 1)done2(02);
			prstr("Too many string/macro names.\n");
			edone(04);
			return(offset = -1);
		}
		contab[i].f = nblok;  /* starting block of definition */
		boff(nblok,sk,of);  /* get offset to return for that block */
		if(!diflg){
			/* macro/string */
			newmn = i;  /* newmn is index of allocated name */
			/* tag allocated name slot if the correct
			   name isn't already there */
			if(oldmn == -1)contab[i].rq = -1; 
		}else{
			/* diversion */
			/* stick in name with macro-bit on */
			contab[i].rq = mn | MMASK;
		}
	}

	app = 0;  /* reset append flag */
	oseek = *sk;
	offset = *of;
	return(nblok);
}


/* Skip spaces to next field on input line */
/*	return:		0 on no end-of-line
			1 on end-of-line
*/
skip(){
	register i;

	while(((i=getch()) & CMASK) == ' ');
	ch=i;  /* un-read this char */
	return(nlflg);
}


/* Copy macro definition to temp file */
/*	return:		name of end-of-copy request
*/
copyb()
{
	register i, j, k;
	int ii, req, state, savoff, savosk;

	/* if can't get end-of-copy request name from the
	   input line, default name to ".." */
	if(skip() || !(j=getrq()))j = '.';
	req = j;  /* end-of-copy request name */
	k = j>>BYTE;  /* cream right byte: k = 2nd char of name */
	j =& BMASK;  /* cream left byte: j = 1st char of name */
	copyf++;  /* copy mode on */
	flushi();  /* eat rest of input line */
	nlflg = 0;
	state = 1;  /* beginning of a line */
	while(1){
		i = (ii = getch()) & CMASK;  /* read a char */
		if(state == 3){
			/* check for match on 2nd char of
			   end-of-copy request name */
			if(i == k)break;  /* match */
			/* check for match on one char
			   end-of-copy request name */
			if(!k){
				ch = ii;  /* re-read this char */
				i = getach();
				ch = ii;  /* un-read it again */
				/* if the char re-read was a delimeter
				   getach() will return 0 and the
				   single char name was matched */
				if(!i)break;
			}
			state = 0;  /* middle of a line */
			goto c0;
		}
		if(i == '\n'){
			state = 1;  /* beginning of a line */
			nlflg = 0;
			goto c0;
		}
		if((state == 1) && (i == '.')){
			/* found control char at beginning of a line */
			state++;  /* state = 2 */
			/* save write ptr in case these chars are
			   not part of the definition being saved */
			savosk = oseek;
			savoff = offset;
			goto c0;
		}
		if((state == 2) && (i == j)){
			/* matched first char of copy-end name */
			state++;  /* state = 3 */
			goto c0;
		}
		state = 0;  /* middle of a line */
c0:
		if(offset > -1)wbf(ii);  /* write a char */
	}
	if(offset > -1){
		wbfl();  /* write rest of def */
		/* write over chars that were part of the
		   end-of-copy request and not part of the
		   definition */
		oseek = savosk;
		offset = savoff;
		wbt(0);  /* write end marker */
	}
	copyf--;  /* copy mode off */
	return(req);
}


/* Copy string definition to temp file */
copys()
{
	register i;

	copyf++;  /* copy mode on */
	if(skip())goto c0;  /* null string */
	/* write first char of string of not delimeter */
	if(((i=getch()) & CMASK) != '"')wbf(i);
	/* copy string */
	while(((i=getch()) & CMASK) != '\n')wbf(i);
c0:
	wbt(0);  /* write end marker */
	copyf--;  /* copy mode off */
}


/* Allocate block in string/macro temp file */
/*	return:		block number of block allocated
			-1 on no blocks available
*/
alloc()
{
	register i;

	/* search block list for available block */
	for(i=0;i<NBLIST;i++){
		if(blist[i] == -1)break;  /* found one */
	}
	if(i == NBLIST){
		/* no available blocks */
		return(-1);
	}else{
		/* ptr to next block of definition is null */
		blist[i] = -2;
		/* return block number */
		return(i);
	}
}


/* Free list of blocks starting at block number i */
free(i)
int i;  /* starting block number */
{
	register j;

	/* traverse until null link is found */
	while(i != -2){
		j = blist[i];  /* save link */
		blist[i] = -1;  /* block free for allocation */
		i = j;  /* follow link */
	}
}


/* Convert block number to offset into temp file for that block */
boff(b,sk,of)
int b,		/* block number */
    *sk,	/* return seeks into temp file */
    *of;	/* return offset from seek point */
{
	/* Offsets into the macro/string temp file are kept as
	   a number of maximal seeks (SEEKDIST blocks) and an
	   offset from the point reached through these seeks.
	   This allows the macro/string temp file to grow beyond
	   the maximum distance of a single seek.  */

	*sk = b/SEEKDIST;
	*of = (b%SEEKDIST)*BLK;
}


/* Write last char and write buffer to macro/string temp file */
wbt(i)
int i;  /* character to write */
{
	wbf(i);  /* write last char */
	wbfl();  /* flush buffer */
}


/* Write a character to macro/string temp file */
wbf(i)
int i;  /* char to write */
{
	register j, nblok;

	if(offset < 0)return;  /* don't write */
	if(woff < 0){
		/* buffer has been written - reinit it */
		woff = offset;  /* reset write offset */
		wbfi = 0;  /* write ptr reset to beginning of buffer */
	}
	wbuf[wbfi++] = i;  /* put char in buffer and increment ptr */
	if(!((++offset) & (BLK-1))){
		/* next char starts a new block */
		/* write buffer - fill current block */
		wbfl();
		if(blist[j = blisti(oseek,--offset)] == -2){
			/* no next block already linked, must allocate one */
			if((nblok = alloc()) == -1){
				prstr("Out of temp file space.\nWhat a Disk Hog!\n");
				done2(01);
			}
			blist[j] = nblok;  /* link to block allocated */
		}
		/* offset is top of next block */
		boff(blist[j],&oseek,&offset);
	}
	/* full write buffer - write it out */
	if(wbfi >= BLK)wbfl();
}


/* Write buffer to macro/string temp file */
wbfl(){
	register i;

	if(woff == -1)return;
	seek(ibf,0,0);  /* top of temp file */
	for(i=0;i < oseek;i++) seek(ibf,MAXSEEK,1);
	seek(ibf, woff<<1, 1);
	write(ibf, &wbuf, wbfi<<1);
	/* writing in same block we're reading from - set roff so
	   the new version of the block is read when rbf0() is called */
	if(((woff & (~(BLK-1))) == (roff & (~(BLK-1)))) && (rseek == oseek))roff = -1;
	woff = -1;  /* block has been written */
}


/* Convert offset into temp file to block number */
blisti(sk,of)
int sk,		/* seeks into temp file */
    of;		/* offset from seek point */
{
	return((sk*SEEKDIST)+(of/(BLK)));
}


/* Read a char from macro/string */
rbf(){
	register i;

	if((i=rbf0(iseek,ip)) == 0){
		/* read an end marker */
		/* pop the input level if not simply reading to end
		   of definition so that an append may be done */
		if(!app)i = popi();
	}else{
		/* read a character */
		/* increment input ptr to read next char */
		incoff(&iseek,&ip);
	}
	return(i);
}


/* Read a char of string/macro - manage read buffer */
rbf0(sk,of)
int sk,	/* seeks into temp file */
    of;	/* offset from seek point */
{
	register i, j;

	if(((i = (of & (~(BLK-1)))) != roff) || (sk != rseek)){
		/* offset p is not in currently loaded read buffer */
		rseek = sk;
		roff = i;  /* new read offset */
		seek(ibf,0,0);
		for(j=0;j < rseek;j++) seek(ibf,MAXSEEK,1);
		seek(ibf,roff<<1,1);
		/* read a buffer-full from new offset */
		if(read(ibf, &rbuf, BLK<<1) == 0)return(0);
	}
	/* return the char at the offset passed */
	return(rbuf[of & (BLK-1)]);
}


/* Increment offset - handle crossing blocks */
incoff(sk,of)
int *sk,	/* return seeks into temp file */
    *of;	/* return offset from seek point */
{
	register j;

	if(!((j = ++(*of)) & (BLK-1))){
		/* incrementing offset leaves current block */
		/* j becomes top of next block */
		if((j = blist[blisti(*sk,--(*of))]) == -2){
			/* no link to next block! */
			prstr("Bad storage allocation.\n");
			done2(-5);
		}
		boff(j,sk,of);
	}
}




/* Push/Pop information on this level of macro nesting.
 * Used to recover from nested macros
 */
popi(){
	register int *p;

	if(frame == stk)return(0);
	if(strflg)strflg--;
	p = nxf = frame;
	*p++ = 0;
	frame = *p++;
	iseek = *p++;
	ip = *p++;
	nchar = *p++;
	rchar = *p++;
	pendt = *p++;
	ap = *p++;
	cp = *p++;
	ch0 = *p++;
	return(*p);
}


pushi(newiseek,newip)
int newiseek,	/* new seeks into temp file */
    newip;	/* new offset from seek point */
{
	register int *p;

	if((enda - (STKSIZE<<1)) < nxf)setbrk(DELTA);
	p = nxf;
	p++; /*nargs*/
	*p++ = frame;
	*p++ = iseek;
	*p++ = ip;
	*p++ = nchar;
	*p++ = rchar;
	*p++ = pendt;
	*p++ = ap;
	*p++ = cp;
	*p++ = ch0;
	*p++ = ch;
	cp = nchar = rchar = pendt = ap = ch0 = ch = 0;
	frame = nxf;
	if(*nxf == 0) nxf =+ STKSIZE;
		else nxf = argtop;
	iseek = newiseek;
	ip = newip;
}


setbrk(x)
char *x;
{
	register char *i;
	char *sbrk();

	if((i = sbrk(x)) == -1){
		prstrfl("Core limit reached.\n");
		edone(0100);
	}else{
		enda = i + x;
	}
	return(i);
}
getsn(){
	register i;

	if((i=getach()) == 0)return(0);
	if(i == '(')return(getrq());
		else return(i);
}
setstr(){
	register i;
	int ssk, sof;

	lgf++;  /* ligatures off */
	if(((i=getsn()) == 0) ||
	   ((i=findmn(i)) == -1) ||
	   !(contab[i].rq & MMASK)){
		lgf--;  /* ligatures back on */
		return(0);
	}else{
		if((enda-2) < nxf)setbrk(DELTA);
		*nxf = 0;  /* no arguments to a string */
		strflg++;
		lgf--;  /* ligatures on to read string */
		boff(contab[i].f,&ssk,&sof);
		pushi(ssk,sof);
		return(1);
	}
}


/* Collect argument list for user macro call */
collect()
{
	register i;
	register int *strp;
	int *argpp, *argppend;
	int quote, *savnxf, *lim;
	extern int ttyod;		/* diagnostic output FD */

	/*
	 * argpp = parameter pointer
	 * argpend = end of parameter pointers
	 */
	copyf++;  /* copy mode on */
	*nxf = 0;
	if(skip())goto rtn;  /* no arguments */
	savnxf = nxf;
	lim = nxf =+ 20*STKSIZE;
	strflg = 0;

	argpp = savnxf + STKSIZE;
	argppend = strp = argpp + 9;
	if( argppend > enda )  setbrk( DELTA );

	for(i=8; i>=0; i--)argpp[i] = 0;

	while((argpp != argppend) && (!skip())){
		*argpp++ = strp;
		quote = 0;
		/* peek for quote */
		if(((i = getch()) & CMASK) == '"')quote++;
			else ch = i;
		/* scan single argument string */
		while(1){
			i = getch();
			/* check for end of arg */
			if( nlflg ||  /* found newline */
			  /* blank not inside quotes */
			  ((!quote) && ((i & CMASK) == ' ')))break;
			/* check for end of quote enclosed arg */
			if(quote && ((i & CMASK) == '"') &&
			  (((i=getch()) & CMASK) != '"')){
				ch = i;
				break;
			}
			*strp++ = i;

			if(strflg && (strp >= lim)){
				prstrfl("Macro argument too long.\n");
				copyf--;
				edone(004);
			}
			if((enda-4) <= strp)setbrk(DELTA);
		}
		*strp++ = 0;
	}
	nxf = savnxf;
	*nxf = argpp - nxf - STKSIZE;
	argtop = strp;
rtn:
	copyf--;
}

seta()
{
	register i;

	if(((i = (getch() & CMASK) - '0') > 0) &&
		(i <= 9) && (i <= *frame))ap = *(i + frame + STKSIZE -1);
}

/* append to a diversion */
caseda(){
	app++;
	casedi();
}

/* Process a diversion.  Abandon hope all yea who enter here! */
casedi(){
	register i, j;

	lgf++;	/* ligature inappropriate */

	if(skip() || ((i=getrq()) == 0)){
		/* terminate the current diversion */
		if(dip->op > -1)wbt(0);  /* write end marker */
		if(dilev > 0){
			/* set height and width of diversion just completed */
			v.dn = dip->dnl;
			v.dl = dip->maxl;

			/* restore to diversion above one just ended */
			dip = &d[--dilev];
			offset = dip->op;
			oseek = dip->os;
		}
		goto rtn;
	}

	/* Start a new diversion */

	if(++dilev == NDI){
		--dilev;
		prstr("Too many nested diversions\n");
		edone(02);
	}
	/* write any diversion in progress to temp file */
	if(dip->op > -1)wbt(0);

	diflg++;
	dip = &d[dilev];
	/* find space for the diversion on temp file */
	finds(i,&(dip->os),&(dip->op));
	dip->curd = i;  /* current diversion pointer */

	/* clear the diversion if it exists already - oldmn set by finds() */
	clrmn(oldmn);
	/* clear diversion info structure - except op and curd set above */
	for(j=1; j<=10; j++)dip[j] = 0;
rtn:
	app = 0;
	diflg = 0;
}


casedt(){
	lgf++;
	dip->dimac = dip->ditrap = dip->ditf = 0;
	skip();
	dip->ditrap = vnumb(0);
	if(nonumb)return;
	skip();
	dip->dimac = getrq();
}


/* Three part titles */
casetl(){
	register i, j;
	int w1, w2, w3, begin, bseek, delim, nblok;
	extern width(), pchar();

	dip->nls = 0;
	skip();
	/* write out buffered part of any current diversion */
	if(dip->op > -1)wbfl();
	if((nblok = alloc()) == -1) return;
	boff(nblok,&oseek,&offset);
	bseek = oseek;
	begin = offset;
	if((delim = getch()) & MOT){
		ch = delim;
		delim = '\'';
	}else delim =& CMASK;
	if(!nlflg)
		while(((i = getch()) & CMASK) != '\n'){
			if((i & CMASK) == delim)i = IMP;
			wbf(i);
		}
	wbf(IMP);wbf(IMP);wbt(0);

	w1 = hseg(width,bseek,begin);
	w2 = hseg(width,-1,0);
	w3 = hseg(width,-1,0);
	oseek = dip->os;
	offset = dip->op;
#ifdef NROFF
	if(offset < 0)horiz(po);
#endif
	hseg(pchar,bseek,begin);
	if(w2 || w3)horiz(j=quant((lt - w2)/2-w1,HOR));
	hseg(pchar,-1,0);
	if(w3){
		horiz(lt-w1-w2-w3-j);
		hseg(pchar,-1,0);
	}
	newline(0);
	if(*dip){if(dip->dnl > dip->hnl)dip->hnl = dip->dnl;}
	else{if(v.nl > dip->hnl)dip->hnl = v.nl;}
	free(nblok);
}
casepc(){
	pagech = chget(IMP);
}


hseg(f,psk,p)
int (*f)();
int psk, *p;
{
	register acc, i;
	static int *q, qsk;

	acc = 0;
	if(p)q = p;
	if(psk != -1)qsk = psk;
	while(1){
		i = rbf0(qsk,q);
		incoff(&qsk,&q);
		if(!i || (i == IMP))return(acc);
		if((i & CMASK) == pagech){
			nrbits = i & ~CMASK;
			nform = fmt[findr('%')];
			acc =+ fnumb(v.pn,f);
		}else acc =+ (*f)(i);
	}
}

/* Print macro names and sizes */
casepm(){
	register i, k;
	register char *p;
	int j, xx, cnt, kk, tot;
	char pmline[10];

	kk = cnt = 0;
	tot = !skip();

	/* search macro/string/request name table */
	for(i = 0; i<NM; i++){
		if(!((xx = contab[i].rq) & MMASK))continue;  /* not a macro/string */
		p = pmline;
		j = contab[i].f;
		k = 1;
		/* count number of blocks in definition by traversing
		   block linked list to end */
		while((j = blist[j]) != -2)k++;
		cnt++;  /* number of macro/strings */
		kk =+ k;  /* total size in blocks */
		if(!tot){
			/* construct print line for each macro/string */
			/* cream left byte - get 1st char of name */
			*p++ = xx & 0177;
			/* cream right byte - get 2nd char of name */
			if(!(*p++ = (xx >> BYTE) & 0177))*(p-1) = ' ';
			*p++ = ' ';
			/* put macro/string size into pmline */
			kvt(k,p);
			/* print the line */
			prstr(pmline);
		}
	}
	if(tot || (cnt > 1)){
		/* put total block count in pmline */
		kvt(kk,pmline);
		/* print this */
		prstr(pmline);
	}
}


/* Convert k (an integer) to ASCII representation and put it where p points */
kvt(k,p)
int k;  /* integer to convert to ASCII representation */
char *p;  /* put ASCII representation starting here */
{
	/* convert each digit to a char and put it where p points */
	if(k>=100)*p++ = k/100 + '0';
	if(k>=10)*p++ = (k%100)/10 + '0';
	*p++ = k%10 + '0';
	*p++ = '\n';
	*p = 0;
}
