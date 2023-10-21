#
/*
   device dependent core graphics driver for genisco
   A Christopher Hall fecit

ASSUMPTION: SCREEN COORDINATES LINE IN RANGE [0,1]

notes:  have 2 fonts, really: going across and going down

*/
#define CLEAR   0       /* complete */
#define INITIAL 1       /* complete */
#define SETVSBL 2       /* na */
#define SETHILIT 3      /* na */
#define SETDTCT 4       /* na */
#define DELETE  5       /* na */
#define ROTATE  6       /* na */
#define SCALE   7       /* na */
#define TRANSLATE 8     /* na */
#define OPENSEG 9       /* na */
#define TERMINATE 10    /* complete */
#define CLOSEG  11      /* na */
#define NDCSP2  12      /* complete */
#define GETCP   13      /* complete */
#define SETPID  14      /* na */
#define SETCOL  15      /* complete */
#define SETWIDTH 16     /* na */
#define SETSTYL 17
#define SETINT  18      /* na */
#define MOVE    19      /* complete */
#define LINE    20      /* complete */
#define TEXT    21      /* check on width of a character for computation of cp */

#define MARK    23      /* complete */
#define SETFONT 24      /* complete */
#define SETSIZE 25      /* don't agree on algorithm, maybe specs are incorrect */
#define SETANGLE 26     /* complete */



