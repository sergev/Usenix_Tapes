From ddj@csnet-relay.csnet Mon Jul 30 03:15:30 1984
Received: from csnet-relay by safe with TCP; Mon, 30 Jul 84 03:15:18 pdt
Received: From brown.csnet by csnet-relay;  30 Jul 84 5:46 EDT
Message-Id: <8407300942.AA03636@nancy.Brown.CSNet>
Date:     30 Jul 84 (Mon) 05:42:49 EDT
From: Dave Johnson <ddj%brown.csnet@csnet-relay.arpa>
To: croft%su-safe@csnet-relay.arpa, pattermann%sumex-aim@csnet-relay.arpa,
        winkler%harvard@csnet-relay.arpa
Subject:  paintimp.c
Status: RO

This is a program I hacked up last night to print macpaint images
on the imagen.  It will print nearly full-scale images on both 240
and 480 printers, and should do fine on a 300 as well, but we don't
have one to test it with.  It also supports 2-up format with about 
66% reduction, which could be useful.  

Since some versions of impress don't support magnification, and in
any case the ones that do only magnify in powers of two, if you want
to send bitmaps and get roughly 8 X 10 images, you must do the factor
of 3 (or 6) magnification yourself.  This would generate a quarter
meg file for the 10/240, and a whole meg spool file for the 5/480.  

This program takes a different approach, and achieves roughly fixed
spool file size by defining 255 glyphs of the proper size for the
printer and format desired, and sending the file as 8 bit characters.
One hangup is that impress only allows 128 members per family, so
you have to switch between families, but I got a lot out of using
the @SP@ instruction, which works as a blank glyph in both families.

The idea for using a glyph for each possible 8-bit pattern was worked
out and used by some of the Computer Center staff here to print on
the Xerox 9700 printer, and I have seen one page printed over there
so far . . .  I could try to get them to submit their program if anyone
is interested in a VM/370 implementation, but don't know what their
policies are at the CC.

Try it out, and if you think I should post this, let me know.
It's about half an hour old now, but it works on 2 pictures,
with/without 2-up mode, and on both the 240 and 480, so it should
be pretty close.  (Dan: I even linted it this time)

Known problem -- I haven't looked up the proper @document(...) stuff.
I've been using it with the berkeley spooler as in:

	paintimp foo.data | lpr -Pip -v

where the -v option specifies "raster" mode, and generates a decent 
header -- mainly defines "language impress".  
	
	Dave Johnson
----------
paintimp.c
----------
#ifndef lint
static	char sccsid[] = "@(#)paintimp.c 0.9 84/07/29 (Brown)";
#endif

/*
 * paintimp -- read macpaint document and output impress document 
 *
 * Dave Johnson, Brown University Computer Science
 *
 * (c) 1984 Brown University 
 * may be used but not sold without permission
 *
 * created ddj 5/06/84 for sun's rasterfile.h format (macfilter.c)
 * revised ddj 7/29/84 -- generate Impress format glyphs and data
 */

#include <stdio.h>

#define MACPAINT_HDRSIZE	512

#define	MACPAINT_BITWIDTH	576
#define	MACPAINT_BITHEIGHT	720

struct macpaint_hdr {
	char macp_data[MACPAINT_HDRSIZE];
};

typedef struct _point {
	int x;
	int y;
} point;

#define RES_240 0
#define RES_300 1
#define RES_480 2

#define DEF_RES 240

int twoup = 0;
int res = DEF_RES;
int res_ind;
int scale;
int imageno = 0;

char usage[] = "usage: \"paintimp [-2] [-r res] files\"\n";

