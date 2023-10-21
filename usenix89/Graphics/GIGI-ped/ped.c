/*
 *	ped - the picture editor for GIGIs.
 *	Col. G. L. Sicherman (decvax!sunybcs!colonel).  Circa 1984.
 *	Version 1. (This does not imply that any newer versions will
 *	be developed!)
 *
 *	Copyright 1987 by G. L. Sicherman.  You may use and alter
 *	this program however you like, so long as you do not claim
 *	it as your own, try to get money for it, or alter this
 *	message.
 *
 *	This is meant for UNIX BSD4.  It has not been extensively
 *	tested.  It comes with no guarantees.
 */

#include <sgtty.h>
#include <signal.h>

#include <ctype.h>
#include <stdio.h>

#include "gigi.h"

/*
 *	Shell escape.
 */

#define SHELL "/bin/csh"

/*
 *	Storage parameters.
 */

#define MAXPOINTS 300
#define MAXLINES 384
#define MAXCIRC 50
#define MAXOPECURV 100
#define MAXLOOPS 100

char *version = "1";

int x=300, y=200;	/* STARTING POINT */
int xfac=1, yfac=1;
int vx, vy, vp;
int cx, cy, cp;
struct spl_ *cs;
int npoints=0, nlines=0, nsplines=0, ncirc=0, nloops=0;
int grid=0;	/* DRAW A GRID? */
int labelpts=1;	/* LABEL THE POINTS? */
char splinetype;	/* S FOR OPEN, B FOR CLOSED */
int vector=0;	/* DRAWING VECTORS? */
int curve=0;	/* DRAWING SPLINE? */
int circle=0;	/* DRAWING CIRCLE? */
int moving=0;	/* RELOCATING A POINT? */
int param=0;
int nchanges = 0;
char fname[72];
FILE *fstr;
int atpoint=0;
struct sgttyb oldt, newt;
struct ct {
	int cce, cci;
}
circ[MAXCIRC];
struct pt {
	int px, py;
} 
point[MAXPOINTS];
struct ln {
	int l0, l1;
} 
line[MAXLINES];
struct spl_ {
	int s0;
	struct spl_ *s1;
} 
*spline[MAXOPECURV], *loop[MAXLOOPS];

