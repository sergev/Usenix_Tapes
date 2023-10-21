/*
 *	tpscript.c
 *	Troff post-processor for postscript devices
 *
 *	Original program by Stephen Frede (stephenf@elecvax.oz)
 *		Dept. Comp. Sci., University of NSW, Sydney, Australia.
 *					...!seismo!munnari!elecvax!stephenf
 *
 *	Extensive modifications by Cameron Davidson (probe@mm730.uq.oz)
 *				University of Queensland, Brisbane, Australia
 *
 *	Other changes by Michael Rourke (michaelr@elecvax.oz) UNSW.
 */

/* NOTES:
 *
 * Originally, changes to a new font would not take effect until
 * characters from that font were required to be printed, but this
 * means that commands passed through to postscript directly (via \!!)
 * may end up with the wrong font. So now font changes actually happen
 * when requested (or needed in the case of the special font).
 *
 */

/*	The language that is accepted by this program is produced by the new
 *	device independent troff, and consists of the following statements,
 *
 *
 *	sn		set the point size to n
 *	fn		set the typesetter font to the one in position n
 *	cx		output the ASCII character x 
 *	Cxyz		output the code for the special character xyz. This
 *			command is terminated by white space.
 *	Hn		go to absolute horizontal position n
 *	Vn		go to absolute vertical position n ( down is positive )
 *	hn		go n units horizontally from current position
 *	vn		go n units vertically from current position
 *	nnc		move right nn units, then print the character c. This
 *			command expects exactly two digits followed by the
 *			character c.
 *			( this is an optimisation that shrinks output file
 *			size by about 35% and run-time by about 15% while
 *			preserving ascii-ness)
 *	w		paddable word space - no action needed
 *	nb a		end of line ( information only - no action needed )
 *			b = space before line, a = space after line
 *	pn		begin page n
 *	in		stipple as no. from 1 to n (BERK).
 *	P		spread ends -- output it (put in by rsort) (BERK).
 *	# ...\n		comment - ignore.
 *	! ...\n		pass through uninterpreted (LOCAL MOD).
 *	Dt ...\n	draw operation 't':
 *
 *		Dl dx dy		line from here to dx, dy
 *		Dc d		circle of diameter d, left side here
 *		De x y		ellipse of axes diameter x,y, left side here
 *		Da dx1 dy1 dx2 dy2	arc counter-clockwise, start here,
 *					centre is dx1, dy1 (relative to start),
 *					end is dx2, dy2 (relative to centre).
 *		D~ x y x y ...	wiggly line (spline) by x,y then x,y ...
 *		Dt d		set line thickness to d pixels (BERK).
 *		Ds d		set line style mask to d (BERK).
 *		Dg x y x y ...	gremlin (BERK).
 *
 *	x ... \n	device control functions:
 *
 *		x i		initialize the typesetter
 *		x T s		name of device is s
 *		x r n h v	resolution is n units per inch. h is
 *				min horizontal motion, v is min vert.
 *				motion in machine units.
 *		x p		pause - can restart the typesetter
 *		x s		stop - done forever
 *		x t		generate trailer
 *		x f n s		load font position n with tables for 
 *				font s. Referring to font n now means
 *				font s.
 *		x H n		set character height to n
 *		x S n		set character slant to n
 *
 *		Subcommands like i are often spelled out as "init"
 *
 *	Commands marked "BERK" are berzerkeley extensions.
 *
 */

#include	"tpscript.h"

#define	FONTDIR	"/usr/lib/font"		/* where font directories live */

FILE	*Debug = NULL;		/* debugging stream if non-null */
char	*fontdir = FONTDIR;	/* where the fonts live */
char	*ifile = 0;		/* current input file name */
int	lineno,			/* line no. in current input file */
	npages = 0;			/* no. pages printed so far */
char	device[100],		/* device name, eg "alw" */
	errbuf[100];		/* tmp buffer for error messages */
int	hpos = 0,		/* current horizontal position */
	vpos = 0;		/* current vertical position (rel. TOP pg.) */
