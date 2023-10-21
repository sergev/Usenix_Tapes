/*
 *	lpscript - version 3.1
 *
 *	Convert plain text to postscript
 *
 *		- Stephen Frede, UNSW, Australia (stephenf@elecvax.oz)
 *			...!seismo!munnari!elecvax!stephenf
 */

#ifndef	lint
static char id[] = "lpscript - version 3.1; Stephen Frede, UNSW, Australia";
#endif	lint

#include	<stdio.h>
#include	"pscript.h"

#define	PAGEOFFSET	(1.0*PU_CM)
#define	FONTSIZE	10.0		/* default font size (in points) */
#define	TABSIZE		8
#define	FONT		"Courier"
#define	DOUBLESPACE	1.8		/* line spacing for double spacing */


char	usage[] = "Valid lpscript options:\n\t-o[offset]\n\t-r[rotation]\n\
\t-s[fontsize]\n\t-a[fontaspect]\n\t-p[pitch]\n\t-f[font]\n\t-t[tabsize]\n\
\t-h[horizontal_spacing]\n\t[-S]\t(manual feed)\n\t[-L]\t(legal paper type)\n";
int	tabsize = TABSIZE;		/* in character positions */

double	atof();

/* ARGSUSED */
main(argc, argv)
int	argc;
char	**argv;
{
	int	status = 0;	/* exit status (no. errors occured) */
	float	pageoffset = 18.0,
		fontsize = FONTSIZE,
		aspect = 1.0,
		linepitch = 0,
		spacing = 0.0,
		rotation = PD_ROTATION;
	char	*fontname = FONT;
	FILE	*istr;
	bool	manualfeed = FALSE,
		doublespace = FALSE;
	int	pagetype	=
#ifdef	ALW
				PT_A4;
#else	ALW
				PT_DEFAULT;
#endif	ALW

	postr = stdout;
	argv++;		/* skip program name */
	while(*argv && **argv == '-')
	{
		char	c;

		(*argv)++;	/* skip the '-' */
		c = **argv;	/* option letter */
		(*argv)++;	/* skip option letter */
		switch(c)
		{
			case 'o':	/* offset */
				if(**argv == '\0')
					pageoffset = PAGEOFFSET;
				else
					pageoffset = atof(*argv) * PU_CM;
				break;

			case 'r':	/* rotation */
				if(**argv == '\0')
					rotation = 90.0;
				else
					rotation = atof(*argv);
				break;

			case 'p':	/* pitch (line spacing) */
				if(**argv == '\0')
					doublespace = TRUE;
				else
					linepitch = atof(*argv);
				break;

			case 's':	/* font size */
				if(**argv == '\0')
					fontsize = 12.0;
				else
					fontsize = atof(*argv);
				break;

			case 't':	/* tab size */
				if(**argv == '\0')
					tabsize = 4;
				else
					tabsize = (int) atof(*argv);
				break;

			case 'f':	/* font */
				if(**argv == '\0')
					fontname = "Times-Roman";
				else
					fontname = *argv;
				break;

			case 'h':	/* horizontal spacing */
				if(**argv == '\0')
					spacing = 0.25;
				else
					spacing = atof(*argv);
				break;

			case 'a':	/* character aspect ratio */
				if(**argv == '\0')
					aspect = 1.0;
				else
					aspect = atof(*argv);
				break;

			case 'S':	/* manual feed */
				manualfeed = TRUE;
				break;

			case 'L':	/* legal paper type */
				pagetype = PT_LEGAL;
				break;

			default:
				fprintf(stderr, "Unknown option: '%c'\n", c);
				status++;
				break;
		}
		argv++;
	}
	if(status)
	{
		fprintf(stderr, usage);
		exit(status);
		/* NOTREACHED */
	}
	if(doublespace)
		linepitch = fontsize * DOUBLESPACE;
	if(linepitch == 0)
		linepitch = fontsize;
	spacing *= fontsize;
	init(fontsize, aspect, pageoffset, linepitch, rotation, fontname,
		spacing, manualfeed, pagetype);
	if(! *argv)
	{
		fprintf(postr, "(stdin ...\\n) ps\n");
		process(stdin);
	}
	else while(*argv)
	{
		if((istr = fopen(*argv, "r")) == NULL)
		{
			perror(*argv);
			status++;
		}
		else
		{
			fprintf(postr, "('%s' ...\\n) ps\n", *argv);
			process(istr);
			fclose(istr);
		}
		argv++;
	}
	pcommfinish(-1, (char *)0);
	putc('\004', postr);
	exit(status);
	/* NOTREACHED */
}

process(istr)
FILE	*istr;
{
	register int	ch;
	register int	x;	/* used for tab calculations */
	register int	prefix;

	x = 0;
	prefix = 0;
	while((ch=getc(istr)) != EOF)
	{
		if(!prefix)
		{
			putc('(', postr);
			prefix = 1;
		}
		if(ch < ' ' && ch != '\t' && ch != '\n' && ch != '\r' && ch != '\f')
			ch = '?';
		switch(ch)
		{	
		case '\t':
			{
			int	n = x + tabsize - (x % tabsize);

			while(x < n)
				pch(' '), x++;
			}
			break;	
		case '\n':
			x = 0;
			fprintf(postr, ")n\n");
			prefix = 0;
			break;
		case '\r':
			x = 0;
			fprintf(postr, ")r\n");
			prefix = 0;
			break;
		case '\f':
			x = 0;
			fprintf(postr, ")n dpage\n");
			prefix = 0;
			break;
		default:
			pch(ch);
			x++;
		}
	}
	if(prefix)
		fprintf(postr, ")n\n");
	fprintf(postr, "dpage ( ---\\n) ps\n");
}

char	*inittab[] = {
	/* current y coord */
	"/y { currentpoint exch pop } def",

	/* var to prevent trailing blank page */
	"/dopage false def",

	/* print a new page */
	"/dpage { dopage { page /dopage false def } if } def",

	/* print a line then move to the next */
	"/n",
	"{ spacing 0 3 -1 roll ashow",
	"  0 y linepitch add moveto",
	"  /dopage true def",
	"  y pgbot lt { dpage } if",
	"} def",

	/* print a line - then return to start */
	"/r",
	"{ spacing 0 3 -1 roll ashow",
	"  0 y moveto",
	"  /dopage true def",
	"} def",

	(char *)0 };

init(fontsize, aspect, pageoffset, linepitch, rotation, fontname,
	spacing, manualfeed, pagetype)
float	fontsize,
	aspect,
	pageoffset,
	linepitch,
	spacing,
	rotation;
char	*fontname;
bool	manualfeed;
int	pagetype;
{
	register char	**p;

	pcomminit(0.0, rotation, pagetype, manualfeed,
			fontname, (char *)0, "lpscript");
	p = inittab;
	while(*p)
		fprintf(postr, "%s\n", *p++);
	fprintf(postr, "/%s findfont [ %.1f 0 0 %.1f 0 0 ] makefont setfont\n",
		fontname, fontsize, fontsize * aspect);
	fprintf(postr, "/linepitch %.1f def\n", -linepitch);
	fprintf(postr, "/spacing %.1f def\n", spacing);
	/* subtract linespacing (add -'ve) to get top text baseline */
	fprintf(postr, "/pgtop pgtop linepitch add def\n");
	/* calculate bottom text baseline */
	fprintf(postr, "/pgbot currentfont /FontBBox get 1 get neg 1000 div %.1f mul def\n", fontsize * aspect);
	/* apply horizontal offset, if any */
	fprintf(postr, "%.1f 0 translate\n", pageoffset);

	/* save state */
	endinit();
}
