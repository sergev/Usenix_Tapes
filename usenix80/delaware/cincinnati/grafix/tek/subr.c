#
int tekbuf[259];
float obotx 0.;
float oboty 0.;
float botx 0.;
float boty 0.;
float scalex 1.;
float scaley 1.;

int oloy -1;
int ohiy -1;
int ohix -1;
int plmode	0;

int ox	-1;
int oy	-1;

#define	TOP	1
#define	BOTTOM	2
#define	LEFT	4
#define	RIGHT	8
extern float deltx, delty;

#define	max(a,b)	(a>b?a:b)
#define	ALMOST0	1e-38
#define	XDENOM	(max(fx-ox,ALMOST0))	/* avoid floating */
#define	YDENOM	(max(fy-oy, ALMOST0))	/* divide checks */


char *plotter "Tektronix 4006-1";

pt(x,y){
	int hix,hiy,lox,loy;
	int c, n;
	float fx, fy;

	/* modified Cohen-Sutherland clipping */

	c = code(x, y);

	while(c){
		fx = x;		/* get floating arithmetic */
		fy = y;

		if(c & LEFT){	/* crosses left edge */
			fy = oy + (fy - oy) * -ox / XDENOM;
			fx = 0.;
		}else
		if(c & RIGHT){	/* you guessed it! */
			fy = oy + (fy - oy) * (deltx - ox) / XDENOM;
			fx = deltx;
		}else
		if(c & BOTTOM){
			fx = ox + (fx - ox) * -oy / YDENOM;
			fy = 0.;
		}else
		if(c & TOP){
			fx = ox + (fx - ox) * (delty - oy) / YDENOM;
			fy = delty;
		}

		x = fx + .5; 	/* round up */
		y = fy + .5;
		code(x, y);
	}

	/* the endpoints now in x and y are guaranteed on the screen,
	 * unless you've done something funny with deltx and delty.
	 * actually, the whole point of this is to avoid the hardware
	 * wraparound...nothing ever disappears from the screen,
	 * but if you adress a point beyond the screen limits,
	 * it gets put at the requested address mod 1024.
	 */

	ox = x;		/* save for label() */
	oy = y;

	hix=(x>>5) & 037;
	hiy=(y>>5) & 037;
	lox = x & 037;
	loy = y & 037;
	n = (abs(hix-ohix) + abs(hiy-ohiy) + 6) / 12;
	if(hiy != ohiy){
		putch(hiy|040);
		ohiy = hiy;
	}
	if(hix != ohix){
		putch(loy|0140);
		putch(hix|040);
		ohix=hix;
		oloy=loy;
	}
	else{
		if(loy != oloy){
			putch(loy|0140);
			oloy=loy;
		}
	}
	putch(lox|0100);
	while(n--)
		putch(0);	/* send nulls for delay */
}

code(x, y)
	register x, y;
{
	register c;

	c = 0;

	if(x < 0) c =+ LEFT;
		else if(x > deltx) c =+ RIGHT;

	if(y < 0) c =+ BOTTOM;
		else if(y > delty) c =+ TOP;
	return(c);
}

putch(c){
	putc(c,tekbuf);
}
extern float botx;
extern float boty;
extern float obotx;
extern float oboty;
extern float scalex;
extern float scaley;
xsc(xi)
register xi;
{
	register xa;
	xa = (xi - obotx)*scalex +botx;
	return(xa);
}
ysc(yi)
register yi;
{
	register ya;
	ya = (yi - oboty)*scaley +boty;
	return(ya);
}
extern int plmode;
penup(){
	putch(035);
	plmode = 1;
}
extern int ox,oy;

pendown(){
	putch(035);
	plmode = 1;
	pt(ox,oy);
}

printmode(){
	putch(037);
}
