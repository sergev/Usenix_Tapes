#include "tnt.h"
#include "tdef.h"
#include "t.h"
#include "tw.h"

/*
 * troff4.c
 * 
 * number registers, conversion, arithmetic
 *
 *
 *		R E V I S I O N   H I S T O R Y
 *
 *	MJM	10/03/77	Some stylistic changes for readability.
 *
 *	JCP	1/1/78		New number register \n.- and \n.F
 *
 */

extern int hyf;
extern int ascii;
extern int cbuf[NC];
extern int *cp;
extern int r[NN];
extern int *vlist;
extern int inc[NN];
extern int fmt[NN];
extern int ch;
extern int lgf;
extern int *frame;
extern int pl;
extern int lastl;
extern int ralss;
extern int totout;
extern int nrbits;
extern int nonumb;
extern int vflag;
extern int noscale;
extern int dfact;
extern int dfactd;
extern int po;
extern int nform;
extern int ll;
extern int in;
extern int font;
extern int lss;
extern int pts;
extern int font1;
extern int *dip;
extern int fi;
extern int res;
extern int cwidth;
extern int dotT;
extern int ulfont;
extern int ev;
extern int ne;
extern int ad, admod;
extern int print;
extern int ls;
extern int xxx;
extern int exfont;
extern int stopval;	/* .S numb reg = -s flag value (usually) */
extern int upid;	/* .U register = process id */

extern struct contab {
	int rq;
	int (*f)();
}contab[NM];

setn()
{
	register i,j;
	int f;

	f = nform = 0;
	if((i=getch() & CMASK) == '+')f = 1;
		else if(i == '-')f = -1;
			else ch = i;
	if((i=getsn()) == 0) return;
	if((i & 0177) == '.') switch(i>>BYTE) {
		case 's':
			i = pts & 077;
		break;
		case 'v':
			i = lss;
			break;
		case 'f':
			i = font + 1;
			break;
		case 'p':
			i = pl;
			break;
		case 't':
			i = findt1();
			break;
		case 'o':
			i = po;
			break;
		case 'l':
			i = ll;
			break;
		case 'i':
			i = in;
			break;
		case '$':
			i = *frame;
			break;
		case 'A':
			i = ascii;
			break;
		case 'c':
			i = v.cd;
			break;
		case 'n':
			i = lastl;
			break;
		case 'a':
			i = ralss;
			break;
		case 'h':
			i = dip->hnl;
			break;
		case 'd':
			if(dip->op > -1)i = dip->dnl; else i = v.nl;
			break;
		case 'u':
			i = fi;
			break;
		case 'j':
			i = ad + 2*admod;
			break;
		case 'w':
			i = cwidth;
			break;
		case 'x':
			i = ulfont + 1;
			break;
		case 'y':
			i = font1;
			break;
		case 'T':		/*-Tterm used in nroff*/
			i = dotT;
			break;
		case 'V':
			i = VERT;
			break;
		case 'H':
			i = HOR;
			break;
		case 'k':
			i = ne;
			break;
		case 'P':
			i = print;
			break;
		case 'L':
			i = ls;
			break;
		case 'z': i = dip->curd;
			cbuf[0] = i & BMASK;
			cbuf[1] = (i >> BYTE) & BMASK;
			cbuf[2] = 0;
			cp = cbuf;
			return;

		/* n(.- hyphenation condition register */
		case '-':
			i = hyf;
			break;

		/* n(.F condition of extended font (casexf) control */
		case 'F':
			i = exfont;
			break;
		case 'S':	/* -s  flag value */
			i = stopval;
			break;

		case 'U':	/* Unique number, = process id */
			i = upid;
			break;

		default:
			goto s0;
	}
	else{
s0:
		if((j=findr(i)) == -1)i = 0;
		else{
			i = (vlist[j] = (vlist[j] + inc[j]*f));
			nform = fmt[j];
		}
	}
	setn1(i);
	cp = cbuf;
}

setn1(i)
int i;
{
	int j;
	extern int wrc();

	cp = cbuf;
	nrbits = 0;
	j=fnumb(i,wrc);
	*cp = 0;
	cp = cbuf;
}

findr(i)
int i;
{
	register j;
	static int numerr;

	if(i == 0)return(-1);
	for(j=0;j<NN;j++){
		if(i == r[j])break;
	}
	if(j != NN)return(j);
	for(j=0; j<NN; j++){
		if(r[j] == 0){
			r[j] = i;
			break;
		}
	}
	if(j==NN){
		if(!numerr)
		{
			prstrfl("Too many number registers.\n");
			casepr();	/* print out list of number registers */
		}
		if(++numerr > 1)done2(04); else edone(04);
	}
	return(j);
}