int	res,			/* resolution in THINGS/inch */
	hor_res,		/* min horizontal movement (in THINGS) */
	vert_res,		/* min vertical movement (in THINGS) */
	respunits;
float	rotation = 0;		/* page orientation (degrees) */
int	currtfont = DEF_FONT,	/* current font number selected by troff */
	papertype = 		/* paper type (different imageable regions) */
#ifdef	ALW
		PT_A4;
#else
		PT_DEFAULT;
#endif
bool	manualfeed = FALSE;	/* normally auto-feed */

/* due to an obscure bug in ditroff, sometimes no initial 'p' command
 * is generated, so we have to remember if any output has happened
 * to decide if a 'p' causes a page print or not.
 */
bool	firstpage = TRUE;	/* nothing yet printed anywhere */

/* font parameters */
struct	fontparam 
	tfp,		/* current troff font parameters */
	pfp;		/* current postscript font parameters */


/* table of font descriptions */
struct fontdesc 
	*fontd = NOFONTDESC,
	*spcfnt1 = NOFONTDESC,	/* special font */
	*spcfnt2 = NOFONTDESC;	/* special font 2 */

/* font mount table - array of pointers to font descriptions */
struct fontdesc	**fontmount;

/* mapping between troff font names and builtin font names
 * This should go in the internal name part of the font description
 * itself, but there is only 10 bytes allocated (see dev.h).
 */
struct fontmap  fontmap[] = {
	{ "R", "Times-Roman" },
	{ "I", "Times-Italic" },
	{ "B", "Times-Bold" },
	{ "BI", "Times-BoldItalic" },
	{ "S", "Symbol" },
	{ "S2", "BracketFont" },	/* locally defined special font */
	{ "C", "Courier" },
	{ "CW", "Courier" },		/* synonym: constant width */
	{ "CB", "Courier-Bold" },
	{ "CO", "Courier-Oblique" },
	{ "CX", "Courier-BoldOblique" },
	{ "H", "Helvetica" },
	{ "HR", "Helvetica" },		/* two-char name for H */
	{ "HB", "Helvetica-Bold" },
	{ "HO", "Helvetica-Oblique" },
	{ "HX", "Helvetica-BoldOblique" },
	{ (char *)0,	(char *)0 }
};

struct dev	dev;

short	*chartab = NULL;	/* char's index in charname array */
char	*charname = NULL;	/* special character names */
int	ncharname;		/* no. special character names */
int	nfonts = 0;		/* no. of fonts mounted */
int	nfontmount;		/* no. of font mount positions */

	/*
	 * this is the width that the printer will have moved following
	 * the last printed character, if troff then says to move a
	 * different amount we will shift the difference
	 */
int	width_pending	= 0;

bool	word_started	= FALSE;	/* we are in middle of word string */


int		strcmp();
char		*emalloc();
struct fontdesc *findfont();
struct fontmap	*getfmap();

main(argc, argv)
int		argc;
register char	**argv;
{
	register FILE	*istr;
	int		status = 0;
	extern 	double	atof();
#ifdef SPACING
	float		spacing;
#endif SPACING

	strcpy(device, DEF_DEV); /* just in case we get a "Di" before a "DT" */
	argv++;
	while(*argv && **argv == '-')
	{
		char	c;

		(*argv)++;	/* skip the '-' */
		c = **argv;
		(*argv)++;	/* skip the character */
		switch(c)
		{
			case 'D':	/* debug */
				Debug = stderr;
				break;

#ifdef SPACING
			case 'h':
				spacing = atof(*argv);
				break;
#endif SPACING
			case 'r':	/* rotate */
				if(**argv == '\0')
					rotation = 90.0;
				else
					rotation = atof(*argv);
				break;

			case 'S':	/* manual feed */
				manualfeed = TRUE;
				break;

			case 'L':	/* legal paper type */
				papertype = PT_LEGAL;
				break;

			case 't':
				postr = stdout;
				break;

			default:
				break;
		}
		argv++;
	}

	if (postr == NULL)
	{
#ifdef	GRIS
		postr = popen("exec sendfile -AC -aprinter -dbasser -ugris -e\"-R -qd\" -ntroff-alw", "w");
		if (postr == NULL)
			error(ERR_SNARK, "can't popen spooler");
#else	GRIS
		postr = stdout;
#endif	GRIS
	}

	if(! *argv)
	{
		ifile = "stdin";
		process(stdin);
	}
	else while(*argv)
	{
		if((istr=fopen(*argv, "r")) == NULL)
		{
			perror(*argv);
			status++;
		}
		else
		{
			ifile = *argv;
			process(istr);
			fclose(istr);
		}
		argv++;
	}
	if (postr != stdout)
		status += pclose(postr);
	exit(status);
	/* NOTREACHED */
}

