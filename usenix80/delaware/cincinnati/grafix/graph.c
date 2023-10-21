#define	INF	1.e+37
#define	F	.25

/* following structure is arranged as two interleaved structures,
 * one for x and one for y to permit growing core point-by-point
 * It would be better to separate out the front matter from the
 * val substructure and so eliminate much of this funny business
*/

struct xy {
	int	xlbf,xubf,	ylbf,yubf;	int pada;
	int	xqf,pad1,	yqf,pad2;	int padb;
	float	xa,		ya;		int padc;
	float	xb,		yb;		int padd;
	float	xlb,		ylb;		int pade;
	float	xub,		yub;		int padf;
	float	xquant,		yquant;		int padg;
	float	xmult,		ymult;		int padh;
	struct {
		float xv,	yv;		int lbptr;
	} val[1];
} *xy;

/* labels are stored in a floating buffer at the
 * top of memory, slab=size, nlab=amt in use
*/
int slab 0;
int nlab 0;
#define B 64
#define BUMP (B*sizeof(xy->val[0]))

float xsize 1.;
float ysize 1.;
float xoffset 0.;
float yoffset 0.;
int tick 50;
int top 1940;
int bot -1860;
int xbot;
int ybot;
float absbot;
int cx,cy,ty;
int	n;
int	framef;
int	gridf;
int	connf;
int	symbf;
int	absf;
float	dx;
char	*plotsymb;

double	atof();
double	floor();
double	ceil();
char 	*sbrk();
int	fin;
#define BSIZ 80
char	labbuf[BSIZ];

main(argc,argv)
char *argv[];
{
	extern int fout;

	fout = 1;
	space(-2048,-2048,2048,2048);
	xy = sbrk( sizeof(xy[0]) );
	setopt(argc,argv);
	if(framef)
		erase();
	readin();
	scale();
	axes();
	plot();
	move(-2047,-2047);
	closevt();
	return(0);
}

setopt(argc,argv)
char *argv[];
{
	int hflag;
	char *p1, *p2;

	hflag = 0;
	framef = 1;
	connf = 1;
	gridf = 2;
	symbf = 0;
	absf = 0;
	xy->xlbf = xy->xubf = xy->ylbf = xy->yubf = 0;
	xy->xqf = xy->yqf = 0;
	xy->xlb = xy->ylb = INF;
	xy->xub = xy->yub = -INF;
	xy->val[0].xv = xy->val[0].yv = 0;
	while(--argc > 0) {
		argv++;
again:		switch(argv[0][0]) {
		case '-':
			argv[0]++;
			goto again;
		case 'l': /* label for plot */
			p1 = labbuf;
			if (argc>=2) {
				argv++;
				argc--;
				p2 = argv[0];
				while (*p1++ = *p2++);
			}
			break;

		case 'a': /*automatic abscissas*/
			absf = 1;
			dx = 1;
			if(!numb(&dx,&argc,&argv)) break;
			if(numb(&absbot,&argc,&argv))
				absf = 2;
			break;

		case 'd':/*disconnected, no lines between points*/
			connf = 0;
			break;

		case 's': /*save screen, overlay plot*/
			framef = 0;
			break;

		case 'g': /*grid style 0 none, 1 ticks, 2 full*/
			gridf = 0;
			if(argv[0][1])
				gridf = argv[0][1]-'0';
			break;

		case 'c': /*character(s) for plotting*/
			if(argc >= 2) {
				symbf = 1;
				plotsymb = argv[1];
				argv++;
				argc--;
			}
			break;

		case 'x':	/*x limits */
			limread(&xy->xlbf,&argc,&argv);
			break;
		case 'y':
			limread(&xy->ylbf,&argc,&argv);
			break;
		case 'h': /*set height of plot */
			numb(&ysize, &argc,&argv);
			break;
		case 'w': /*set width of plot */
			numb(&xsize, &argc, &argv);
			break;
		case 'r': /* set offset to right */
			numb(&xoffset, &argc, &argv);
			break;
		case 'u': /*set offset up the screen*/
			numb(&yoffset,&argc,&argv);
			break;
		default:
			badarg();
		}
	}
	if(absf && xy->xlbf==0) {
		xy->xlb = 0;
		xy->xlbf = 1;
	}
}

limread(p, argcp, argvp)
register struct xy *p;
{
	if(!numb(&p->xlb,argcp,argvp))
		return;
	p->xlbf = 1;
	if(!numb(&p->xub,argcp,argvp))
		return;
	p->xubf = 1;
	if(!numb(&p->xquant,argcp,argvp))
		return;
	p->xqf = 1;
}

numb(np, argcp, argvp)
int *argcp;
float *np;
register char ***argvp;
{
	register char c;

	if(*argcp <= 1)
		return(0);
	while((c=(*argvp)[1][0]) == '+')
		(*argvp)[1]++;
	if(!(digit(c) || c=='-'&&(*argvp)[1][1]<'A' || c=='.'))
		return(0);
	*np = atof((*argvp)[1]);
	(*argcp)--;
	(*argvp)++;
	return(1);
}