main(argc,argv)
int argc;
char **argv;
{
	int i, quit();
	int nquits = 0, nexpunges = 0;
	char cmd, oldcmd='q';
	if (argc>2) {
		fprintf(stderr,"usage: ped [file]\n");
		exit(1);
	}
	gtty(0,&oldt);
	signal(SIGINT,quit);
	newt=oldt;
	newt.sg_flags |= CBREAK;
	newt.sg_flags &= ~ECHO;
	stty(0,&newt);
	puts(GR_ON,stdout);
	puts(RESET,stdout);
	printf("S(E)(I(D))\n");
	if (argc==2) {
		loadfrom(argv[1]);
		redraw(stdout);
	}
	mvcur();
/*
 *	The command loop.
 */
	for (;;) {
		if (1>read(0,&cmd,1)) quit();
		if (cmd!='q') nquits=0;
		if (cmd!='x') nexpunges=0;
		if (cmd!='h' && cmd!='l' && !isdiag(cmd)) xfac=1;
		if (cmd!='j' && cmd!='k' && !isdiag(cmd)) yfac=1;
		if (isorth(cmd) || isdiag(cmd)) atpoint=0;
		if (cmd!=oldcmd) xfac=yfac=1;
		if (isdigit(cmd)) {
			param=param*10+cmd-'0';
			continue;
		}
		switch (cmd) {
		case '?':
			help();
			break;
		case '!':
			escape();
			break;
		case 'q':
			if (nchanges && !nquits++) beep();
			else quit();
			break;
		case 'r':
			redraw(stdout);
			break;
		case 's':
			scaleup();
			break;
		case 'g':
			grid = !grid;
			redraw(stdout);
			break;
		case 'f':
			for (i=0; i<npoints; i++) point[i].py=479-point[i].py;
			redraw(stdout);
			nchanges++;
			break;
		case 'J':
			if (!param) param=1;
			for (i=0; i<npoints; i++) point[i].py+=param;
			redraw(stdout);
			nchanges++;
			break;
		case 'K':
			if (!param) param=1;
			for (i=0; i<npoints; i++) point[i].py-=param;
			redraw(stdout);
			nchanges++;
			break;
		case 'L':
			dsplines();
			break;
		case 'm':
			if (param) {
				if (param>npoints) {
					beep();
					break;
				}
				gopoint(param);
			}
			else if (!atpoint) mkpoint();
			moving=atpoint;
			vector=0;
			printf("W(I%d)V[]W(I%d)\n",GR_RED,GR_WHITE);
			nchanges++;
			break;
		case 'k':
			hjkl(0,-1);
			break;
		case 'j':
			hjkl(0,1);
			break;
		case 'h':
			hjkl(-1,0);
			break;
		case 'l':
			hjkl(1,0);
			break;
		case 'y':
			hjkl(-1,-1);
			break;
		case 'u':
			hjkl(1,-1);
			break;
		case 'b':
			hjkl(-1,1);
			break;
		case 'n':
			hjkl(1,1);
			break;
		case 'e':
			printf("%s\n",GR_OFF);
			stty(0,&oldt);
			printf("File name:  ");
			if (1==scanf(" %s",fname)) loadfrom(fname);
			stty(0,&newt);
			printf("%s\n",GR_ON);
			redraw(stdout);
			break;
		case 'w':
			printf("%s\n",GR_OFF);
			stty(0,&oldt);
			printf("File name:  ");
			if (1==scanf(" %s",fname)) {
				fstr=fopen(fname,"w");
				if (fstr==NULL)
					printf("cannot open %s\n",fname);
				else {
					labelpts=0;
					fprintf(fstr,"%s\n",GR_ON);
					fprintf(fstr,"%s\n",RESET);
					strucdump(fstr);
					redraw(fstr);
					fprintf(fstr,"%s\n",GR_OFF);
					labelpts=1;
					fclose(fstr);
					printf("Dumped.\n");
				}
			}
			while ('\n'!=getchar());	/* FLUSH INPUT LINE */
			nchanges=0;
			stty(0,&newt);
			printf("%s\n",GR_ON);
			break;
		case 'x':
			if (!atpoint) beep();
			else if (!nexpunges++ && pointused(atpoint)) beep();
			else {
				delpoint(atpoint);
				redraw(stdout);
			}
			break;
		case 'V':
			for (i=1; i<=npoints; i++)
			if (!pointused(i)) delpoint(i);
			redraw(stdout);
			break;
		case 'o':
		case 'z':
			if (vector || curve || circle) beep();
			else {
				curve=1;
				splinetype = 'o'==cmd? 'S': 'B';
				cx=x;
				cy=y;
				if (!atpoint) mkpoint();
				cs = (struct spl_ *)malloc(sizeof(struct spl_));
				if (cmd=='o') spline[nsplines]=cs;
				else loop[nloops]=cs;
				cs->s0 = cp = atpoint;
				cs->s1 = NULL;
			}
			break;
		case 'c':
			if (vector || curve || circle) beep();
			else {
				circle=1;
				cx=x;
				cy=y;
			}
			if (!atpoint) mkpoint();
			cp=atpoint;
			break;
		case 'v':
			if (vector || curve || circle) beep();
			else {
				vector=1;
				vx=x;
				vy=y;
			}
			if (!atpoint) mkpoint();
			vp=atpoint;
			break;
		case 'p':
			if (param) {
				if (param>npoints || moving) {
					beep();
					break;
				}
				else gopoint(param);
			}
			else if (moving) {
				point[moving-1].px=x;
				point[moving-1].py=y;
				printf("V[]T'%d'P[%d,%d]\n",moving,x,y);
				moving=0;
			}
			else mkpoint();
			if (curve) xtndspline();
			else if (circle) {
				printf("P[%d,%d]C[%d,%d]\n",cx,cy,x,y);
				circ[ncirc].cce=cp;
				circ[ncirc].cci=param;
				ncirc++;
				nchanges++;
				circle=0;
			}
			else if (vector) {
				printf("P[%d,%d]V[%d,%d]\n",vx,vy,x,y);
				vx=x;
				vy=y;
				line[nlines].l0=vp;
				line[nlines].l1=param;
				nlines++;
				nchanges++;
				vp=param;
			}
			break;
		case 'i':
			if (vector) vector=0;
			else if (curve) {
				curve=0;
				if (!atpoint) {
					mkpoint();
					xtndspline();
				}
				if (splinetype=='S') nsplines++;
				else nloops++;
			}
			else beep();
			break;
		default:
			beep();
		}
		oldcmd=cmd;
		param=0;
	}
}

escape() {
	printf("%s\n",GR_OFF);
	system(SHELL);
	cntinu();
}

