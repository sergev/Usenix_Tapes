/*
 *	miscellaneous subroutines
 */

#define	FBUF	1000	/* floating buffer size */

FILE *in;

/* graph simulator
 * entries are formatted one point to a line
 * curve is always broken **after** a character string
 */
grplot(s)
char *s;
{
	float *x, *y;
	float xold;
	float thick;
	long dum, igraf, iclip, nout, noutm1;
	long lget();
	int id, ir;
	int state;
	char c[80];
	POINT sc,ro,off;
	POINT ll, ur, vgetp();

	if( (in=fopen(s, "r")) == NULL ) {
		yyerror("graphxy:couldnt open file");
		return(-1);
	}

	x = (float *) malloc ( (unsigned) (sizeof(float)*FBUF) );
	y = (float *) malloc ( (unsigned) (sizeof(float)*FBUF) );

	fseek(in, (long)0, 0);
	thick = vget("thick");
	iclip = lget("noclip");
	igraf = 0l;
	sc=vgetp("graph_scale");
	ro=vgetp("graph_rotate");
	off = vgetp("graph_offset");
	params();

	state = EOF + 1;
	nout = 0;
	ir = lget("repeat");
	if( ir <= 0 ) ir=1;
	while( state != EOF ) {
		if(nout>1) {
			x[0]=x[nout-1];
			y[0]=y[nout-1];
			nout = 1;
		}
		while( ((state = pcan(&x[nout],&y[nout],c)) != EOF)
			&& (++nout < FBUF)
			&& (state < 3) )
			;
		for (id=0;id<nout;id++) {
		xold=x[id];
		x[id]=off.xp+(ro.xp*sc.xp*x[id]+ro.yp*sc.yp*y[id]);
		y[id]=off.yp+(ro.xp*sc.yp*y[id]-ro.yp*sc.xp*xold);
		}
		noutm1 = nout;
		if( noutm1>1 )
			for(id=1; id<=ir; id++)
				nplot_(&noutm1, x, y, &igraf, &iclip, &thick,
				&dum, &dum);
		/* character plotting */

		if( state==3 && strcmp(c,"\"\"")!=0 && strcmp(c,"\" \"") != 0  ) {
			int i;
			POINT q;
			if(c[0]=='\"' && c[strlen(c)-1]== '\"') {
				int l;
				l=strlen(c)-2;
				for(i=0; i< l; i++) c[i]=c[i+1];
				c[l]='\0';
			}
			q.xp=x[nout-1];
			q.yp=y[nout-1];
			ll=vgetp("map.ll");
			ur=vgetp("map.ur");
			if( (q.xp-ll.xp)*(ur.xp-q.xp)>0.
			 && (q.yp-ll.yp)*(ur.yp-q.yp)>0. ) {
				vpush();
				vputp("x",q);
				textout(c);
				vpop();
			}
		}
		if(state==3) nout = 0;
	}
	fclose(in);
	free((char *)x);
	free((char *)y);
	return(nout);
}

int gscan(s)
char *s;
{
	char c[80];
	float xmin, xmax, ymin, ymax, x, y;
	long n;

	n=0l;
	if( (in=fopen(s,"r"))==NULL) {
		yyerror("gscan:couldnt open");
		return(-1);
	}
	fseek(in, 0l, 0);
	while( pcan(&x,&y,c)!=EOF ) {
		if(n<=0l) {
			xmin=xmax=x;
			ymin=ymax=y;
		} else {
			if(x>xmax) xmax=x;
			else if(x<xmin) xmin=x;
			if(y>ymax) ymax=y;
			else if(y<ymin) ymin=y;
		}
		n++;
	}
	fclose(in);
	vput("XMIN",xmin);
	vput("XMAX",xmax);
	vput("YMIN",ymin);
	vput("YMAX",ymax);
	vput("POINTS",(float)n);
	return(0);
}

int pcan(f1,f2,s)
float *f1, *f2;
char *s;
{
	char c;
	*s = '\0';
	if( fscanf(in,"%f%f",f1,f2) == EOF) return(EOF);
	while( (c=getc(in)) == ' ' || c == '\t') ;
	if(c == '\n') return(2);
	ungetc(c,in);
	while( (c=getc(in)) != '\n') {
		*s = c;
		s++;
	}
	--s;
	while( *s==' ' || *s=='\t' ) s--;
	s++;
	*s = '\0';
	return(3);
}

params()	/* pass miscellaneous control parameters */
{
	long lget();
	long l1;
	static long ssav= -1;
	static long csav= -1;
	static long lsav= -1;

	l1 = lget("speed");
	if(l1 != ssav ) {
		ssav=l1;
		pspeed_(&l1);
		}
	l1 = lget("color");
	if(l1 != csav) {
		selpen_(&l1);
		csav=l1;
		}
	l1 = lget("line");
	if(l1 != lsav) {
		ltype_(&l1);
		lsav=l1;
		}
	return(0);
}