readin()
{
	register i;
	register t;

	fin = dup(0);
	close(0);
	if(absf == 1)absbot = xy->xlb;
	for(;;) {
		if(more()==0)
			return;
		for(i=0; i<B; i++) {
			if(absf)
				xy->val[n].xv = n*dx + absbot;
			else
				if(!getfloat(&xy->val[n].xv))
					return;
			if(!getfloat(&xy->val[n].yv))
				return;
			xy->val[n].lbptr = -1;
			t = getstring();
			if(t>0)
				xy->val[n].lbptr = copystring(t);
			n++;
			if(t<0)
				return;
		}
	}
}

more()
{
	register i;
	register char *p;

	if(sbrk(BUMP) == -1) {
		write(2,"points omitted\n",15);
		return(0);
	}
	p = sbrk(0);
	for(i=0;i<slab;i++) {
		--p;
		p[0] = p[-BUMP];
	}
	return(1);
}

copystring(k)
{
	register char *p;
	register i;
	int q;

	if(k+1+nlab>slab) {
		if(more()==0)
			return;
		slab =+ BUMP;
	}
	p = sbrk(0);
	q = nlab;
	for(i=0;i<=k;i++)
		*(p-slab+nlab++) = labbuf[i];
	return(q);
}

float
modceil(f,t)
float f,t;
{

	return(ceil(f/t)*t);
}

float
modfloor(f,t)
float f,t;
{
	return(floor(f/t)*t);
}

getlim(p)
register struct xy *p;
{
	register i;

	i = 0;
	do {
		if(!p->xlbf && p->xlb>(p->val[i].xv))
			p->xlb = p->val[i].xv;
		if(!p->xubf && p->xub<(p->val[i].xv))
			p->xub = p->val[i].xv;
		i++;
	} 
	while(i < n);
}

setlim(p)
register struct xy *p;
{
	float r,s,t,delta,sign;
	float lb,ub;
	int lbf,ubf;

	lb = p->xlb;
	ub = p->xub;
	delta = ub-lb;
	if(p->xqf) {
		if(delta*p->xquant <=0 )
			badarg();
		p->xmult = 1;
		return;
	}
	if(delta == 0) {
		if(ub > 0) {
			ub = 2*ub;
			lb = 0;
		} 
		else
			if(lb < 0) {
				lb = 2*lb;
				ub = 0;
			} 
			else {
				ub = 1;
				lb = -1;
			}
		delta = ub-lb;
	}
	sign = 1;
	lbf = p->xlbf;
	ubf = p->xubf;
	if(delta < 0) {
		sign = -1;
		t = lb;
		lb = ub;
		ub = t;
		t = lbf;
		lbf = ubf;
		ubf = t;
		delta = -delta;
	}
	r = s = 1;
	while(delta*s < 1)
		s =* 10;
	delta =* s;
	while(10*r < delta)
		r =* 10;
	lb =* s;
	ub =* s;
	if(r>=5*delta/6)
		r =/ 5;
	else if(r>=delta/2)
		r =/ 2;
	else if(r<delta/5)
		r =* 2;
	if(!ubf)
		ub = modceil(ub,r);
	if(!lbf)
		lb = modfloor(lb,r);
	if(!lbf && ub<=6*r && lb>0)
		lb = 0;
	else if(!ubf && lb>=-6*r && ub<0)
		ub = 0;
	if(sign > 0) {
		p->xlb = lb;
		p->xub = ub;
		p->xquant = r;
	} 
	else {
		p->xlb = ub;
		p->xub = lb;
		p->xquant = -r;
	}
	p->xmult = s;
}

scale()
{
	float edge;
	register struct xy *p;

	p = xy;
	getlim(&p->xlbf);
	setlim(&p->xlbf);
	getlim(&p->ylbf);
	setlim(&p->ylbf);
	edge = top-bot;
	p->xa = xsize*edge/(xy->xub-xy->xlb);
	xbot = bot + edge*xoffset;
	p->xb = xbot - xy->xlb*xy->xa;
	p->ya = ysize*edge/(p->yub-p->ylb);
	ybot = bot + edge*yoffset;
	p->yb = ybot - p->ylb*p->ya;
}

axes()
{
	int ix,ix0,ix1,iy,iy0,iy1;
	float h,v;
	extern float quant();

	ix0 = xbot;
	iy0 = ybot;
	ix1 = xbot + (top-bot)*xsize;
	iy1 = ybot + (top-bot)*ysize;
	if(gridf==2) {
		for(h=quant(&xy->xlbf); lim(h,&xy->xlbf); h=+xy->xquant) {
			ix = h*xy->xa+xy->xb;
			line(ix,iy0,ix,iy1);
		}
		for(v=quant(&xy->ylbf); lim(v,&xy->ylbf); v=+xy->yquant) {
			iy = v*xy->ya+xy->yb;
			line(ix0,iy,ix1,iy);
		}
	}
	if(gridf==1) {
		h = 0;
		if((xy->xlb < 0 && xy->xub <= 0) ||
		   (xy->xlb >= 0 && xy->xub >= 0))
			h = xy->xlb;
		ix = h*xy->xa+xy->xb;
		line(ix,iy0,ix,iy1);
		for(v=quant(&xy->ylbf); lim(v,&xy->ylbf); v=+xy->yquant) {
			iy = v*xy->ya+xy->yb;
			line(ix-tick,iy,ix+tick,iy);
		}
		v = 0;
		if((xy->ylb < 0 && xy->yub <= 0) || (xy->ylb >= 0 && xy->yub >= 0))
			v = xy->ylb;
		iy = v*xy->ya+xy->yb;
		line(ix0,iy,ix1,iy);
		for(h=quant(&xy->xlbf); lim(h,&xy->xlbf); h=+xy->xquant) {
			ix = h*xy->xa+xy->xb;
			line(ix,iy-tick,ix,iy+tick);
		}
	}
	if(gridf)
		title();
}