fnumb(i,f)
int i, (*f)();
{
	register j;

	j = 0;
	if(i < 0){
		j = (*f)('-' | nrbits);
		i = -i;
	}
	switch(nform){
		default:
		case '1':
		case 0: return(decml(i,f) + j);
		case 'i':
		case 'I': return(roman(i,f) + j);
		case 'a':
		case 'A': return(abc(i,f) + j);
	}
}

decml(i,f)
int i, (*f)();
{
	register j,k;

	k = 0;
	nform--;
	if((j=i/10) || (nform > 0))k = decml(j,f);
	return(k + (*f)((i%10 + '0') | nrbits));
}

roman(i,f)
int i, (*f)();
{

	if(!i)return((*f)('0' | nrbits));
	if(nform == 'i')return(roman0(i,f,"ixcmz","vldw"));
	else return(roman0(i,f,"IXCMZ","VLDW"));
}

roman0(i,f,onesp,fivesp)
int i, (*f)();
char *onesp, *fivesp;
{
	register q, rem, k;

	k = 0;
	if(!i)return(0);
	k = roman0(i/10,f,onesp+1,fivesp+1);
	q = (i=i%10)/5;
	rem = i%5;
	if(rem == 4){
		k =+ (*f)(*onesp | nrbits);
		if(q)i = *(onesp+1);
			else i = *fivesp;
		return(k =+ (*f)(i | nrbits));
	}
	if(q)k =+ (*f)(*fivesp | nrbits);
	while(--rem >= 0)
		k =+ (*f)(*onesp | nrbits);
	return(k);
}

abc(i,f)
int i, (*f)();
{
	if(!i)return((*f)('0' | nrbits));
	else return(abc0(i-1,f));
}

abc0(i,f)
int i, (*f)();
{
	register j, k;

	k = 0;
	if(j=i/26)k = abc0(j-1,f);
	return(k + (*f)((i%26 + nform) | nrbits));
}

wrc(i)
int i;
{
	if(cp >= &cbuf[NC])return(0);
	*cp++ = i;
	return(1);
}

atoin(){
	register i;
	extern LONG0 atoi0();

	return(i=atoi0());
}

LONG0 atoi0()
{
	register ii, k, cnt;
	LONG0 i, acc;
	extern LONG0 ckph();

	i = 0; acc = 0;
	nonumb = 0;
	cnt = -1;
a0:
	cnt++;
	switch((ii=getch()) & CMASK){
		default:
			ch = ii;
			if(cnt)break;
		case '+':
			i = ckph();
			if(nonumb)break;
			acc =+ i;
			goto a0;
		case '-':
			i = ckph();
			if(nonumb)break;
			acc =- i;
			goto a0;
		case '*':
			i = ckph();
			if(nonumb)break;
			acc =* i;
			goto a0;
		case '/':
			i = ckph();
			if(nonumb)break;
			if(i == 0){
				prstrfl("Divide by zero.\n");
				acc = 0;
			}else acc =/ i;
			goto a0;
		case '%':
			i = ckph();
			if(nonumb)break;
			acc =% i;
			goto a0;
		case '&':	/*and*/
			i = ckph();
			if(nonumb)break;
			if((acc > 0) && (i > 0))acc = 1; else acc = 0;
			goto a0;
		case ':':	/*or*/
			i = ckph();
			if(nonumb)break;
			if((acc > 0) || (i > 0))acc = 1; else acc = 0;
			goto a0;
		case '=':
			if(((ii=getch()) & CMASK) != '=')ch = ii;
			i = ckph();
			if(nonumb){acc = 0; break;}
			if(i == acc)acc = 1;
			else acc = 0;
			goto a0;
		case '>':
			k = 0;
			if(((ii=getch()) & CMASK) == '=')k++; else ch =ii;
			i = ckph();
			if(nonumb){acc = 0; break;}
			if(acc > (i - k))acc = 1; else acc = 0;
			goto a0;
		case '<':
			k = 0;
			if(((ii=getch()) & CMASK) == '=')k++; else ch =ii;
			i = ckph();
			if(nonumb){acc = 0; break;}
			if(acc < (i + k))acc = 1; else acc = 0;
			goto a0;
		case ')': break;
		case '(':
			acc = atoi0();
			goto a0;
	}
	return(acc);
}

LONG0 ckph(){
	register i;
	LONG0 j;
	extern LONG0 atoi0();
	extern LONG0 atoi1();

	if(((i = getch()) & CMASK) == '(')j = atoi0();
	else{
		ch = i;
		j = atoi1();
	}
	return(j);
}