help() {
	printf("%s\n",GR_OFF);
	printf("?\tmenu\t\t\tn\tdown right\n");
	printf("J\tpicture down\n");
	printf("K\tpicture up\n");
	printf("L\tshow lines\t\to\topen curve\n");
	printf("V\tdelete unused points\tp\tpoint\n");
	printf("b\tdown left\t\tq\tquit\n");
	printf("c\tcircle\t\t\tr\tredraw screen\n");
	printf("e\tedit a ped file\t\ts\tscale up or down\n");
	printf("f\tflip picture\n");
	printf("g\tdraw/erase grid\t\tu\tup right\n");
	printf("h\tleft\t\t\tv\tbegin vectors\n");
	printf("i\tend vector/curve\tw\twrite to file\n");
	printf("j\tdown\t\t\tx\tremove point\n");
	printf("k\tup\t\t\ty\tup left\n");
	printf("l\tright\t\t\tz\tloop (closed curve)\n");
	printf("m\tmove point\n");
	cntinu();
}

mvcur() {
	printf("P[%d,%d]\n",x,y);
}

quit()
{
	puts(GR_OFF,stdout);
	stty(0,&oldt);
	exit(0);
}

beep()
{
	printf("\007");
	fflush(stdout);
}

redraw(stm)
FILE *stm;
{
	int i;
	fprintf(stm,";S(E)\n");
	if (labelpts && grid) {
		fprintf(stm,"W(I(G))\n");
		for (i=1; i<8; i++) fprintf(stm,"P[%d,1]V[,+470]\n",i*100);
		for (i=1; i<5; i++) fprintf(stm,"P[1,%d]V[+760]\n",i*100);
		fprintf(stm,"W(I(W))\n");
	}
	for (i=0; i<npoints; i++) {
		if (labelpts)
		fprintf(stm,"P[%d,%d]V[]T'%d'\n",point[i].px,point[i].py, i+1);
		fprintf(stm,"P[%d,%d]\n",point[i].px,point[i].py);
		x=point[i].px;
		y=point[i].py;
		atpoint=i;
	}
	for (i=0; i<nlines; i++) {
		fprintf(stm,"P[%d,%d]V[%d,%d]\n",
		point[line[i].l0-1].px,point[line[i].l0-1].py,
		point[line[i].l1-1].px,point[line[i].l1-1].py);
		x=point[line[i].l1-1].px;
		y=point[line[i].l1-1].py;
		atpoint=line[i].l1;
	}
	if (labelpts) fprintf(stm,"W(I%d)\n",GR_YELLOW);
	for (i=0; i<ncirc; i++) fprintf(stm,"P[%d,%d]C[%d,%d]\n",
		point[circ[i].cce-1].px, point[circ[i].cce-1].py,
		point[circ[i].cci-1].px, point[circ[i].cci-1].py);
	if (labelpts) fprintf(stm,"W(I%d)\n",GR_WHITE);
	for (i=0; i<nsplines; i++) drawspl(stm,'S',i+1);
	for (i=0; i<nloops; i++) drawspl(stm,'B',i+1);
}

mkpoint()
{
	point[npoints].px=x;
	point[npoints].py=y;
	printf("V[]T'%d'P[%d,%d]\n",++npoints,x,y);
	atpoint=param=npoints;
	nchanges++;
}

dsplines() {
	int i;
	puts(GR_OFF,stdout);
	printf("LineNo\tStart\tX\tY\tEnd\tX\tY\n");
	for (i=0; i<nlines; i++) printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
	i+1,line[i].l0,point[line[i].l0-1].px,point[line[i].l0-1].py,
	line[i].l1,point[line[i].l1-1].px,point[line[i].l1-1].py);
	cntinu();
}

gopoint(param)
int param;
{
	x=point[param-1].px;
	y=point[param-1].py;
	mvcur();
	atpoint=param;
}

xtndspline()
{
	cs->s1 = (struct spl_ *)malloc(sizeof (struct spl_));
	cs=cs->s1;
	cs->s0 = atpoint;
	cs->s1 = NULL;
	drawspl(stdout,splinetype,(splinetype=='S'? nsplines: nloops)+1);
	nchanges++;
}

drawspl(stm,t,i)
char t;
int i;
FILE *stm;
{
	struct spl_ *s;
	if (t=='S') s = spline[i-1];
	else s = loop[i-1];
	fprintf(stm,"P[%d,%d]C(%c)\n",
	point[s->s0-1].px,point[s->s0-1].py,t);
	while (s = s->s1)
	    fprintf(stm,"[%d,%d]\n",
	x=point[(atpoint=s->s0)-1].px,y=point[s->s0-1].py);
	fprintf(stm,"C(E)\n");
}