process(istr)
FILE	*istr;
{
	int	ch;
	char	str[50];
	int	n;
	register int	i;

	lineno = 1;	/* start processing 1st input line */

	while((ch=getc(istr)) != EOF)
	{
			/*
			 * the first switch group can safely be scanned without
			 * having to first ensure the horizontal position is
			 * up to date.
			 */
		switch(ch)
		{
			/* noise */
			case ' ':
			case '\0':
				continue;

			case '\n':
				lineno++;
				continue;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				ungetc(ch, istr);
				fscanf(istr, "%2d", &n);

				width_pending -= n;
				hpos += n;

				/* drop through to process the next char */

			case 'c':	/* ascii character */

					/*
					 * if this char and preceeding were
					 * not simply successive chars in the
					 * same word then we need some
					 * horizontal motion to reset position
					 */
				if ( width_pending != 0 )
					hgoto( );

				ch = getc(istr);

				width_pending += GETWIDTH( tfp.fp_font,
					(i = tfp.fp_font->f_fitab[ch - NUNPRINT] ));

				if(ch != ' ')
					putch(tfp.fp_font->f_codetab[i] & BMASK);
				else
					putch(' ');	/* no code for ' ' */
				continue;

			case 'C':	/* troff character */

				if ( width_pending != 0 )
					hgoto( );

				fscanf(istr, "%s", str);
				putspec(str);
				continue;

			case 'h':	/* relative horizontal movement */
				fscanf(istr, "%d", &n);

				/*
				 * we continually accumulate horizontal
				 * motions and all relative requests are
				 * translated into absolute ones.
				 * This avoids accumulation of character
				 * width rounding errors
				 * beyond a single word. (These errors arise
				 * because troff requires widths to be
				 * integral to the unit resolution whereas in
				 * the printer they may be fractional).
				 */

				hpos += n;
				if ( ( width_pending -= n ) != 0 )
					hgoto( );	/* most likely end of word */

				continue;

			case 'w':
				firstpage = FALSE;
				CLOSEWORD();
				continue;

			case 'n':	/* newline */
				fscanf(istr, "%*f %*f");
				width_pending = 0;	/* doesn't matter now */
				continue;

			case 'f':	/* select font no. */
				fscanf(istr, "%d", &n);
				if(n > nfonts || n < 0 || fontmount[n] == NULL)
				{
					sprintf(errbuf, "ERROR: font %d not mounted",
						n);
					error(ERR_WARN, errbuf);
				}
				else
				{
					tfp.fp_font = fontmount[n];
					currtfont = n;
				}
				continue;

			case 's':	/* size in points */
				fscanf(istr, "%d", &n);
				if(n <= 0)
				{
					sprintf(errbuf, "Illegal point size %d\n", n);
					error(ERR_WARN, errbuf);
				}
				else
				{
					tfp.fp_size = n;
					tfp.fp_height = (float) n;
				}
				continue;

			case 'H':	/* absolute horizontal position */

				fscanf(istr, "%d", &hpos);
				hgoto();
				continue;

			case 'V':	/* absolute vertical position */
				fscanf(istr, "%d", &vpos);
				vgoto();
				continue;

			case 'v':	/* relative vertical movement */
				fscanf(istr, "%d", &n);
				vmot(n);
				continue;

		}
			/*
			 * If the input char is in the second group
			 * then we must make sure the printer is positioned
			 * where troff thinks it is
			 * and close any word currently being printed
			 */
		if ( width_pending != 0 )
			hgoto( );
		else
			CLOSEWORD();

		switch(ch)
		{
			case 'x':	/* device control function */
				devcntrl(istr);
				break;

			case 'D':	/* draw */
				draw(istr);
				break;

			case 'p':	/* new page */
				fscanf(istr, "%d", &n);
				page(n);
				break;

			case '#':	/* comment */
				while((ch=getc(istr)) != '\n' && ch != EOF);
				lineno++;
				break;

			case 't':	/* text */
				text(istr);
				break;

# ifdef HASH
			/*
			 * debug - to be manually inserted in input stream if needed
			 * if n >= 0 && n <= HASH_SIZE
			 *	then will print entire hash contents
			 * otherwise will dump just names in hash_tab[n] entry
			 */
			case 'Z':
				fscanf(istr, "%d", &n);
				dumphash( n );
				break;
				
# endif

			case '!':	/* pass through uninterpreted */
				setfont(FALSE);	/* ensure current font is set */
				putc('\n', postr);
				while((ch=getc(istr)) != '\n' && ch != EOF)
					putc(ch, postr);
				break;

			default:
				sprintf(errbuf, "Unknown command '%c'", ch);
				error(ERR_FATAL, errbuf);
		}
	}
}

