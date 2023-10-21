/*
 * Reads standard graphics input
 * Makes a plot on a 200 dot-per-inch 11" wide
 * Versatek plotter.
 *
 * Creates and leaves /tmp/raster (1000 blocks)
 * which is the bitmap
 * 
 * modified by D. Harvey on April 5, 1980 to read
 * from routine pread.c.  also modified to plot
 * lower left hand cornaer of plot in lower left
 * hand corner of versatec page instead of centering.
 */
#include <math.h>
#include "stdio.h"
#include <signal.h>

#define	NB	88
#define BSIZ	512
#define	mapx(x)	((1536*((x)-botx)/del)+centx)
#define	mapy(y)	((1536*(del-(y)+boty)/del)-centy)
#define SOLID -1
#define DOTTED 014
#define SHORTDASHED 034
#define DOTDASHED 054
#define LONGDASHED 074
#define	SETSTATE	(('v'<<8)+1)

int	linmod	= SOLID;
int	again;
int	done1;
char	chrtab[][16];
int	plotcom[]	{ 0200, 0, 0};
int	eotcom[]		{ 0210, 0, 0};
char	blocks	[NB][BSIZ];
int	obuf[264];
int	lastx;
int	lasty;
double	topx	= 1536;
double	topy	= 1536;
double	botx	= 0;
double	boty	= 0;
int	centx;
int	centy;
double	delx	= 1536;
double	dely	= 1536;
double	del	= 1536;
char s[256];
int pat[256];
float xfp[10];
double slant, angle;
double rad = .0174532925;
double height, ratio;
double tanslant = 0.;
double sinangle = 0.;
double cosangle = 1.;
double wwdd2;
double xdim = 1;
double ydim = 1;
double ixdim = 14;
double iydim = 20;
int ixref = 8;
int iyref = 16;
double xspace = 16;

struct	buf {
	int	bno;
	char	*block;
};
struct	buf	bufs[NB];

int	in, out;
char *picture = "/tmp/raster";

main(argc, argv)
char **argv;
{
	extern int onintr();
	register i;

	if (argc>1) {
		in = open(argv[1], 0);
		putpict();
		exit(0);
	}
	signal(SIGTERM, onintr);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);
another:
	for (i=0; i<NB; i++) {
		bufs[i].bno = -1;
		bufs[i].block = blocks[i];
	}
	out = creat(picture, 0666);
	in = open(picture, 0);
/* CIRES 3/80 mls
 * /tmp/raster is unlinked immediately after creation
 * to avoid leaving garbage raster files around
 */
unlink(picture);
	zseek(out, 32*32);
	write(out, blocks[0], BSIZ);
/*delete following code when filsys deals properly with
holes in files*/
	for(i=0;i<512;i++)
		blocks[0][i] = 0;
	zseek(out, 0);
	for(i=0;i<32*32;i++)
		write(out,blocks[0],512);
/**/
	getpict();
	for (i=0; i<NB; i++)
		if (bufs[i].bno != -1) {
			zseek(out, bufs[i].bno);
			write(out, bufs[i].block, BSIZ);
		}
	putpict();
	if (again) {
		close(in);
		close(out);
		goto another;
	}
	exit(0);
}

getpict()
{
	register x1, y1;
	int pread(); long l; long sl;
	int c,i;
	double sin(),cos();

	again = 0;
	l = 256;
	while((c=pread(pat, xfp, s, &sl, l, 0)) != -1){
		switch(c){
		/* case 'm' */
		case 1:
			lastx = mapx(pat[0]);
			lasty = mapy(pat[1]);
			break;
		/* case 'l' */
		case 4:
			done1 |= 01;
			x1 = mapx(pat[0]);
			y1 = mapy(pat[1]);
			lastx = mapx(pat[2]);
			lasty = mapy(pat[3]);
			line(x1, y1, lastx, lasty);
			break;
		/* case 't' */
		case 5:
			done1 |= 01;
			for (y1 = 0; y1 < (long) sl; y1++)
				plotch(x1=s[y1]);
			break;
		/* case 'e' */
		case 8:
			if (done1) {
				again++;
				return;
			}
			break;
		/* case 'p' */
		case 3:
			done1 |= 01;
			lastx = mapx(pat[0]);
			lasty = mapy(pat[1]);
			point(lastx, lasty);
			point(lastx+1, lasty);
			point(lastx, lasty+1);
			point(lastx+1, lasty+1);
			break;
		/* case 'n' */
		case 2:
			done1 |= 01;
			x1 = mapx(pat[0]);
			y1 = mapy(pat[1]);
			line(lastx, lasty, x1, y1);
			lastx = x1;
			lasty = y1;
			break;
		/* case 's' */
		case 10:
			botx = pat[0];
			boty = pat[1];
			topx = pat[2];
			topy = pat[3];
			delx = topx-botx;
			dely = topy-boty;
			if (dely/delx > 1536./2048.)
				del = dely;
			else
				del = delx * (1566./2048.);
			centx = 0;
			centx = (2048 - mapx(topx)) / 2;
			centy = 0;
			centy = mapy(topy) / 2;
			centx = 0; centy = 0;
			break;
		/* case 'a' */
		case 6:
			break;
		/* case 'c' */
		case 7:
			break;
		/* case 'f' */
		case 9:
			switch(pat[0])	{
				case 1:
					linmod = DOTTED;
					break;
				case 2:
					linmod = SOLID;
					break;
				case 3:
					linmod = LONGDASHED;
					break;
				case 4:
					linmod = SHORTDASHED;
					break;
				case 5:
					linmod = DOTDASHED;
					break;
				}
			break;
		/* case 'd' */
		case 18:
			break;
		/* case 'j' */
		case 16:
			height = xfp[0];
			ratio = xfp[1];
			wwdd2 = ratio*height;
			ydim = (1536*height/del)/iydim;
			xdim = (1536*wwdd2/del)/ixdim;
			xspace = xdim*1.5*ixdim;
			slant = xfp[2];
			slant *= rad;
			tanslant = sin(slant)/cos(slant);
			ixref = 2;
			iyref = 20;
			break;
		/* case 'k' */
		case 17:
			angle = xfp[0];
			angle *= rad;
			sinangle = sin(angle);
			cosangle = cos(angle);
			break;
		}
	}
}