hjkl(dx,dy)
int dx, dy;
{
	if (param) {
		x+=param*dx;
		xfac=1;
		y+=param*dy;
		yfac=1;
	}
	else {
		x+=xfac*dx;
		xfac*=2;
		if (xfac>100) xfac=100;
		y+=yfac*dy;
		yfac*=2;
		if (yfac>100) yfac=100;
	}
	if (x<0) x=0;
	if (x>767) x=767;
	if (y<0) y=0;
	mvcur();
}

int
isorth(c)
char c;
{
	return c=='h' || c=='j' || c=='k' || c=='l';
}

int
isdiag(c)
char c;
{
	return c=='y' || c=='u' || c=='b' || c=='n';
}

cntinu()
{
	char c;
	printf("\nType space to continue");
	fflush(stdout);
	while (1==read(0,&c,1)) if (c==' ') break;
	puts(GR_ON,stdout);
	redraw(stdout);
}

strucdump(str)
FILE *str;
{
	int i;
	struct spl_ *s;
	fprintf(str,";\"\n");
	fprintf(str,"%d\n",npoints);
	for (i=0; i<npoints; i++) fprintf(str,"%d %d\n",point[i].px,point[i].py);
	fprintf(str,"%d\n",nlines);
	for (i=0; i<nlines; i++) fprintf(str,"%d %d\n",line[i].l0,line[i].l1);
	fprintf(str,"%d\n",ncirc);
	for (i=0; i<ncirc; i++) fprintf(str,"%d %d\n",circ[i].cce,circ[i].cci);
	fprintf(str,"%d\n",nsplines);
	for (i=0; i<nsplines; i++) {
		for (s=spline[i]; s; s=s->s1) fprintf(str,"%d ",s->s0);
		fprintf(str,"-1 \n");
	}
	fprintf(str,"%d\n",nloops);
	for (i=0; i<nloops; i++) {
		for (s=loop[i]; s; s=s->s1) fprintf(str,"%d ",s->s0);
		fprintf(str,"-1 \n");
	}
	fprintf(str,"\"\n");
}

loadfrom(fname)
char *fname;
{
	FILE *str;
	char k;
	int i, j;
	struct spl_ *s;
	str=fopen(fname,"r");
	if (str==NULL) {
		printf("cannot read %s\n",fname);
		return;
	}
	for (i=0; i<2; i++)
	while (';'!=(k=getc(str))) if (EOF==k) {
		printf("Error in file; load aborted.\n");
		return;
	}
	if ('"'!=getc(str)) {
		printf("Error in file; load aborted.\n");
		return;
	}
	fscanf(str," %d",&npoints);
	for (i=0; i<npoints; i++) fscanf(str," %d %d",&point[i].px,&point[i].py);
	fscanf(str," %d",&nlines);
	for (i=0; i<nlines; i++) fscanf(str," %d %d",&line[i].l0,&line[i].l1);
	fscanf(str," %d",&ncirc);
	for (i=0; i<ncirc; i++) fscanf(str," %d %d",&circ[i].cce,&circ[i].cci);
	fscanf(str," %d",&nsplines);
	for (i=0; i<nsplines; i++) {
		s=spline[i]=(struct spl_ *)malloc(sizeof (struct spl_));
		fscanf(str," %d",&(s->s0));
		for (;;) {
			fscanf(str," %d",&j);
			if (j<0) break;
			s=s->s1=(struct spl_ *)malloc(sizeof (struct spl_));
			s->s0=j;
		}
		s->s1=0;
	}
	fscanf(str," %d",&nloops);
	for (i=0; i<nloops; i++) {
		s=loop[i]=(struct spl_ *)malloc(sizeof (struct spl_));
		fscanf(str," %d",&(s->s0));
		for (;;) {
			fscanf(str," %d",&j);
			if (j<0) break;
			s=s->s1=(struct spl_ *)malloc(sizeof (struct spl_));
			s->s0=j;
		}
		s->s1=0;
	}
	fclose(str);
}

scaleup()
/* SCALE THE WHOLE PICTURE UP OR DOWN */
{
	int i;
	double fac;
	printf("%s\n",GR_OFF);
	stty(0,&oldt);
	printf("Scale factor:  ");
	if (1==scanf(" %lf",&fac)) {
		if (0.0==fac) printf("That's too small\n");
		else for (i=0; i<npoints; i++) scalpoint(&point[i],fac);
	}
	else printf("* Ignored *\n");
	while ('\n'!=getchar());	/* FLUSH INPUT LINE */
	stty(0,&newt);
	printf("%s\n",GR_ON);
	redraw(stdout);
}