devcntrl(istr)
FILE	*istr;
{
	char		str[50];
	int		fontn,
			ch;
	float		f;

	fscanf(istr, "%s", str);
	switch(*str)
	{
		case 'i':	/* device initialisation */
			initfonts(device);
			devinit();
			break;

		case 'T':	/* we had better get this before an 'init' */
			fscanf(istr, "%s", device);
			break;

		case 'r':	/* resolution */
			fscanf(istr, "%d %d %d", &res, &hor_res, &vert_res);
			respunits = res / PU_INCH;
			break;

		case 'f':	/* load font */
			fscanf(istr, "%d %s", &fontn, str);
			loadfont(str, fontn);
			break;

		case 's':	/* stop */
			finish(0);
			break;

		case 'p':	/* pause */
			break;

		case 't':	/* trailer */
			break;

		case 'H':	/* character height (in points) */
			fscanf(istr, "%f", &f);
			if(f <= 0 || f > 1000)
			{
				sprintf(errbuf,
					"Illegal character height %.1f", f);
				error(ERR_WARN, errbuf);
			}
			else
				tfp.fp_height = f;
			break;

		case 'S':
			fscanf(istr, "%f", &f);
			if(f < -80 || f > 80)
			{
				sprintf(errbuf, "Illegal character slant %.1f degrees", f);
				error(ERR_WARN, errbuf);
			}
			else
				tfp.fp_slant = f;
			break;

		default:
			sprintf(errbuf, "Unknown device control '%s'", str);
			error(ERR_WARN, errbuf);
			break;
	}
	while((ch=getc(istr)) != '\n' && ch != EOF);	/* skip rest of input line */
	lineno++;
}

error(errtype, errmsg)
int	errtype;
char	*errmsg;
{
	switch(errtype)
	{
		case ERR_WARN:
			fprintf(stderr, "Warning");
			break;

		case ERR_FATAL:
			fprintf(stderr, "Error");
			break;

		case ERR_SNARK:
			fprintf(stderr, "Snark");
			break;
	}
	fprintf(stderr, "\t%s pscript input, line %d of '%s'\n",
		errtype == ERR_SNARK ? "at" : "in",
		lineno, ifile);
	if(errmsg && *errmsg)
		fprintf(stderr, "\t%s\n", errmsg);
	if(errtype != ERR_WARN)
		finish(1);
}

finish(status)
int	status;
{
	page(-1);
	pcommfinish(npages, "");
	if(status != 0)
		fprintf(stderr, "\t... aborted processing\n");
	exit(status);
}