float quant(p)
register struct xy *p;
{
	if(p->xquant>0)
		return(modceil(p->xlb,p->xquant));
	else
		return(modfloor(p->xlb,p->xquant));
}

lim(l,p)
float l;
struct xy *p;
{

	l =- p->xub + p->xquant/2;
	if(p->xquant > 0)
		return(l<0);
	return(l>0);
}

plot()
{
	int ix,iy,lx,ly;
	int i;
	int first;

	first = 0;
	for(i=0; i<n; i++) {
		if(!inlim(xy->val[i].xv,&xy->xlbf)||
		   !inlim(xy->val[i].yv,&xy->ylbf)) {
			first = 0;
			continue;
		}
		ix = xy->xa*xy->val[i].xv*xy->xmult+xy->xb;
		iy = xy->ya*xy->val[i].yv*xy->ymult+xy->yb;
		if(connf) {
			if(first > 0)
				line(ix,iy,lx,ly);
			lx = ix;
			ly = iy;
			first = 1;
		}
		symbol(ix,iy,xy->val[i].lbptr);
	}
}

inlim(xv,p)
float xv;
register struct xy *p;
{
	if(!p->xlbf&&!p->xubf)
		return(1);
	if(p->xquant >0)
		return(xv>=p->xlb&xv<=p->xub);
	else
		return(xv>=p->xub&xv<=p->xlb);
}

char savchar ' ';
getfloat(p)
float *p;
{
	register i;

	while(blank(savchar)) {
		if((savchar=getchar())==0)
			return(0);
	}
	for(i=0; i<BSIZ-1; ) {
		labbuf[i] = savchar;
		if((savchar=getchar())==0)
			return(0);
		i++;
		if(digit(savchar))
			continue;
		switch(savchar) {
		case '.':
		case '+':
		case '-':
		case 'E':
		case 'e':
			continue;
		}
		break;
	}
	labbuf[i] = ' ';
	*p = atof(labbuf);
	return(1);
}

getstring()
{
	register i;

	i = 0;
	labbuf[0] = 0;
	while(blank(savchar)) {
		if((savchar=getchar())==0)
			return(-1);
	}
	if(digit(savchar))
		return(0);
	switch(savchar) {
	case '.':
	case '+':
	case '-':
		return(0);
	}
	if(savchar=='"') {
		while(i<BSIZ-1) {
			switch(savchar=getchar()){
			case 0:
				return(-1);
			case '"':
			case '\n':
				break;
			default:
				labbuf[i++] = savchar;
				continue;
			}
			break;
		}
	} else {
		for(;;) {
			labbuf[i++] = savchar;
			if((savchar=getchar())==0)
				return(-1);
			if(blank(savchar))
				break;
		}
	}
	labbuf[i] = 0;
	return(i);
}

digit(c)
{
	return('0'<=c&c<='9');
}

blank(c)
{
	switch(c) {
	case ' ':
	case '\t':
	case '\n':
		return(1);
	}
	return(0);
}

symbol(ix,iy,k)
{

	if(symbf==0&&k<0) {
		if(connf==0)
			point(ix,iy);
	} 
	else {
		move(ix,iy);
		label(k>=0?sbrk(0)-slab+k:plotsymb);
	}
}

title()
{
	move(xbot,ybot-60);
	bound(xy->xlb,&xy->xlbf);
	label(" -x- ");
	bound(xy->xub,&xy->xlbf);
	label("      ");
	bound(xy->ylb,&xy->ylbf);
	label(" -y- ");
	bound(xy->yub,&xy->ylbf);
	if (labbuf[0]) {
		label("       ");
		label(labbuf);
	}
}

char	putbuf[30];
int	putpos;

bound(f,p)
float f;
struct xy *p;
{

	fconv(putbuf,f/p->xmult);
	label(putbuf);
}

fconv(cp,f)
	double f;
	char *cp;
	{
	char *cdig;
	int decpt, sign, i;
	cdig = fcvt(f, 6, &decpt, &sign);
	if (sign)
		*cp++ = '-';
	if(decpt<0) 
		*cp++ = '.';
	for(i=0;i>decpt;i--)
		*cp++ = '0';
	for (i=0; i < 6; i++)
		{
		if (i== decpt)
			*cp++ = '.';
		*cp++ = *cdig++;
		}
	*cp = '\0';
	}

badarg()
{
	write(2,"bad argument\n",13);
	exit(1);
}