scalpoint(p,f)
struct pt *p;
double f;
{
	int newx, newy;
	newx=384+f*(p->px-384);
	newy=240+f*(p->py-240);
	if (newx<0) {
		newy=((p->py-240)*(p->px-384))/384 + 240;
		newx=0;
	}
	else if (newx>767) {
		newy=((p->py-240)*(p->px-384))/384 + 240;
		newx=767;
	}
	if (newy<0) {
		newx=((p->px-384)*(p->py-240))/240 + 384;
		newy=0;
	}
	else if (newy>479) {
		newx=((p->py-384)*(p->px-240))/240 + 384;
		newy=479;
	}
	p->px=newx;
	p->py=newy;
}

delpoint(n)
int n;
/* DELETE POINT N AND RENUMBER REMAINING POINTS */
{
	int i;
	struct spl_ *s;
	if (n>npoints) {
		beep();
		return;
	}
	if (pointused(n)) delrefs(n);
	for (i=n-1; i<npoints-1; i++) point[i]=point[i+1];
	for (i=0; i<nlines; i++) {
		if (line[i].l0>n) line[i].l0--;
		if (line[i].l1>n) line[i].l1--;
	}
	for (i=0; i<ncirc; i++) {
		if (circ[i].cce>n) circ[i].cce--;
		if (circ[i].cci>n) circ[i].cci--;
	}
	for (i=0; i<nsplines; i++) {
		s=spline[i];
		while (s) {
			if (s->s0>n) s->s0--;
			s=s->s1;
		}
	}
	for (i=0; i<nloops; i++) {
		s=loop[i];
		while (s) {
			if (s->s0>n) s->s0--;
			s=s->s1;
		}
	}
	npoints--;
}

delrefs(n)
/* DELETE LINES & CURVES CONTAINING POINT n */
{
	int i, j;
	struct spl_ **s;
	for (i=0; i<nlines; i++)
	if (line[i].l0==n || line[i].l1==n) {
		for (j=i; j<nlines-1; j++) line[j]=line[j+1];
		nlines--;
		i--;
	}
	for (i=0; i<ncirc; i++)
	if (circ[i].cce==n || circ[i].cci==n) {
		for (j=i; j<ncirc-1; j++) circ[j]=circ[j+1];
		ncirc--;
		i--;
	}
	for (i=0; i<nsplines; i++) {
		s = &spline[i];
		while (*s) {
			if ((*s)->s0==n) *s = (*s)->s1;
			else s = &((*s)->s1);
		}
	}
	for (i=0; i<nloops; i++) {
		s = &loop[i];
		while (*s) {
			if ((*s)->s0==n) *s = (*s)->s1;
			else s = &((*s)->s1);
		}
	}
}

int
pointused(n)
int n;
/* IS POINT PART OF A LINE OR CURVE? */
{
	int i;
	struct spl_ *s;
	for (i=0; i<nlines; i++) if (line[i].l0==n || line[i].l1==n) return 1;
	for (i=0; i<ncirc; i++) if (circ[i].cce==n || circ[i].cci==n) return 1;
	for (i=0; i<nsplines; i++) {
		s=spline[i];
		while (s) {
			if (s->s0==n) return 1;
			s=s->s1;
		}
	}
	for (i=0; i<nloops; i++) {
		s=loop[i];
		while (s) {
			if (s->s0==n) return 1;
			s=s->s1;
		}
	}
	return 0;
}

mrgpts(i,j)
/* MERGE TWO POINTS INTO ONE */
int i, j;
{
	int k;
	struct spl_ *s;
	if (i>j) {
		k=i;
		i=j;
		j=k;
	}
	for (k=0; k<nlines; k++) {
		if (line[k].l0==j) line[k].l0=i;
		if (line[k].l1==j) line[k].l1=i;
	}
	for (k=0; k<ncirc; k++) {
		if (circ[k].cce==j) circ[k].cce=i;
		if (circ[k].cci==j) circ[k].cci=i;
	}
	for (k=0; k<nsplines; k++) {
		s=spline[k];
		while (s) {
			if (s->s0==j) s->s0=i;
			s=s->s1;
		}
	}
	for (k=0; k<nloops; k++) {
		s=loop[k];
		while (s) {
			if (s->s0==j) s->s0=i;
			s=s->s1;
		}
	}
	delpoint(j);
}