main(ac, av)
char **av;
{
	char *name;

	ac--; av++;
	while (ac && av[0][0] == '-' && av[0][1]) {
		switch (av[0][1]) {
		case '2':
			twoup++;
			break;

		case 'r':
			if (ac < 2)
				goto bad_usage;
			ac--; av++;
			res = atoi(av[0]);
			break;
			
		default:
			goto bad_usage;
		}
		ac--; av++;
	}
	switch (res) {
	case 240:
		res_ind = RES_240;
		break;
	case 300:
		res_ind = RES_300;
		break;
	case 480:
		res_ind = RES_480;
		break;
	default:
		fprintf(stderr, "illegal resolution %d;", res);
		fprintf(stderr, " use -r {240,300,480}\n");
		goto bad_usage;
	}
	if (ac == 0) 
		goto bad_usage;
	init_images(MACPAINT_BITWIDTH, MACPAINT_BITHEIGHT);
	write_imphdr();
	write_glyphs();
	while (ac) {
		name = av[0];
		filter(name);
		ac--; av++;
	}
	exit(0);
bad_usage:
	fprintf(stderr, usage);
	exit(1);
}

filter(name)
char *name;
{
	register int x, y;
	FILE *fp;
	int c;

	if (name[0] == '-')
		fp = stdin;
	else
		fp = fopen(name, "r");
	if (fp == NULL) {
		perror(name);
		return;
	}

	x = 0;
	y = 0;
	imageno++;
	read_painthdr(fp);
	begin_image();
	while ((c = getbits(fp)) != EOF) {
		putbits(c);
		x += 8;
		if (x >= MACPAINT_BITWIDTH) {
			putcrlf();
			x = 0;
			y++;
			if (y >= MACPAINT_BITHEIGHT) {
				end_image();
				fclose(fp);
				return;
			}
		}
	}
}


/* macpaint input routines */

read_painthdr(fp)
FILE *fp;
{
	fseek(fp, (long)MACPAINT_HDRSIZE, 0);
}

getbits(fp)
FILE *fp;
{
	static int count, rep, chr;
	int c;

	if (rep) {
		rep--;
		return chr;
	}
	if (count) {
		count--;
		return getc(fp);
	}
	c = getc(fp);
	if (c & 0x80) {			/* repeated character count */
		rep = 0x100 - c;	/* byte length 2's comp + 1 */
					/* 	allow for this call */
		chr = getc(fp);		/* character to repeat */
		return chr;
	}
	else {
		count = c;		/* already counted this char */
		return getc(fp);
	}
}

/* impress output routines */

#define FAMILY_MASK 0x80
#define MEMBER_MASK 0x7f
#define BYTE_MASK 0xff

putbits(c)
int c;
{
	static int fam = 0;

	if (c == 0) {
		putspace();
		return;
	}
	if ((c & FAMILY_MASK) && fam == 0) {
		fam = 1;
		putfam(fam);
	}
	else if (!(c & FAMILY_MASK) && fam == 1) {
		fam = 0;
		putfam(fam);
	}
	putchar(c & MEMBER_MASK);
}

#define IMP_SP		128
#define IMP_CRLF	197
#define IMP_SET_FAMILY	207

putcrlf()
{
	putchar(IMP_CRLF);
}

putspace()
{
	putchar(IMP_SP);
}

putfam(f)
int f;
{
	putchar(IMP_SET_FAMILY);
	putchar(f & BYTE_MASK);
}


/* really grungy stuff */

struct imp_scale {
	int normal_scale;
	int rotate_scale;
} is[] = {	/* tuned only for macpaint sized bitmaps */
	{ 3, 2 },	/* RES_240 */
	{ 4, 2 },	/* RES_300 */
	{ 6, 4 },	/* RES_480 */
};

#define ORG_CURRENT 3

#define AXES_NOP 0

#define ORIENT_NORMAL 4
#define ORIENT_ROTATE 5

static point origin;
static point image_space;

static int hv_system;
	
/* determine scale and origin points for output images */