/*
 *	Output the postscript "prologue" that is the start of each program
 *	generated. This sets up definitions, sets the scale to be troff
 *	units, etc.
 *	By convention, single character variables are procedure names,
 *	while multi-character variables are local to procedures.
 */

char	*inittab[] = {
	/* initialise current path to non-null */
	"0 0 moveto",
	/* fix to make "joined" lines better */
	"2 setlinecap",
	/* routine for RELATIVE HORIZONTAL RIGHT */
	/* need no more
	"/x { 0 rmoveto } def",
	/* routine for RELATIVE VERTICAL DOWN */
	"/y { neg 0 exch rmoveto } def",
	/* routine for ABSOLUTE HORIZONTAL (rel left edge page) */
	"/X { currentpoint exch pop moveto } def",
	/* routine for ABSOLUTE VERTICAL (rel top of page) */
	"/Y { pgtop exch sub currentpoint pop exch moveto } def",
#ifdef	SPACING
	"/s { currentpoint spacing 0 5 -1 roll ashow moveto } def",
#else
	"/s { show } def",
#endif	SPACING
	"/l { neg rlineto currentpoint stroke moveto } def",
	/* circle - arg is diameter.
	 * Current point is left edge
	 */
	"/c {",
	/* save radius and current position */
	"2 div /rad exch def currentpoint /y0 exch def /x0 exch def",
	/* draw circle */
	"newpath x0 rad add y0 rad 0 360 arc stroke",
	/* move to right edge of circle */
	"x0 rad add rad add y0 moveto",
	" } def",
	/* Arc anticlockwise, currentpoint is start;
	 * args are dx1, dy1 (centre relative to here)
	 * and dx2, dy2 (end relative to centre).
	 */
	"/a {",
	/* save all parameters */
	"/y2 exch neg def /x2 exch def /y1 exch neg def /x1 exch def",
	/* move to centre, push position for moveto after arc */
	"x1 y1 rmoveto currentpoint",
	/* push centre for args to arc */
	"currentpoint",
	/* calculate and push radius */
	"x2 x2 mul y2 y2 mul add sqrt",
	/* start angle */
	"y1 neg x1 neg atan",
	/* end angle */
	"y2 x2 atan",
	/* draw the arc, and move to end position */
	"newpath arc stroke moveto x2 y2 rmoveto",
	"} def",
	/* ellipse - args are x diameter, y diameter;
	 * current position is left edge
	 */
	"/e {",
	/* save x and y radius */
	"2 div /yrad exch def 2 div /xrad exch def",
	/* save current position */
	"currentpoint /y0 exch def /x0 exch def",
	/* translate to centre of ellipse */
	"x0 xrad add y0 translate",
	/* scale coordinate system */
	"xrad yrad scale",
	/* draw the ellipse (unit circle in scaled system) */
	"newpath 0 0 1 0 360 arc",
	/* restore old scale + origin */
	"savematrix setmatrix",
	/* actually draw the ellipse (with unscaled linewidth) */
	"stroke",
	/* move to right of ellipse */
	"x0 xrad add xrad add y0 moveto",
	"} def",
		/*
		 * common procedure for spline curves
		 */
	"/spln {",
		/* setup curve, remember where we are, fill in line,
		** and reset current point
		*/
	"rcurveto currentpoint stroke moveto",
	"} def",
	/* routine to select a font */
	"/ft { /fonttype exch def /xsiz exch def /ysiz exch def /sl exch def",
	" fonttype [ xsiz pt 0 sl sin sl cos div ysiz pt mul ysiz pt 0 0 ]",
	" makefont setfont",
	/* point size also affects linewidth (see Pic user manual, p. 17) */
	" xsiz 1.7 div setlinewidth } def",
	(char *) 0 };