init()
{
	long l1;
	if(ready==0) {
		ready++;
		l1=lget("rotate");
		initt_( &l1, dest, (long)strlen(dest) );
		page( from, to);
		map( lleft, uright);
	}
	return(0);
}

page(a,b)
POINT a,b;
{
	float f1, f2;
	f1 = b.xp - a.xp;
	f2 = b.yp - a.yp;
	setdim_( &f1, &f2, &a.xp, &a.yp );
	vputp("page.ll",a);
	vputp("page.ur",b);
	map(lleft,uright);
	return(0);
}

map(a,b)
POINT a,b;
{
	setscl_( &a.xp, &b.xp, &a.yp, &b.yp );
	vputp("map.ll",a);
	vputp("map.ur",b);
	{	/* scale factors patched in */
		float scale;
		POINT ul, ur;
		ul=vgetp("page.ll");
		ur=vgetp("page.ur");
		vput("XSCALE",scale=(ur.xp-ul.xp)/(b.xp-a.xp));
		vput("XOFF",ul.xp-scale*a.xp);
		vput("YSCALE",scale=(ur.yp-ul.yp)/(b.yp-a.yp));
		vput("YOFF",ul.yp-scale*a.yp);
	}
	return(0);
}

use(a,b)
POINT a,b;
{
	float f1;
	f1 = vget("margin");	/* margin dimension */
	params();
	xlimit_( &a.xp, &b.xp, &a.yp, &b.yp, &f1);
	vputp("use.ll",a);
	vputp("use.ur",b);
	return(0);
}

frame(a,b)
POINT a,b;
{
	long l1;
	float f1;
	int id, ir;
	f1 = vget("thick");
	l1 = lget("noclip");
	params();
	ir=lget("repeat");
	if(ir<=0) ir=1;
	for(id=1; id<=ir; id++)
		box_( &a.xp, &b.xp, &a.yp, &b.yp, &f1, &l1);
	vputp("frame.ll",a);
	vputp("frame.ur",b);
	return(0);
}

draw(a,b)
POINT a,b;
{
	long clip;
	float thick;
	int id, ir;
	thick = vget("thick");
	clip = lget( "noclip");
	params();
	ir=lget("repeat");
	if(ir<=0) ir=1;
	for(id=1; id<=ir; id++)
		line_( &a.xp, &a.yp, &b.xp, &b.yp, &thick, &clip);
	return(0);
}

dline(a,b)
POINT a;
int b;
{
	float l, t, dx, dy;
	POINT c,d;
	l=vget("length");
	t=vget("angle");
	dx=cos((double)(0.01745329*t))*l;
	dy=sin((double)(0.01745329*t))*l;
	d=a;
	c=a;
	if(b==0) {
		d.xp += dx;
		d.yp += dy;
	} else if( b==1 ) {
		c.xp -= dx;
		c.yp -= dy;
	} else {
		c.xp -= dx/2;
		c.yp -= dy/2;
		d.xp += dx/2;
		d.yp += dy/2;
	}
	draw(c,d);
	return(0);
}

subbox(a,b)
POINT a,b;
{
	float dx, dy;
	POINT c;

	vputp( "f.ll",a); vputp("f.ur",b);

	dx = b.xp - a.xp; dy = b.yp - a.yp; c=a;

	c.yp += dy/2.; vputp("f.cl",c);
	c.yp=b.yp; vputp("f.ul",c);
	c.xp += dx/2.; vputp("f.ct",c);
	c.yp -= dy/2.; vputp("f.cc",c);
	c.yp = a.yp;  vputp("f.cb",c);
	c=b; c.yp -= dy/2.; vputp("f.cr",c);
	c.yp = a.yp; vputp("f.lr",c);

	return(0);
}

ticks(a,b)
POINT a,b;
{
	float stic, dtic, tlen, thick;
	long style, number;
	int id, ir;

	stic = vget("totick1");
	dtic = vget("nexttick");
	tlen = vget("ticksize");
	thick = vget("tickthick");
	style = lget("style");
	number = lget("ticks");
	ir=lget("repeat");
	if(ir<=0) ir=1;

	if( number == 0l ) return(0);

	params();
	for(id=1; id<=ir; id++)
		tics_( &a.xp, &a.yp, &b.xp, &b.yp, &stic, &number,
		&dtic, &tlen, &thick, &style);
	return(0);
}

circle(c,r)
POINT c;
float r;
{
	long n, fill, clip, narc, nsize;
	int ir;
	float x, y, rad, thick, *xbf, *ybf;

	fill = lget("cfill");
	narc = lget("narcs");
	clip = lget("noclip");
	x = c.xp;
	y = c.yp;
	rad = r;
	thick = vget("thick");
	nsize = narc+1;

	xbf = (float *)malloc( (unsigned)(sizeof(float)*(nsize)) );
	ybf = (float *)malloc( (unsigned)(sizeof(float)*(nsize)) );

	params();
	ir=(int)lget("repeat");
	if( ir <= 0 ) ir=1;
	for(;ir>0;ir--)
		circle_(&x,&y,&rad,&narc,&fill,&clip,&thick,xbf,ybf,&nsize);
	free((char *)xbf);
	free((char *)ybf);
	return(0);
}