plotch(c)
register c;
{
	register j;
	register char *cp;
	int i,xref;

	if (c<' ' || c >0177)
		return;
	cp = chrtab[c-' '];
	for (i = -iyref; i < (32-iyref); i += 2) {
		c = *cp++;
		for (j=7; j>=0; --j)
			if ((c>>j)&1) {
				xref = (7-j)*2 - ixref;
				cpoint(xref,i);
			}
	}
	lastx += xspace*cosangle;
	lasty -= xspace*sinangle;
}

cpoint(ix,iy)

int ix,iy;
{

	double x,y,xx,yy;

	yy = iy*ydim;
	xx = ix*xdim - yy*tanslant;
	tran(xx,yy);
	xx += xdim;
	tran(xx,yy);
	yy -= ydim;
	tran(xx,yy);
	xx -= xdim;
	tran(xx,yy);
}

tran(xx,yy)

double xx,yy;

{

	double x,y;
	int ix,iy;

	x =  xx*cosangle + yy*sinangle;
	y = -xx*sinangle + yy*cosangle;
	ix = lastx + x;
	iy = lasty + y;
	point(ix,iy);
}

int	f; /* versatec file number */
putpict()
{
	register x, *ip, *op;
	int y;

	if (f==0){
		f = open("/dev/vp0", 1);
		if (f < 0) {
			printf("Cannot open vp\n");
			exit(1);
		}
		if( ioctl(f, SETSTATE, plotcom) != 0 ) {
			printf("ioctl failure vp\n");
			exit( 1 );
		}
	}
	op = obuf;
	lseek(in, 0L, 0);
	for (y=0; y<2048; y++) {
		if ((y&077) == 0)
			read(in, blocks[0], 32*BSIZ);
		for (x=0; x<32; x++)  {
			ip = (int *)&blocks[x][(y&077)<<3];
			*op++ = *ip++;
			*op++ = *ip++;
			*op++ = *ip++;
			*op++ = *ip++;
		}
		*op++ = 0;
		*op++ = 0;
		*op++ = 0;
		*op++ = 0;
		if (y&1) {
			write(f, (char *)obuf, sizeof(obuf));
			op = obuf;
		}
	}
}

line(x0, y0, x1, y1)
register x0, y0;
{
	int dx, dy;
	int xinc, yinc;
	register res1;
	int res2;
	int slope;

	xinc = 1;
	yinc = 1;
	if ((dx = x1-x0) < 0) {
		xinc = -1;
		dx = -dx;
	}
	if ((dy = y1-y0) < 0) {
		yinc = -1;
		dy = -dy;
	}
	slope = xinc*yinc;
	res1 = 0;
	res2 = 0;
	if (dx >= dy) while (x0 != x1) {
	if((x0+slope*y0)&linmod)
	if (((x0>>6) + ((y0&~077)>>1)) == bufs[0].bno)
		bufs[0].block[((y0&077)<<3)+((x0>>3)&07)] |= 1 << (7-(x0&07));
	else
		point(x0, y0);
		if (res1 > res2) {
			res2 += dx - res1;
			res1 = 0;
			y0 += yinc;
		}
		res1 += dy;
		x0 += xinc;
	} else while (y0 != y1) {
	if((x0+slope*y0)&linmod)
	if (((x0>>6) + ((y0&~077)>>1)) == bufs[0].bno)
		bufs[0].block[((y0&077)<<3)+((x0>>3)&07)] |= 1 << (7-(x0&07));
	else
		point(x0, y0);
		if (res1 > res2) {
			res2 += dy - res1;
			res1 = 0;
			x0 += xinc;
		}
		res1 += dx;
		y0 += yinc;
	}
	if((x1+slope*y1)&linmod)
	if (((x1>>6) + ((y1&~077)>>1)) == bufs[0].bno)
		bufs[0].block[((y1&077)<<3)+((x1>>3)&07)] |= 1 << (7-(x1&07));
	else
		point(x1, y1);
}

point(x, y)
register x, y;
{
	register bno;

	bno = ((x&03700)>>6) + ((y&03700)>>1);
	if (bno != bufs[0].bno) {
		if (bno < 0 || bno >= 1024)
			return;
		getblk(bno);
	}
	bufs[0].block[((y&077)<<3)+((x>>3)&07)] |= 1 << (7-(x&07));
}

getblk(b)
register b;
{
	register struct buf *bp1, *bp2;
	register char *tp;

loop:
	for (bp1 = bufs; bp1 < &bufs[NB]; bp1++) {
		if (bp1->bno == b || bp1->bno == -1) {
			tp = bp1->block;
			for (bp2 = bp1; bp2>bufs; --bp2) {
				bp2->bno = (bp2-1)->bno;
				bp2->block = (bp2-1)->block;
			}
			bufs[0].bno = b;
			bufs[0].block = tp;
			return;
		}
	}
	zseek(out, bufs[NB-1].bno);
	write(out, bufs[NB-1].block, BSIZ);
	zseek(in, b);
	read(in, bufs[NB-1].block, BSIZ);
	bufs[NB-1].bno = b;
	goto loop;
}

onintr()
{
	exit(1);
}

zseek(a, b)
{
	return(lseek(a, (long)b*512, 0));
}