devinit()
{
	register char	**ptab;
	register int	i;

	/* postscript basic units are "1/PU_INCH" inches.
	 * Normally PU_INCH=72, making postscript units points (1/72 inch)
	 * Scale postscript to accept whatever resolution we are given
	 * Typically res=300 for a 300 dot/inch laser printer
	 */
	pcomminit(PU_INCH / (float) res, rotation, papertype, manualfeed, 0,
		(char *)0, "troff->tpscript");
	ptab = inittab;
	while(*ptab)
		fprintf(postr, "%s\n", *ptab++);
	/* conversion back to points for font sizes etc. */
	fprintf(postr, "/pt { %d mul } def\n", respunits);

#if	defined(UQMINMET) && !defined(ALW)
			/* to compensate for "setmargins" */
	fprintf( postr, "\n-90 230 translate\n" );
#endif
	/* All graphics transformations have been done. Save the
	 * transformation matrix
	 */
	fprintf(postr, "/savematrix matrix currentmatrix def\n");
#ifdef SPACING
	/* set increased character spacing (if any) */
	fprintf(postr, "/spacing %.1f pt def\n", spacing);
#endif SPACING

	s2init();	/* initialise special font 2 */

	/* set up font abbreviations */
	for(i=1; i<nfonts+1; i++)
		fprintf(postr, "/f.%s /%s findfont def\n",
			fontd[i].f_extname, fontd[i].f_intname);
	/* select default current font */
	tfp.fp_size = DEF_SIZE;
	tfp.fp_height = (float) DEF_SIZE;
	tfp.fp_slant = 0;
	tfp.fp_font = &fontd[DEF_FONT];
	pfp.fp_font = (struct fontdesc *) NULL;
	setfont(FALSE);

	/* save state */
	endinit();
}


/*
 *	Called when some use of characters or line-drawing
 *	is about to be made, to ensure that the correct font and
 *	line thickness is selected in postscript.
 */
setfont(force)
bool	force;
{

	if(tfp.fp_size == pfp.fp_size &&
		tfp.fp_height == pfp.fp_height &&
		tfp.fp_slant == pfp.fp_slant &&
		tfp.fp_font == pfp.fp_font &&
		! force)
		return;
	CLOSEWORD();
	fprintf(postr, "\n%.1f %.0f %d f.%s ft",
		tfp.fp_slant,
		tfp.fp_height, tfp.fp_size,
		tfp.fp_font->f_extname);
	pfp = tfp;
}

draw(istr)
FILE	*istr;
{
	int	ch;
	int	x, y,
		x1, y1,
		d;

	setfont( FALSE );	/* in case of size change affecting line thickness */

	switch(ch=getc(istr))
	{
		case 'l':
			fscanf(istr, "%d %d", &x, &y);
			fprintf(postr, "\n%d %d l", x, y);
			break;

		case 'c':
			fscanf(istr, "%d", &d);
			fprintf(postr, "\n%d c", d);
			break;

		case 'e':
			fscanf(istr, "%d %d", &x, &y);
			fprintf(postr, "\n%d %d e", x, y);
			break;

		case 'a':
			fscanf(istr, "%d %d %d %d", &x, &y, &x1, &y1);
			fprintf(postr, "\n%d %d %d %d a", x, y, x1, y1);
			break;

		case '~':
			draw_spline( istr );
			break;

		default:
			sprintf(errbuf, "Illegal draw function '%c'", ch);
			error(ERR_WARN, errbuf);
			break;
	}
	while((ch=getc(istr)) != '\n' && ch != EOF);
	lineno++;
}


text(istr)
FILE	*istr;
{
	register int	ch;

	fprintf(postr, "\n(");
	while((ch=getc(istr)) != '\n' && ch != EOF)
		pch(ch);
	fprintf(postr, ")s");
}

page(n)
register int	n;
{
	hpos = 0; vpos = 0;
	/* for each page except the first, print the previous one */
	if(firstpage)
		firstpage = FALSE;
	else
	{
		fprintf(postr, "\npage");
		setfont(TRUE);
		resetspcl();		/* it forgets definitions on next page */
	}
	if(n >= 0)		/* beginning of a new page */
		fprintf(postr, "\n%%%%Page: %d %d\n", n, ++npages);
}