LONG0 atoi1()
{
	register i, j, digits;
	long acc;
	int neg, abs, field;

	neg = abs = field = digits = 0;
	acc = 0;
a0:
	switch((i = getch()) & CMASK){
		default:
			ch = i;
			break;
		case '+':
			goto a0;
		case '-':
			neg = 1;
			goto a0;
		case '|':
			abs = 1 + neg;
			neg = 0;
			goto a0;
	}
a1:
	while(((j = ((i = getch()) & CMASK) - '0') >= 0) && (j <= 9)){
		field++;
		digits++;
		acc = 10*acc + j;
	}
	if((i & CMASK) == '.'){
		field++;
		digits = 0;
		goto a1;
	}
	ch = i;
	if(!field)goto a2;
	switch((i = getch()) & CMASK){
		case 'u':
			i = j = 1;
			break;
		case 'v':	/*VSs - vert spacing*/
			j = lss;
			i = 1;
			break;
		case 'm':	/*Ems*/
			j = EM;
			i = 1;
			break;
		case 'n':	/*Ens*/
			j = EM;
#ifndef NROFF
			i = 2;
#endif
#ifdef NROFF
			i = 1;	/*Same as Ems in NROFF*/
#endif
			break;
		case 'p':	/*Points*/
			j = INCH;
			i = 72;
			break;
		case 'i':	/*Inches*/
			j = INCH;
			i = 1;
			break;
		case 'c':	/*Centimeters*/
			j = INCH*50;
			i = 127;
			break;
		case 'P':	/*Picas*/
			j = INCH;
			i = 6;
			break;
		default:
			j = dfact;
			ch = i;
			i = dfactd;
	}
	if(neg) acc = -acc;
	if(!noscale){
		acc = (acc*j)/i;
	}
	if((field != digits) && (digits > 0))while(digits--)acc =/ 10;
	if(abs){
		if(dip->op > -1)j = dip->dnl; else j = v.nl;
		if(!vflag)j = v.hp;
		if(abs == 2)j = -j;
		acc =- j;
	}
a2:
	nonumb = !field;
	return(acc);
}

caserr(){
	register i,j;

	lgf++;
	skip();
	if((i=getrq()) == 0)return;
	for(j=NNAMES; j<NN; j++){  /*NNAMES predefined names*/
		if(i == r[j])break;
	}
	if(j!=NN){
		r[j]=vlist[j]=inc[j]=fmt[j]=0;
	}
}


casenr(){
	register i, j;

	lgf++;
	skip();
	if((i = findr(getrq())) == -1)goto rtn;
	skip();
	j = inumb(&vlist[i]);
	if(nonumb)goto rtn;
	vlist[i] = j;
	skip();
	j = atoin();
	if(nonumb)goto rtn;
	inc[i] = j;
rtn:
	return;
}

casepr(){
	register j; char buf[20];
	register char *p;
	for(j=NNAMES; j<NN; j++)
	{
		if(r[j] != 0)
		{
			p = &buf[0];
			*p++ = r[j] & 0177;
			if(!((*p = r[j] >> BYTE) & 0177)) *p = ' ';	/* one character name */;
			p++;
			*p++ = ' ';
			kvt(vlist[j],p);	/* value of number register */
			while(*p != '\n') p++;
			*p++ = ' ';
			*p++ = (fmt[j] <= 9) ? (fmt[j] ? fmt[j] + '0' : '1') : fmt[j];	/* format of number register */
			*p++ = ' ';
			kvt(inc[j],p);	/* increment value of number regester */
			prstr(&buf[0]);
		}
	}
}

caseaf(){
	register i, j, k;

	lgf++;
	if(skip() || !(i = getrq()) || skip())return;
	k = 0;
	if(!alph(j=getch())){
		ch = j;
		while(((j = getch() & CMASK) >= '0') &&
			(j <= '9'))k++;
	}
	if(!k)k=j;
	fmt[findr(i)] = k & BMASK;
}

vnumb(i)
int *i;
{
	vflag++;
	dfact = lss;
	res = VERT;
	return(inumb(i));
}

hnumb(i)
int *i;
{
	dfact = EM;
	res = HOR;
	return(inumb(i));
}

inumb(n)
int *n;
{
	register i, j, f;

	f = 0;
	if(n){
	if((j = (i = getch()) & CMASK) == '+')f = 1;
		else if(j == '-')f = -1;
			else ch = i;
	}
	i = atoin();
	if(n && f)i = *n + f*i;
	i = quant(i,res);
	vflag = 0;
	res = dfactd = dfact = 1;
	if(nonumb)i = 0;
	return(i);
}

quant(n,m)
int n, m;
{
	register i, neg;

	neg = 0;
	if(n<0){
		neg++;
		n = -n;
	}
	i = n/m;
	if((n - m*i) > (m/2))i =+ 1;
	i =* m;
	if(neg)i = -i;
	return(i);
}