init_images(width, height)
int width, height;
{
	if (twoup) {
	    /*
		+------------+
		| ^org       |
	       ^|            |
	       x|            |
		+------------+
		 y->
	     */
		scale = is[res_ind].rotate_scale;
		image_space.x = (17 * res) / 2;	/* 8.5 inches */
		image_space.y = (11 * res) / 2;	/* 5.5 inches */

		/* set new origin (in old coordinates) */
		origin.x = (image_space.x + height * scale) / 2;
		origin.y = (image_space.y - width * scale) / 2;

		hv_system = (ORG_CURRENT<<5) | (AXES_NOP<<3) | ORIENT_ROTATE;
	}
	else {
		scale = is[res_ind].normal_scale;
		image_space.x = (17 * res) / 2;	/* 8.5 inches */
		image_space.y = 11 * res;	/* 11 inches */

		origin.x = (image_space.x - width * scale) / 2;
		origin.y = (image_space.y - height * scale) / 2;

		hv_system = (ORG_CURRENT<<5) | (AXES_NOP<<3) | ORIENT_NORMAL;
	}
}

#define IMP_SET_ABS_H	135
#define IMP_SET_ABS_V	137
#define IMP_SET_HV_SYSTEM 205
#define IMP_ENDPAGE	219
#define IMP_SET_IL	208
#define IMP_SET_BOL	209
#define IMP_SET_SP	210
#define IMP_BGLY	199

begin_image()
{
	point o;

	o.y = 0;
	if ((imageno & 1) || !twoup)
		o.x = 0;
	else
		o.x = image_space.y + origin.y;
	putchar(IMP_SET_BOL);
	putshort(o.x);
	set_abs_pos(o);
}

end_image()
{
	if (twoup && (imageno & 1))
		return;
	putchar(IMP_ENDPAGE);
}

write_imphdr()
{
	/* SET_SP (8*scale), SET_IL (scale), SET_BOL (0); all shorts */
	putchar(IMP_SET_IL);
	putshort(scale);
	putchar(IMP_SET_SP);
	putshort(scale*8);
	set_abs_pos(origin);
	putchar(IMP_SET_HV_SYSTEM);
	putchar(hv_system);
}

write_glyphs()
{
	register int glyph;

	for (glyph = 1; glyph <= 255; glyph++) {
		write_glyph(glyph);
	}
}

char bits[8];		/* code assumes scale <= 8 */

write_glyph(g)
register int g;
{
	register int i, j;
	register int b, xpos;
	int bit;

	if (twoup) {
		putchar(IMP_BGLY);
		g |= (1 << 14);		/* rotate 90 degrees clockwise */
		putshort(g);
		putshort(scale*8);
		putshort(scale);
		putshort(0);
		putshort(scale*8);
		putshort(0);
		for (b = 0x80; b; b >>= 1) {
			bits[0] = 0;
			bit = g & b;
			for (i = 0; i < scale; i++)
				if (bit) setbit(i);
			for (i = 0; i < scale; i++)
				putchar(bits[0] & BYTE_MASK);
		}
	}
	else {
		putchar(IMP_BGLY);
		putshort(g);
		putshort(scale*8);
		putshort(scale*8);
		putshort(0);
		putshort(scale);
		putshort(0);
		xpos = 0;
		for (i = 0; i < scale; i++)
			bits[i] = 0;
		for (b = 0x80; b; b >>= 1) {
			bit = g & b;
			for (i = 0; i < scale; i++, xpos++)
				if (bit) setbit(xpos);
		}
		for (j = 0; j < scale; j++) {
			for (i = 0; i < scale; i++)
				putchar(bits[i] & BYTE_MASK);
		}
	}
}

setbit(x)
int x;
{
	int bitno;
	char *bitp;
	
	bitno = 7 - (x % 8);
	bitp = &bits[x / 8];
	*bitp |= (1 << bitno);
}

set_abs_pos(pt)
point pt;
{
	putchar(IMP_SET_ABS_H);
	putshort(pt.x);
	putchar(IMP_SET_ABS_V);
	putshort(pt.y);
}

putshort(s)
int s;
{
	putchar((s >> 8) & BYTE_MASK);
	putchar(s & BYTE_MASK);
}

