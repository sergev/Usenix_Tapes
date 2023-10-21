
/*
 * epson_ps: convert Epson MX/RX-format files to PostScript
 *	missing compressed & expanded modes, auto margins and graphics.
 *	tested with Super Writer and Word Perfect.
 *	the -w flag rotates and compresses the page to print 136 columns.
 *
 * Copyright (c) 1986, Eric Gisin, <egisin@waterloo.CSNET>
 *	This program may be redistributed in source form,
 *	provided no fee is charged and this copyright notice is preserved.
 */

#include <stdio.h>
#include <ctype.h>

#define	HRES	60
#define	VRES	(3*72)

#define	setfont(F)	font |= (F)
#define	clrfont(F)	font &= ~(F)
#define	tstfont(F)	(font&(F))
#define	BOLD	0x01
#define	ITALIC	0x02
#define	PITCH12	0x04
#define	UNDLINE	0x08
#define	SCRIPT	0x10
#define	SUPER	0x20

char *	Head [] = {
	"%!PS-Adobe-1.0",
	"%%Creator: epson_ps",
	"%%For: ?",
	"%%DocumentFonts: Courier Courier-Bold Courier-Oblique Courier-BoldOblique",
	"%%Pages: (atend)",
	"",
	"/Hscale 72 60 div def",
	"/Vscale 72 -216 div def",
	"/Vlength 72 11 mul def",
	"/Xlate {Hscale mul exch Vscale mul Vlength add} def",
	"/M {Xlate moveto} def",
	"/F {",
	"	/f exch def",
	"	[/Courier /Courier-Bold /Courier-Oblique /Courier-BoldOblique]",
	"	f 16#3 and get findfont",
	"	f 16#4 and 0 eq {12} {10} ifelse",
	"	f 16#10 and 0 ne {2 sub} if",
	"	scalefont setfont",
		"	f 16#10 and 0 ne {0 f 16#20 and 0 ne {4} {-2} ifelse rmoveto} if",
	"} def",
	"/S {show} def",
	"/P+ {/Save save def} def",
	"/P- {gsave showpage grestore Save restore} def",
	"/G {pop pop} def",	/* Graphics, not implemented */
	"18 0 translate",
	NULL};

int	page = 0;
char	hexchar [] = "0123456789ABCDEF";

main(argc, argv)
	char **	argv;
{
	int	i;
	int	nofiles = 1;

	for (i = 0; Head[i]!=NULL; i ++)
		printf("%s\n", Head[i]);
	for (i = 1; i < argc; i ++)
	    if (argv[i][0]=='-')
		switch (argv[i][1]) {
		  case 'w':
			printf("[0 -.727 0.727 0 3 737] concat]\n");
			/* should set hend = 14i in epson_ps */
			break;
		  default:
			fprintf(stderr, "Usage: %s [-w] file ...\n", argv[0]);
			exit(1);
		}
	    else {
		FILE *	f;
		nofiles = 0;
		f = fopen(argv[i], "r");
		if (f==NULL) {
			fprintf(stderr, "%s: Can't open input %s\n", argv[0], argv[i]);
			continue;
		}
		epson_ps(f);
		fclose(f);
	    }
	if (nofiles)
		epson_ps(stdin);
	printf("%%%%Trailer\n");
	printf("%%%%Pages: %d\n", page);
}