gensco1(ddstruct)
struct {
	int opcode;
	int logical;
	char *string;
	int int1;
	float float1;
	float float2;
	float float3;
} *ddstruct;
{
char onechr[3];
register int i,j;
register int i1,i2,i3;
int i4,i5,i6;
double f1,f2;
static int x,y;
static int zoom,direct,angle;   /* parameters for text */
static int color1[16]   0,15, 0, 0,15,15, 0,15,0,8,0,0,8,8,0,8;
static int color2[16]   0, 0,15, 0,15, 0,15,15,0,0,8,0,8,0,8,8;
static int color3[16]   0, 0, 0,15, 0,15,15,15,0,0,0,8,0,8,8,8;
static int topcol;


	switch(ddstruct->opcode) {

	case INITIAL:
		gopen();
		gucolor(1);
		zoom = direct = angle = 0;
		gscolor( 0, 0, 0, 0);
		gscolor(15, 0, 0, 1);
		gscolor( 0,15, 0, 2);
		gscolor( 0, 0,15, 3);
		gscolor(15,15, 0, 4);
		gscolor(15, 0,15, 5);
		gscolor( 0,15,15, 6);
		gscolor(15,15,15, 7);
		gscolor( 0, 0, 0, 8);
		gscolor( 8, 0, 0, 9);
		gscolor( 0, 8, 0,10);
		gscolor( 0, 0, 8,11);
		gscolor( 8, 8, 0,12);
		gscolor( 8, 0, 8,13);
		gscolor( 0, 8, 8,14);
		gscolor( 8, 8, 8,15);
	case TERMINATE:
	case CLEAR:
		gclear();
		x = y = 0;
		break;
	case SETCOL:
		f1 = .5 + 15. * ddstruct->float1;
		i1 = f1;
		f1 = .5 + 15. * ddstruct->float2;
		i2 = f1;
		f1 = .5 + 15. * ddstruct->float3;
		i3 = f1;
/*
	convert user's color to one which is in the VLT
*/
		if (i1<=4) i4 = 0;
			else if (i1<=10) i4 = 8;
				else i4 = 15;
		if (i2<=4) i5 = 0;
			else if (i2<=10) i5 = 8;
				else i5 = 15;
		if (i3<=4) i6 = 0;
			else if (i3<=10) i6 = 8;
				else i6 = 15;
/*
   now i4:6 can be mixed 0's and 15's or 0's and 8's, but not 8's and 15's.
   In the last case, convert the 8's to 15's if i1+i2+i3 >= 25;
   else convert the 15's to 8's
*/
		if (((i4==15) | (i5==15) | (i6==15)) &
			((i4==8) | (i5==8) | (i6==8))) {

			if ((i1 + i2 + i3) >= 25) {
				if (i4==8) i4 = 15;
				if (i5==8) i5 = 15;
				if (i6==8) i6 = 15;
			}
			else {
				if (i4==15) i4 = 8;
				if (i5==15) i5 = 8;
				if (i6==15) i6 = 8;
			}
		}
/*
   now find this color in the lookup table
   note, by the way, that black will be found in slot 8, not slot 0
   thus, slot 0 is not used, except to erase the screen
*/
		j = -1;
		for (i=1; i<=15; i++)
			if ((color1[i] == i4) && (color2[i] == i5)
				&& (color3[i] == i6)) j = i;

		if (j >= 0) {
			gucolor(j);
			break;
		}
/*
	If color not found, horrendous error; use white
*/
		gucolor(7);
		break;
	case SETSTYL:
		printf("SETSTYL not implemented yet\n");
		break;
/*
for text:
   normal-size characters are 5x7 in a 7x12 box
   sizes are limited to 1,2,3,...,16 * normal size
   angle is 0, 90, 180, 270
   font defines direct = 0,1
*/
	case SETFONT:
		direct = ddstruct->int1;
		goto setgt;
	case SETANGLE:
		angle = (45.5 + 360. * ddstruct->float1 / 6.28) / 90.;
		if (angle != 0) angle = 4 - angle;
		goto setgt;
	case SETSIZE:
		f1 = 511. * ddstruct->float1 / 7.;
		f2 = 511. * ddstruct->float2 / 5.;
		zoom = ((f1 < f2) ? f1 : f2) - .999999;
		if (zoom<0) zoom = 0;
setgt:          gtparm(zoom,direct,angle,0);
		break;
	case MARK:
		i1 = x - 3;
		i2 = 511 - (y + 5);
		gtparm(0,0,0,0);
		onechr[0] = ddstruct->string;
		onechr[1] = '\0';
		onechr[2] = '\0';
		gtext(i1,i2,onechr);
		goto setgt;
	case TEXT:
		i1 = x - (zoom + 1);
		i2 = (511 - y) - (7 + (zoom * 8));
		gtext(i1,i2,ddstruct->string);
		if(ddstruct->logical)
		   {
		   x =+ length(ddstruct->string) * 7 * (zoom + 1);
		   }
		break;
	case GETCP:
		ddstruct->float1 = x / 511.;
		ddstruct->float2 = y / 511.;
		break;
	case MOVE:
		i1 = x = 511. * ddstruct->float1 + .5;
		i2 = 511. - (y = 511. * ddstruct->float2 + .5);
		/* ADJUST Y COORDINATE TO ACCOUNT FOR FLIPPING OF SCREEN */
		gmove(i1,i2);
	case NDCSP2:
		break;
	case LINE:
		i1 = x = 511. * ddstruct->float1 + .5;
		i2 = 511 - (y = 511. * ddstruct->float2 + .5);
		/* ADJUST Y COORDINATE TO ACCOUNT FOR FLIPPING OF SCREEN */
		glinea(i1,i2);
		break;
	default:
		break;
	}
}
/*
	graphics subroutines. load with your favorite graphics program
	A Christopher Hall fecit

The following routines are defined:

INITIALIZERS:
	gopen()         hook up the genisco device
	gclear()        clear the screen

BASIC DRAWING ROUTINES:
	gmove(x,y)      move to (x,y)
	gpoint(x,y)     light the point at (x,y)
	glinea(x,y)     line from present position to (x,y)
	gliner(dx,dy)   line from present position relative (dx,dy) units
	gline(x1,y1,x2,y2)      line from (x1,y1) to (x2,y2)
	gbox(x1,y1,x2,y2)       outline of a box with given opposite corners
	gsolid(x1,y1,x2,y2)     solid box with given opposite corners
	gsolid1(x1,y1,x2,y2,s)  same, using given VLT slot
	gcircle(x,y,rad)        circle at (x,y) of radius rad

TEXT:
	gtext(x,y,string)       given text string, ULHC at (x,y)
	gtparm(zoom,direct,angle,backgr)        set text parameters
	askfor(string)          collect text string from terminal

VIDEO LOOKUP TABLE MANIPULATORS:
	gscolor(red,gre,blu,slot)  set VLT slot to given intensities
	gucolor(slot)           write memory planes given by slot

ROUTINES FOR USING MEMORY PLANES INDEPENDENTLY:
	[gucolor(15) should be called before these are used]
	gselect(planes)         write only the given planes *
	gsplanes(red,green,blue,plane)  set up given color in plane
	gshowp(plane)           display the given plane
	ghidep(plane)           suppress the given plane

* for the above, plane is in the range 1-4. But for gselect, planes
  is in 0-15, where each plane is represented by a single bit. So
  planes == 5 means write on planes 1 and 3; planes == 8 => write on 4.

HARDWARE EXPLOITERS:
	gscroll(x,y)    set screen origin to (x,y)
	gzoom(zoom)     zoom by a factor of 2 ** zoom
	gfill(planes)   set fill mode on given planes


THE FOLLOWING DEFINES THE COMMANDS KNOWN BY THE GENISCO OPERATING SYSTEM
*/
/*#define CLEAR  8        /* 0 args */
/*#define MOVE   1        /* 2 args */
/*#define POINT  2        /* 2 args */
/*#define LINEA  3        /* 2 args */
/*#define LINER  4        /* 2 args */
/*#define TEXT   14       /* 2 args */
/*#define UCOLOR 5        /* 1 args */
/*#define SCOLOR 9        /* 4 args */
/*#define SOLID  7        /* 4 args */
/*#define FILL   10       /* 1 arg  */
/*#define SCROLL 11       /* 2 args */
/*#define ZOOM   12       /* 1 arg  */
/*#define SELECT 6        /* 1 arg  */