hgoto()
{
	CLOSEWORD();
	width_pending = 0;	/* doesn't matter now */
	fprintf(postr, "\n%d X", hpos);
}

vgoto( )
{
	CLOSEWORD();
	fprintf(postr, "\n%d Y", vpos);
}

vmot(n)
int	n;	/* +'ve is DOWN */
{
	CLOSEWORD();
	fprintf(postr, "\n%d y", n);
	vpos += n;
}

/*
 *	Read the DESC file for the current device. This includes
 *	information about all the common fonts. The format is:
 *
 *		struct dev	(see dev.h)
 *		point size table	(dev.nsizes * sizeof(short))
 *		char index table	(chtab; dev.nchtab * sizeof(short))
 *		char name table		(chname; dev.lchname)
 *
 *	followed by dev.nfonts occurrences of	
 *		struct font	(see dev.h)
 *		width tables		(font.nwfont)
 *		kern tables		(font.nwfont)
 *		code tables		(font.nwfont)
 *		font index table	(dev.nchtab + NASCPRINT)
 */

initfonts(devname)
char	*devname;
{
	register int			i;
	register struct fontdesc	*fd;
	FILE				*fstr;
	char				path[100];

	sprintf(path, "%s/dev%s/DESC.out", fontdir, devname);
	if((fstr=fopen(path, "r")) == NULL)
	{
		sprintf(errbuf, "Can't open '%s' (%s)",
			path, sys_errlist[errno]);
		error(ERR_FATAL, errbuf);
	}
	if(efread((char *)&dev, sizeof(dev), 1, fstr) != 1)
	{
		sprintf(errbuf, "%s: bad format (read dev failed)", path);
		error(ERR_SNARK, errbuf);
	}

	nfonts = dev.nfonts;
	/* nfontmount should be at least nfonts+2 */
	nfontmount = nfonts + 20;
	ncharname = dev.nchtab;
	fontd = (struct fontdesc *)
			emalloc((unsigned)(nfonts+2) * sizeof(struct fontdesc));
	fontmount = (struct fontdesc **)
			emalloc((unsigned)nfontmount * sizeof(struct fontdesc *));

	/* skip point size table */
	efseek(fstr, (int)((dev.nsizes + 1)*sizeof(short)));

	chartab = (short *) emalloc((unsigned)ncharname * sizeof(short));
	efread((char *)chartab, sizeof(* chartab), ncharname, fstr);

	charname = emalloc((unsigned)dev.lchname);
	efread(charname, sizeof(* charname), dev.lchname, fstr);

	hash_init();

	for(i=1; i <= nfonts; i++)
	{
		register int			nw;
		struct font			f;
		struct fontmap			*fm;

		/* read struct font header */
		efread((char *)&f, sizeof(f), 1, fstr);

		nw = (int)(f.nwfont & BMASK);	/* NO sign extension */
		fd = &fontd[i];
		fd->f_nent = nw;

		fd->f_widthtab = emalloc((unsigned)nw);
		fd->f_codetab = emalloc((unsigned)nw);
		fd->f_fitab = emalloc((unsigned)(ncharname+NASCPRINT));
		/* remember if font is special */
		if(f.specfont == 1)
		{
			if(spcfnt1 == NOFONTDESC )
				spcfnt1 = fd;
			else if ( spcfnt2 == NOFONTDESC )
				spcfnt2 = fd;
			else
			{
				sprintf( errbuf,
					"Too many special fonts, %s ignored",
					fd->f_extname );
				error(ERR_WARN, errbuf );
			}
		}

		fm = getfmap(f.namefont);
		if(fm)
		{
			fd->f_intname = fm->fm_intname;
			fd->f_extname = fm->fm_extname;
			fd->f_mounted = TRUE;
		}
		else
			fprintf(stderr, "font name '%s' not known\n",
				f.namefont);

		efread(fd->f_widthtab, sizeof(char), nw, fstr);
		efseek(fstr, 1*nw);	/* skip kern tables */
		efread(fd->f_codetab, sizeof(char), nw, fstr);
		efread(fd->f_fitab, sizeof(char), ncharname+NASCPRINT, fstr);
	}

	fclose(fstr);

	for(i=0; i < nfontmount; i++)
		fontmount[i] = NOFONTDESC;

	/* zeroth font desc entry reserved for "extra" fonts */
	fd = &fontd[0];
	fd->f_intname = "";	/* not NULL */
	fd->f_extname = "";	/* not NULL */
	fd->f_codetab = emalloc((unsigned)MAXCHARS);
	fd->f_fitab = emalloc((unsigned)(ncharname+NASCPRINT));
	fd->f_nent = MAXCHARS;

	/* sentinel fontdesc entry */
	fd = &fontd[nfonts+1];
	fd->f_intname = (char *)NULL;
	fd->f_extname = (char *)NULL;
	fd->f_nent = 0;
	fd->f_codetab = (char *)NULL;
	fd->f_fitab = (char *)NULL;
}

