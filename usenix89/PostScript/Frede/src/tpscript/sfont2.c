/*
 * sfont2.c
 * routines for manipulating special font 2 (the characters generated
 * specifically to match troff requirements - because kludging the available
 * bracket chars could never be made to work completely properly)
 */

#include	"tpscript.h"

#include	"sfont2defs.h"

struct spcdefs {
	short	s2_index;	/* index in encoding structure == value in width tables */
	short	s2_width;	/* width of char as in width tables */
	char	*s2_name;	/* troff 2(?)-char name */
	char	*s2_def;	/* the postscript definition to build the char */
};

		/*
		 * if chars are wider than this then you cannot use the
		 * default C.setC procedure for calling setcachedevice
		 * This comes about because, while I could just call
		 * setcachedevice with the values in FontBBox, I assume that
		 * I will save caching memory if i dont use a value which is
		 * twice as big as I really need for most chars.
		 * ... But do I really save anything since the definition is
		 * now a bit bigger? - probably I do, since the space defined
		 * by the bounding box at 10 pt is about 2400 pixels
		 * (~ 300 bytes)
		 */
#define	DFLT_CACHE_WIDTH	50

	/*
	 * note - I really should look up the width in the width tables and
	 * just use that but when we run s2init() we havent read in the
	 * tables yet - and while about it i should probably read in the
	 * index value as well.
	 */

typedef	struct	spcdefs	S2DEF;

S2DEF	s2defs[]  = {
	{ 0101, 50, "bv", "C.bv" },
	{ 0102, 50, "lt", DEF_lt },
	{ 0103, 50, "lk", DEF_lk },
	{ 0104, 50, "lb", DEF_lb },
	{ 0105, 50, "rt", DEF_rt },
	{ 0106, 50, "rk", DEF_rk },
	{ 0107, 50, "rb", DEF_rb },
	{ 0110, 50, "lc", DEF_lc },
	{ 0111, 50, "lf", DEF_lf },
	{ 0112, 50, "rc", DEF_rc },
	{ 0113, 50, "rf", DEF_rf },
	{ 0114, 0, "br",  DEF_br },
	{ 0115, 50, "rn", DEF_rn },
	{ 0116, 100, "ci", DEF_ci },
	{ 0117, 17,  "||", DEF_sp_6 },
	{ 0120, 8,   "^^", DEF_sp_12 },
	{ 0121, 80,  "r1", DEF_r1 },
	{ 0122, 80,  "r2", DEF_r2 },
	{ 0, 0,    "",   "" }
};
#define	NUM_S2DEFS	(sizeof s2defs/sizeof(S2DEF) -1)

	/*
	 * bracket font initialisation
	 */
char	*bf_0init[] = {
	"/BracketFontDict 9 dict def /$workingdict 10 dict def",
	"BracketFontDict begin",
		/* locally defined font */
	"/FontType 3 def",
		/* give it a name like the built-in fonts */
	"/FontName (Bracket) cvn def",
		/* the standard matrix for font definitions */
	"/FontMatrix [ 0.001 0 0 0.001 0 0] def",
	"/FontBBox [ -50 -250 1000 1000 ] def",
		/* will be a sparse array so fill initally with .notdef */
	"/Encoding 256 array def 0 1 255 { Encoding exch /.notdef put } for",
	"Encoding",
	(char *) 0 };
		/* at this point we put the real encoding and the procedures for
		* building the chars
		*/

		/*
		 * predefined common routines for inclusion in CharProcs
		 */
char	*bf_CPinit[] = {
			/*
			 * define setcachedevice with default width passed
			 * as argument (on stack)
			 */
	"/setC { 0 -50 -250 500 1000 setcachedevice} def",
			/*
			 * common stuff for solid vert bar (\(bv)
			 * also used for floor and ceilings
			 */
	"/C.bv {220 -250 moveto 0 1000 rlineto",
		"60 0 rlineto 0 -1000 rlineto fill } def",
			/*
			 * stuff for horiz bar for ceiling
			 * X coord passed as arg
			 */
	"/C.barc { 750 moveto 180 0 rlineto 0 -60 rlineto -180 0 rlineto fill } def",
			/*
			 * stuff for horiz bar for floor
			 */
	"/C.barf { -250 moveto 180 0 rlineto 0 60 rlineto -180 0 rlineto fill } def",
			/*
			 * common routine for drawing the end parts of brackets
			 * e.g. \(lt, \(lb
			 * start with starting x,y on stack
			 * draw line and then curve to tip
			 */
	"/C.brk.end { 1 setlinewidth moveto rlineto rcurveto",
			/* back to start, over by width of line */
			/* draw line and then curve to near tip */
		"reversepath 60 0 rlineto rlineto rcurveto fill } def",

			/*
			 * set the linewidth rounded to the nearest pixel
			 * We need a dummy y entry as well but ignore it.
			 * The reason for using the X rounded value is only
			 * relevant when ( x-scale != y-scale ):
			 * since troff keeps char width constant and scales
			 * the height up/down, we also scale relative to the
			 * width.
			 */
	"/C.setl {dup dtransform exch round exch idtransform pop setlinewidth } def",

	(char *) 0 };

#define	N_CP_PROCS	6	/* number of extra common procs */

		/*
		 * the mandatory BuildChar routine for the font description
		 */
char	*bf_1init[] = {
	"/BuildChar",
	"{",
	"	$workingdict begin",
	"	/charcode exch def",
	"	/fontdict exch def",
	"	fontdict /CharProcs get begin",
	"	fontdict /Encoding get",
	"	charcode get load",
	"	gsave",
	"	0 setlinecap 0 setgray newpath",
	"	exec",
	"	grestore",
	"	end end",
	"} def end",
	"/BracketFont BracketFontDict definefont pop",
	(char *) 0 };

		/*
		 * setup the definitions for building our own special font
		 * for the characters that the laserwriter does not do
		 * properly
		 */
s2init()
{
	register	char	**ptab;
	register	S2DEF	*s2p;

	ptab = bf_0init;
	while(*ptab)
		fprintf(postr, "%s\n", *ptab++);
			/*
			 * now put the entries in the encoding table
			 */
	for ( s2p = &s2defs[0] ; s2p->s2_index != 0 ; s2p++ )
		fprintf( postr, "dup %d /C%s put\n",
				s2p->s2_index, s2p->s2_name);
			/*
			 * define the CharProcs dictionary which contains
			 * the procedures required by BuildChar to construct
			 * the characters
			 */
	fprintf( postr, "pop\n/CharProcs %d dict dup begin\n",
			NUM_S2DEFS + N_CP_PROCS );

	ptab = bf_CPinit;
	while(*ptab)
		fprintf(postr, "%s\n", *ptab++);

	for ( s2p = &s2defs[0] ; s2p->s2_index != 0 ; s2p++ )
	{
		if ( s2p->s2_width == DFLT_CACHE_WIDTH )
			fprintf( postr, "/C%s {\n%d setC\n%s\n} def\n",
				s2p->s2_name,
				(int)(s2p->s2_width * respunits),
					/* this is unwise because it assumes
					 * unitwidth 10 in DESC file
					 */
				s2p->s2_def);
		else
			fprintf( postr,
				"/C%s {\n%d 0 -50 -250 %d 1000 setcachedevice\n%s\n} def\n",
				s2p->s2_name,
				(int)(s2p->s2_width * respunits),
				(int)(s2p->s2_width * respunits),
				s2p->s2_def);
	}
	fputs( "end def\n", postr);
	ptab = bf_1init;
	while(*ptab)
		fprintf(postr, "%s\n", *ptab++);
}