#define CLEAR  0        /* 0 args */
#define MOVE   1        /* 2 args */
#define POINT  2        /* 2 args */
#define LINEA  3        /* 2 args */
#define LINER  4        /* 2 args */
#define TEXT   5        /* 2 args */
#define UCOLOR 6        /* 1 args */
#define SCOLOR 7        /* 4 args */
#define SOLID  8        /* 4 args */
#define FILL   9        /* 1 arg  */
#define SCROLL 10       /* 2 args */
#define ZOOM   11       /* 1 arg  */
#define SELECT 12       /* 1 arg  */

int pointc[3];          /* output vector */
int fd;                 /* genisco file device */
int ucolor 0;
int zoom   0;
int direct 0;
int angle  0;
int back   0;
int plared[5];          /* colors saved in VLT */
int plagre[5];
int plablu[5];
int onplanes;           /* flags for planes on: 1 & 3 on => == 5 (= 0101) */
int placol[15];         /* plane which this VLT slot belongs to */
/*
	subroutines to do things with the genisco

	like clear the screen
*/
gclear() {
	pointc[0] = CLEAR;      /* clear screen */
	write (fd,pointc,6);
}
/*
	select memory planes given by planes
*/
gselect(planes) {
	pointc[0] = SELECT;
	pointc[1] = planes;
	write (fd,pointc,6);
}
/*
	set up color in VLT slots - for when memory planes are
	used to hold independent images.

	If plane==4, loads slots 8-15 with given color;
	If plane==3, loads slots 4-7
	If plane==2, loads slots 2-3
	If plane==1, just loads slot 1.

	Thus, plane 8 dominates 4 dominates 2 dominates 1
	ie, if images overlap, the one that will appear will be on the
	higher-numbered plane
*/
gsplanes(red,green,blue,plane) {
register int i,j,k;

	if ((plane < 1) | (plane > 4)) {
		printf("plane number too high\n");
		return;
	}
	k = (1 << (plane-1));
	j = (k << 1) - 1;
	pointc[0] = SCOLOR;
	pointc[1] = (blue << 8) | (green << 4) | red;

	for (i=k; i<=j; i++) {
		pointc[2] = i;
		write (fd,pointc,6);
		placol[i] = k;
	}
	plared[plane] = red;
	plagre[plane] = green;
	plablu[plane] = blue;
	onplanes =| k;
}
/*
	show the given plane
*/
gshowp(plane) {
		onplanes =| 1 << (plane-1);
		gvltp();
}
/*
	suppress the given plane
*/
ghidep(plane) {
		onplanes =& ~(1 << (plane-1));
		gvltp();
}
/*
	function used by gshowp & ghidep to mess with VLT
	Not intended to be called by outsiders
*/
gvltp() {
register int i,c,d;

	for (i=1; i<=15; i++) {
		c = i & onplanes;       /* find highest turned-on plane */
		if (c >= 8) d = 4;      /* and load its color into this */
		    else if (c >= 4) d = 3;     /* slot */
			else if (c >= 2) d = 2;
			    else if (c == 1) d = 1;
				else d = 0;
		if (placol[i] != d) {
			gscolor(plared[d],plagre[d],plablu[d],i);
			placol[i] = d;
}       }       }
/*
	set up color in VLT slot
*/
gscolor(red,green,blue,slot) {
	pointc[0] = SCOLOR;
	pointc[1] = (blue << 8) | (green << 4) | red;
	pointc[2] = slot;
	write (fd,pointc,6);
}
/*
	set to draw using given VLT slot
*/
gucolor(slot) {
	ucolor = slot;
	pointc[0] = UCOLOR;
	pointc[1] = slot;
	write (fd,pointc,6);
}
/*
	scroll so that point (x,y) is at origin (ULHC)
*/
gscroll(x,y) {
	pointc[0] = SCROLL;
	pointc[1] = x;
	pointc[2] = y;
	write (fd,pointc,6);
}
/*
	zoom by a factor of 2**zoom
(MIGHT BE NICE TO ZOOM AROUND A GIVEN POINT)
*/
gzoom(zoom) {
	pointc[0] = ZOOM;
	pointc[1] = zoom << 10;
	write (fd,pointc,6);
}
/*
	set fill mode on planes given by set. set == 0 => clear fill mode
	set == -1 => fill on all planes
*/
gfill(set) {
	pointc[0] = FILL;
	pointc[1] = set;
	write (fd,pointc,6);
}
/*
	draw a line from (x1,y1) to (x2,y2)
*/
gline(x1,y1,x2,y2) {
	pointc[0] = MOVE;
	pointc[1] = x1;
	pointc[2] = y1;
	write (fd,pointc,6);
	pointc[0] = LINEA;
	pointc[1] = x2;
	pointc[2] = y2;
	write (fd,pointc,6);
}
/*
	draw a line from present position to (x,y)
*/
glinea(x,y) {
	pointc[0] = LINEA;
	pointc[1] = x;
	pointc[2] = y;
	write (fd,pointc,6);
}
/*
	draw a line from present position over dx by dy units
*/
gliner(dx,dy) {
	pointc[0] = LINER;
	pointc[1] = dx;
	pointc[2] = dy;
	write (fd,pointc,6);
}
/*
	move to (x,y)
*/
gmove(x,y) {
	pointc[0] = MOVE;
	pointc[1] = x;
	pointc[2] = y;
	write (fd,pointc,6);
}
/*
	light point at (x,y)
*/
gpoint(x,y) {
	pointc[0] = POINT;
	pointc[1] = x;
	pointc[2] = y;
	write (fd,pointc,6);
}
/*
   subroutine to draw a solid box. Uses current color, like it should.
   However, for drawing gray pictures which consist of a lot of little
   boxes of different shades, it is faster to include the slot to be
   used with the solid call. In that case, call solid1. The current
   color remains unchanged after solid1; it's just a one-shot deal.
*/
gsolid(x1,y1,x2,y2) {
	gsolid1(x1,y1,x2,y2,-1);        /* -1 means use current color */
}
/*
   subroutine to draw a solid box, using given VLT slot
*/
gsolid1(x1,y1,x2,y2,slot) {
register int i;

	if (x1 > x2) {i=x1; x1=x2; x2=i;}
	if (y1 > y2) {i=y1; y1=y2; y2=i;}
	pointc[0] = SOLID;
	pointc[1] = x1;
	pointc[2] = y1;
	write (fd,pointc,6);
	pointc[0] = x2;
	pointc[1] = y2;
	pointc[2] = slot;
	write (fd,pointc,6);
}
/*
	draw a box, given opposite corner coords
*/
gbox(x1,y1,x2,y2) {
	gline(x1,y1,x1,y2);
	glinea(x2,y2);
	glinea(x2,y1);
	glinea(x1,y1);
}
/*
	draw a circle, given coords of center and radius
*/
gcircle(x,y,rad) {
	register int i,j,k;
	double angle, sin(), cos(), inc;

	inc = 8. / rad;

	gmove(x+rad,y);
	for (angle=0.0; angle<=6.3; angle =+ inc) {
		i = x + rad*cos(angle);
		j = y + rad*sin(angle);
		glinea(i,j);
	}
	glinea(x+rad,y);
}
/*
	set text parameters
*/
gtparm(b,c,d,e) {
	zoom = 15 & b;
	direct = 1 & c;
	angle = 3 & d;
	back = e;
}
/*
	display text
*/
gtext(x,y,s)
    char *s; {
register int i;

	pointc[0] = TEXT;
	i = (((angle << 1) | direct ) << 5) | zoom;
	if (back) i = i | 020;
	pointc[1] = i;
	pointc[2] = back;       /* background color */
	write (fd,pointc,6);

	pointc[0] = ucolor;     /* foreground color */
	pointc[1] = x;          /* x-position */
	pointc[2] = y;          /* y-position */
	write (fd,pointc,6);

	i = -1;                    /* send characters until null found */
	while (i) {
		if ((pointc[0] = *s++) == 0)
			pointc[1] = pointc[2] = i = 0;
		else if ((pointc[1] = *s++) == 0)
			pointc[2] = i = 0;
		else if((pointc[2] = *s++) == 0) i = 0;
		write (fd,pointc,6);
	}
}
/*
   open genisco device
*/
gopen() {
	if ((fd = open("/dev/gn",1)) < 0)
	{       printf ("Can't open the genisco\n");
		exit(1);
}       }