textout(s)
char *s;
{
	float vget();
	long lget();
	POINT vgetp();
	float f1, f2, f3;
	long l1, l2, l3;
	POINT tp;

	params();
	l1 = 0l;
	ltype_(&l1);
	f1 = vget("angle");
	l2 = lget("font");
	cfont_(&l2);
	chrdir_(&f1);
	f1 = vget("size");
	f2 = vget("aspect");
	f3 = vget("slant");
	chrsiz_(&f1, &f2, &f3);

	l3 = strlen(s);
	tp = vgetp("x");
	f1 = tp.xp;
	f2 = tp.yp;
	f3 = vget("angle");
	l1 = lget("cmode");
	l2 = lget("repeat");
	text_(&f1, &f2, &f3, &l1, s, &l2, l3);
	return(0);
}
/*
 *	subroutine plot written by D. Harvey July, 1980
 *
 *	this routine will read single precision floating point data from
 *	a file and plot a single curve for each call
 *
 *	inputs  - name	= pointer to a character string which contains the
 *			  data file name.  
 *		  ix	= integer array number which specifies which data
 *			  array be used as the x - coordinate
 *		  iy    = integer array number which specifies which data
 *			  array be used as the y - coordinate
 *
 *	note:	The data file must be written in the following way.  Data
 *		arrays are dumped onto disk using raw c io and arrays are
 *		separated by the floating point word 1.23456e-30.  The
 *		integer array number refers to the numerical order of the
 *		individual arrays (i.e. ix = 3 means the third array of the
 *		data file will be used as the x coordinates and the third
 *		array starts immediately after the second occurence of the
 *		word 1.23456e-30)
 */

#define	NARRAY	100
#define ENDARRAY 1.23456e-30

char oname[100] = "/dev/null";
int iarray;
long parray[NARRAY], narray[NARRAY];
int	fd;

plot(name,ffx,ffy)

char	name[];
POINT ffx, ffy;

{

	float *xbuf, *ybuf;
	int ix, iy;
	int i,j;
	long px,py,npoint,n;
	long dum,igraf,iclip;
	float thick;
	double fabs();

	ix=ffx.xp;
	iy = ffy.xp;
	xbuf = (float *) malloc((unsigned)(sizeof(float)*FBUF));
	ybuf = (float *) malloc((unsigned)(sizeof(float)*FBUF));

	thick = vget("thick");
	iclip = lget("noclip");
	igraf = 0l;
	params();

/*
 *	check to see if file previously opened - if so skip opening
 *	and array pointer determination
 */

	if (strcmp(name,oname) != 0)  {
		if ((fd=open(name, 0)) < 0) {
			fprintf(stderr, "plot: cant't open %s\n", name);
			exit(1);
			}
		strcpy(oname,name);
		lseek(fd, 0L, 0);
		iarray = 0;
		narray[0] = 0;
		parray[0] = 0;
		parray[1] = 0;
		while (read(fd,(char *)xbuf,4) == 4)  {
			if ( fabs(xbuf[0]-ENDARRAY) < 0.1*ENDARRAY )  {
				iarray += 1;
				parray[iarray] = parray[iarray] + 4;
				narray[iarray] = 0;
			}
			else  {
				parray[iarray+1] += 4;
				narray[iarray] += 1;
			}
		}
	}

/*
 *	load plot buffers and plot
 */

	px = parray[ix-1];
	py = parray[iy-1];
	npoint = narray[ix-1];
	if (narray[iy-1] < npoint) npoint = narray[iy-1];
	j = npoint/FBUF;
	n = FBUF;
	for (i=0; i<j; i++)  {
		lseek(fd, px, 0);
		read(fd, (char *)xbuf, 4*FBUF);
		lseek(fd, py, 0);
		read(fd, (char *)ybuf, 4*FBUF);
		nplot_(&n,xbuf,ybuf,&igraf,&iclip,&thick,&dum,&dum);
		px += 4*FBUF - 4;
		py += 4*FBUF - 4;
		}
	n = 4*npoint - px + parray[ix-1];
	n = n/4;
	if (n > 2) {
		i = 4*n;
		lseek(fd, px, 0);
		read(fd, (char *)xbuf, i);
		lseek(fd, py, 0);
		read(fd, (char *)ybuf, i);
		nplot_(&n,xbuf,ybuf,&igraf,&iclip,&thick,&dum,&dum);
	}
	free( (char *)xbuf );
	free( (char *)ybuf );
}