epson_ps(f)
	register FILE *	f;
{
	register c;
	register i;
	int	started = 0;	/* any output on page? */
	int	font = 0, lastfont;
	int	hpos = 0, hlast, hspace = HRES/10, htab = HRES*8/10;
	int	hbeg = 0, hend = HRES*8;
	int	vpos = 0, vlast, vspace = VRES/6, vtab = 0;
	int	vlength = VRES*11, vmarg = 0;
	int	charset = 0;

	do switch((c = getc(f))) {
	  case EOF:
		vpos = vlength;
		goto CheckPage;
	  case 0x00:	/* tab terminator ? */
		break;
	  case 0x08:	/* backspace */
		hpos -= hspace;
		break;
	  case 0x09:	/* Hor tab */
		if (!htab)
			break;
		for (i = hbeg; i<=hpos; i += htab)
			;
		hpos = i;
		break;
	  case 0x0A:	/* line feed */
		hpos = hbeg;
		vpos += vspace;
		goto CheckPage;
	  case 0x0B:	/* vert tab */
		if (!vtab)
			break;
		for (i = 0; i<=vpos; i += vtab)
			;
		vpos = i;
		break;
	  case 0x0C:	/* form feed */
		vpos = vlength;
	  CheckPage:
		if (vpos<vlength || !started)
			break;
		/* output page end */
		vpos %= vlength;
		fprintf(stdout, "P-\n");
		started = 0;
		break;
	  case 0x0D:	/* return */
		hpos = hbeg;
		break;
	  case 0x0E:	/* expand on (one line) */
		/* ?? */
		break;
	  case 0x0F:	/* compress on not implemented. use -w */
		break;
	  case 0x12:	/* compress off not implemented. use -w */
		break;
	  case 0x14:	/* expand off (one line) */
		/* ?? */
		break;
	  case 0x1B:	/* escape sequence */
		c = getc(f);
		switch (c) {
		  case '*':	/* graphics mode */
			c = getc(f);
		  Graphics:
			vlast = c;	/* save mode in wrong variable */
			i = getc(f);
			i += getc(f) << 8;
			fprintf(stdout, "%d %d M ", vpos, hpos);
			hlast = -1;
			putc('<', stdout);
			while (--i >= 0) {
				c = getc(f);
				putc(hexchar[c>>4], stdout);
				putc(hexchar[c&&0xF], stdout);
			}
			putc('>', stdout);
			fprintf(stdout, " %d G\n", vlast);
			break;
		  case '-':	/* underline */
			if (!getc(f))
				clrfont(UNDLINE);
			else	setfont(UNDLINE);
			break;
		  case '0':	/* vert spacing */
			vspace = VRES/8;
			break;
		  case '1':
			vspace = VRES*7/72;
			break;
		  case '2':	/* vert spacing */
			vspace = VRES/6;
			break;
		  case '3':
			vspace = VRES*getc(f)/216;
			break;
		  case '4':
			setfont(ITALIC);
			break;
		  case '5':
			clrfont(ITALIC);
			break;
		  case '@':	/* reset all */
		  Reset:
			started = 0;
			font = 0;
			hpos = 0; hspace = HRES/10; htab = HRES*8/10;
			hbeg = 0; hend = HRES*8;
			vpos = 0; vspace = VRES/6; vtab = 0;
			vlength = VRES*11; vmarg = 0;
			charset = 0;
			break;
		  case 'A':	/* vert space */
			vspace = VRES*getc(f)/72;
			break;
		  case 'C':	/* form length */
			c = getc(f);
			if (c)
				vlength = vspace*c;
			else
				vlength = VRES*getc(f);
			break;
		  case 'E':	/* emphasize on */
			setfont(BOLD);
			break;
		  case 'F':	/* emphasize off */
			clrfont(BOLD);
			break;
		  case 'G':	/* double strike on */
			setfont(BOLD);
			break;
		  case 'H':	/* double strike off */
			clrfont(BOLD);
			break;
		  case 'J':	/* n feed */
			vpos += VRES*getc(f)/216;
			goto CheckPage;
		  case 'K':	c = 0;	goto Graphics;
		  case 'L':	c = 1;	goto Graphics;
		  case 'M':	/* elite on */
			setfont(PITCH12);
			hspace = HRES/12;
			break;
		  case 'N':	/* perf margin */
			vmarg = getc(f);
			break;
		  case 'O':	/* perf margin off */
			vmarg = 0;
			break;
		  case 'P':	/* elite off */
			clrfont(PITCH12);
			hspace = HRES/10;
			break;
		  case 'Q':	/* right margin */
			hend = hspace*getc(f);
			break;
		  case 'R':	/* char set */
			charset = getc(f);
			break;
		  case 'S':	/* script on */
			if (!getc(f))
				setfont(SUPER);
			setfont(SCRIPT);
			break;
		  case 'T':
			clrfont(SCRIPT|SUPER);
			break;
		  case 'U':	/* unidir on */
			break;
		  case 'W':	/* expand off/on */
			/* ?? */
			break;
		  case 'Y':	c = 2;	goto Graphics;
		  case 'Z:':	c = 3;	goto Graphics;
		  case 'e':	/* tab incr */
			if (!getc(f))
				htab = hspace*getc(f);
			else
				vtab = vspace*getc(f);
			break;
		  case 'f':	/* n space */
			if (!getc(f))
				hpos = hspace*getc(f);
			else {
				vpos = vspace*getc(f);
				goto CheckPage;
			}
			break;
		  case 'l':	/* left margin */
			hbeg = hspace*getc(f);
			break;
		  case 'm':	/* control 2 */
			break;
		  case 's':	/* print speed */
			break;
		}
		break;
	  case ' ':
		if (tstfont(UNDLINE))
			goto Printing;
		hpos += hspace;
		break;
	  default:
		if (!isprint(c&0x7F))
			break;
	  Printing:
		/* output page begin */
		if (!started) {
			page ++;
			fprintf(stdout, "%%%%Page: ? %d\n", page);
			fprintf(stdout, "P+\n");
			started = 1;
			lastfont = -1;
			hlast = -1;
		}
		/* output position */
		if (vpos!=vlast || hpos!=hlast) {
			fprintf(stdout, "%d %d M ", vpos, hpos);
			vlast = vpos; hlast = hpos;
		}
		/* output font information */
		if (font!=lastfont) {
			fprintf(stdout, "%d F ", font);
			lastfont = font;
		}
		/* output text */
		putc('(', stdout);
		while (c==' ' || isprint(c&0x7F)) {
			if (c=='\\' || c=='(' || c==')')
				putc('\\', stdout);
			putc(c, stdout);
			hpos += hspace;
			c = getc(f);
		}
		putc(')', stdout);
		fputs("S\n", stdout);
		ungetc(c, f);
		/* underline text */
		if (tstfont(UNDLINE)) {
			i = hpos;
			hpos = hlast;
			fprintf(stdout, "%d %d M ", vpos, hpos);
			fprintf(stdout, "%d F ", font&PITCH12);
			lastfont = font&PITCH12;
			putc('(', stdout);
			while (hpos<i) {
				putc('_', stdout);
				hpos += hspace;
			}
			putc(')', stdout);
			fputs("S\n", stdout);
		}
		hlast = hpos;
		if (tstfont(SCRIPT))
			hlast = -1;
		break;
	} while (!feof(f));
}