loadfont(extname, fpos)
char	*extname;	/* troff font name */
int	fpos;		/* font position */
{
	register struct fontdesc	*font;

	if(fpos > nfontmount || fpos < 0)
	{
		sprintf(errbuf, "Illegal font mount position %d\n", fpos);
		error(ERR_WARN, errbuf);
		return;
	}
	if ( (font = findfont(extname)) == (struct fontdesc *) NULL )
	{
		sprintf(errbuf, "No such font '%s'\n", extname);
		error(ERR_WARN, errbuf);
		return;
	}
	fontmount[fpos] = font;
}

struct fontmap *
getfmap(extname)
char	*extname;
{
	struct fontmap	*fm;

	fm = fontmap;
	while(fm->fm_intname && strcmp(fm->fm_extname, extname) != 0)
		fm++;
	if(fm->fm_intname)
		return(fm);
	else
		return((struct fontmap *)NULL);
}

#ifndef	UQMINMET

struct fontdesc *
findfont(extname)
char	*extname;
{
	struct fontdesc	*fd;

	fd = fontd;
	while(fd->f_intname && strcmp(fd->f_extname, extname) != 0)
		fd++;
	if(fd->f_intname)
		return(fd);
	else
		return((struct fontdesc *)NULL);
}

#else	UQMINMET
		/*
		 * find font including from possible synonym
		 * - use internal name instead of troff name.
		 * troff names need not uniquely correspond to a given
		 * internal name
		 */
struct fontdesc *
findfont(extname)
char	*extname;
{
	struct fontmap	*fm;
	struct fontdesc	*fd;

	if ( (fm = getfmap( extname )) == (struct fontmap *)NULL )
		return((struct fontdesc *)NULL);
	fd = fontd;
	while(fd->f_intname && strcmp(fd->f_intname, fm->fm_intname) != 0)
		fd++;
	if(fd->f_intname)
		return(fd);
	else
		return((struct fontdesc *)NULL);
}
#endif UQMINMET

char *
emalloc(size)
unsigned size;
{
	char		*malloc();
	register char	*s;

	s = malloc(size);
	if(s == NULL)
	{
		fprintf(stderr, "Ran out of memory allocating %u bytes\n",
			size);
		finish(1);
	}
	return(s);
}

efread(buf, size, nel, istr)
char	*buf;
int	size,
	nel;
FILE	*istr;
{
	register int n;

	if((n=fread(buf, size, nel, istr)) != nel)
		fprintf(stderr, "Bad format font file\n");
	return(n);
}

efseek(istr, offset)
FILE	*istr;
int	offset;
{
	if(fseek(istr, (long)offset, 1) != 0)
		fprintf(stderr, "Snark: Bad seek on font file\n");
}


putch(ch)
int	ch;
{
	setfont(FALSE);	/* ensure correct font */

	if ( word_started == FALSE ) {
		word_started = TRUE;
		putc('(', postr);
	}
	pch(ch);
}
