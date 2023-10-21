#define VERSION "2.10"

/*
 *
 * AUTHOR(s)
 *     Mark Senn wrote the early versions of this program for the
 *     BBN BitGraph.  Stephan Bechtolsheim, Bob Brown, Richard
 *     Furuta, James Schaad and Robert Wells improved it.  Norm
 *     Hutchinson ported the program to the Sun.  Neal Holtz ported
 *     it to the Apollo, and from there to producing PostScript
 *     output. Scott Jones added intelligent font substitution.
 *     Rob McMahon converted from this version to the Xerox 2700.
 */

/* Basic method:
 * 
 * Using the local font cacheing machinery that was in the previewer, we
 * can easily manage to send the bitmap for each chracter only once.  Two
 * passes are made over the DVI file.  The first pass simply outputs the
 * bitmaps for all characters that haven't been sent before for this
 * section of the document.  The section ends when the font job becomes
 * too complicated, or at the end of the document.  The second pass on a
 * section outputs all the character setting and positioning commands.
 * 
 */


/* Change log:
 *
 * Early 1985, (nmh) -- ported sun version to Apollo. 
 * A little later (nmh) -- changed to continue on in the event of missing
 *                      font files.
 * 30-Mar-85 (nmh) -- added -a option to specify a different PXL area
 * 30-Mar-85 (nmh) -- changed default PXL area to /pxl118
 * 31-Mar-85 (nmh) -- fixed bug in OpenFontFile() regarding more than MAXOPEN
 *                    PXL files -- changed to mark files as closed in font_entry.
 *  7-Apr-85 (nmh) -- made command line argument decoding case insensitive.
 *                    cleaned up handling of DVI file name argument.
 * 30-May-85 (nmh) -- new version; hacked to output PostScript commands
 *  6-Jun-85 (nmh) -- added relative positioning (20% smaller PostScript output)
 *                    add -m option to specify mag
 * 11-Jun-85 (nmh) -- fixed bug regarding char spacings in very long "words"
 * 12-Jun-85 (nmh) -- v1.02 - process DVI pages in reverse order
 * 13-Jun-85 (nmh) -- fixed bug re PXL files getting opened too often when no PreLoad
 * 14-Jun-85 (nmh) -- font dict created in PostScript only when 1st char of font downloaded
 *                    add -m0 -mh -m1 etc. to specify magsteps
 * 16-Aug-85 (nmh) -- added -c option t0 create output file in spool area (Apollo specific)
 *                    added -h option to copy header file to output
 *                    added -o option to pass options through to PostScript
 * 20-Aug-85 (nmh) -- v1.03
 * 24-Aug-85 (nmh) -- add -q option (for quiet operation).
 *                    changed -o option to check PostScript option
 *                    changed to output coordinates in TeX system (but
 *                    scaled to raster units) -- (0,0) at 1" in and down from
 *                      top left (better for use with different size paper).
 *                 -- v2.00
 * 25-Aug-85 (nmh) -- added dictionary enclosure to Tex.ps, and output
 *                      suitable prolog here.
 * 26-Aug-85 (nmh) -- changes to tex.ps to support Macintosh documents.
 * 14-Sep-85 (nmh) -- added keyword=value decoding to \special;
 * 15-Sep-85 (nmh) -- added -i file option.
 * 23-Sep-85 (saj) -- added font substitution for case when font is
 *                    unavailable at requested mag. (a frequent occurrence
 *                    with some macro packages like LaTeX)
 * 6-Jan-87 (cudcv) - major conversion to Xerox 2700.
 *			cope with PK files, and with colon separated paths
 *			for pixel area.
 */


/**********************************************************************/
/************************  Global Definitions  ************************/
/**********************************************************************/

/* This version purports to drive a Xerox 2700 */


typedef int BOOLEAN;
#define NEW(A) ((A *) malloc(sizeof(A)))
#define DEBUG   1			/* for massive printing of input */
					/* trace information; select by -d */
					/* option after filename: */
					/* dviview filename -d */
#ifdef DEBUG
int Debug = 0;
#endif
                          /* to enable statistics reporting via -s option */
#define STATS

#define BINARYOPEN fopen		/* byte-oriented host version */

#define ARITHRSHIFT 1                   /* define if ">>" operator is a */
                                        /* sign-propagating arithmetic  */
                                        /*   right shift */
#define USEGLOBALMAG 1			/* when defined, the dvi global */
   					/*   magnification is applied   */
      
/* We can leave USEGLOBALMAG undefined when we have a limited
   number of font magnifications (at 300dpi) available.  Otherwise, we
   will simply complain about missing PXL files
 */

/* #undef USEGLOBALMAG */

                        /* define for "optimal" relative postioning, rather
                           than absolute.  Relative can reduce size of postcript
                           output 20% (and reduce print time by almost as much */
#define USERELPOS 1 

#define  DVIFORMAT        2
#define  TRUE             1
#define  FALSE            0
#define  FIRSTPXLCHAR     0
#define  LASTPXLCHAR    127

#ifndef FONTAREA
#define  FONTAREA         "/usr/lib/tex/fonts"
#endif

#ifdef apollo
#ifndef SPOOLFILE
#define SPOOLFILE       "/local/spool/laser/tex."
#endif
#define MAXFLEN 28
#endif apollo

#ifdef apollo
#define  MAXOPEN         45  /* limit on number of open PXL files */
#else !apollo
#define  MAXOPEN         45	/* limit on number of open PXL files */
#endif

#define  NPXLCHARS      128
#define  PXLID         1001
#define	PK_POST	245
#define	PK_PRE	247
#define	PK_ID	89
#define  READ             4  /* for access() */
#define  RESOLUTION      300
#define  hconvRESOLUTION 300
#define  vconvRESOLUTION 300
#define  STACKSIZE      100
#define  STRSIZE        257
#define	MAXnPXLvec	10	/* number of components in PXLpath */
#define  NONEXISTANT     -1   /* offset for PXL files not found */
#define  NO_FILE        (FILE *)-1
#define	FF	'\014'
#define	ESC	'\033'
#define	DEL	'\177'
#define	MAXFONTID	9	/* number of assigned fonts on 2700 */
#define	MINCH	32		/* minimum char in 2700 font */
#define	MAXCH	254		/* maximum  "   "   "    "   */
#define	MAX_PAT	0x40000		/* maximum pattern data in one font */
#define	MAX_LDFONTS	14	/* maximum number of loaded fonts */
#define	MAX_FONT_DATA	150000	/* maximum number of font bytes */
#define	TOP	3500		/* top of page - 1" in pixels for A4 */
#define	LEFT	0		/* indent 1" from left margin */

/* may need to byte swap for xerox */
#ifdef	vax
#define	htoxs(x)	(x)
#else
#define	htoxs(x)	((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
#endif
#define	xtohs(x)	htoxs(x)

#ifdef apollo           /* define for enabling of -c option (create output file) */
#define CREOPT
#endif apollo

/**********************************************************************/
/***********************  external definitions  ***********************/
/**********************************************************************/

#include "commands.h"
#include <sys/param.h>
#include <sys/file.h>
#include <strings.h>
#include <signal.h>
#include <stdio.h>
#include "findfile.h"
#include "fontstr.h"
#include <ctype.h>
#include <errno.h>

extern int	errno;
int  access();
char *index();
char *malloc();
int  free();
char *rindex();
char *sprintf();
char *strcpy(); 
char *logname();

#define EQ(a,b) (strcmp(a,b)==0)

                        /* output a formatted string */
#define EMIT      (void) fprintf
                        /* output a simple string */
#define EMITS(s)  fputs(s,outfp)
                        /* output an escaped octal number */
#define EMITO(c)  PutOct(c)
                        /* output a decimal integer */
#define EMITN(n)  PutInt(n)
                        /* output a byte value in Hex */
#define EMITH(h)  (void) putc(*(digit+((h>>4)&0xF)),outfp),\
                  (void) putc(*(digit+(h&0xF)),outfp)
                        /* output a single character */
#define EMITC(c)  (void) putc(c,outfp)
                        /* output a scaled X dimension */
#define EMITX(x)  PutInt(PixRound(x,hconv))
                        /* output a scaled Y dimension */
#define EMITY(y)  PutInt(PixRound(y,vconv))

                                   /* formatted i/o was killing us, so build some tables */
char    *digit = "0123456789ABCDEF";

typedef union {
    int code;
    struct {
#ifndef	vax
	unsigned int spare		:24;
	unsigned int flag_dyn_f		:4;
	unsigned int flag_turn_on	:1;
	unsigned int flag_extended	:1;
	unsigned int flag_length_prefix	:2;
#else
	unsigned int flag_length_prefix	:2;
	unsigned int flag_extended	:1;
	unsigned int flag_turn_on	:1;
	unsigned int flag_dyn_f		:4;
	unsigned int spare		:24;
#endif
    } flag_split;
} FlagByte;
#define	dyn_f		flag_split.flag_dyn_f
#define	turn_on		flag_split.flag_turn_on
#define	extended	flag_split.flag_extended
#define	length_prefix	flag_split.flag_length_prefix

/**********************************************************************/
/*************************  Global Procedures  ************************/
/**********************************************************************/

/* Note: Global procedures are declared here in alphabetical order, with
   those which do not return values typed "void".  Their bodies occur in
   alphabetical order following the main() procedure.  The names are
   kept unique in the first 6 characters for portability. */

void	AbortRun();
float	ActualFactor();
void	AllDone();
FILE*	BINARYOPEN();
void    CopyFile();
void    DecodeArgs();
void    DoSpecial();
void    EmitChar();
void	Encode();
void	Fatal();
void	FindPostAmblePtr();
void	FlushXFont();
void	GetBytes();
void	GetFontDef();
char   *GetKeyStr();
int     GetKeyVal();
int     HasBeenRead();
int     IsSame();
void    lcase();
void	MoveDown();
void	MoveOver();
int     NoSignExtend();         /* see cautionary note in code, re arithmetic vs logical shifts */
void	OpenFontFile();
#ifdef CREOPT
FILE*   OpenOutput();
#endif CREOPT
int	PixRound();
void    PutInt();
int	ReadFontDef();
void	ReadPostAmble();
void	SetChar();
void	SetFntNum();
void    SetPosn();
void	SetRule();
/* void    SetString(); */
void	SetXFont();
int     SignExtend();           /* see cautionary note in code, re arithmetic vs logical shifts */
void	SkipFontDef();
FlagByte	SkipSpecials();
void	Warning();


/**********************************************************************/
/***********************  Font Data Structures  ***********************/
/**********************************************************************/


struct char_entry {		/* character entry */
   unsigned short width, height;/* width and height in pixels */
   short xOffset, yOffset;      /* x offset and y offset in pixels */
   struct {
       int isloaded;		/* == loadpass if in current load */
       union {
	   long fileOffset;
	   long *pixptr; } address;
       struct xfont *xfont;	/* 2700 font this character lives in */
       int glyph;		/* position in 2700 font */
       } where;
   int tfmw;			/* TFM width */
   int realw;			/* best the 2700 can do */
   int kern;			/* correction for +ve xOffset */
   FlagByte flag;		/* PK flag byte */
   };

struct font_entry {  /* font entry */
   int k, c, s, d, a, l;
   char n[STRSIZE];	/* FNT_DEF command parameters  */
   int font_space;	/* computed from FNT_DEF s parameter        */
   int font_mag;	/* computed from FNT_DEF s and d parameters */
   char name[STRSIZE];	/* full name of PXL file                    */
   FILE *font_file_id;  /* file identifier (NO_FILE if none)         */
   int type;			/* FONT_PK or FONT_PXL */
   long	offset;			/* offset in PK file */
   int magnification;	/* magnification read from PXL file         */
   int designsize;	/* design size read from PXL file           */
   struct char_entry ch[NPXLCHARS];/* character information         */
   struct font_entry *next;
   int ncdl;            /* # of different chars actually downloaded */
#ifdef STATS
   int nbpxl;           /* # of bytes of PXL data downloaded        */
   int ncts;            /* total # of characters typeset */
#endif
};
#define	FONT_PXL	0	/* this is a PXL font */
#define	FONT_PK		1	/* this is a PK font */

struct pixel_list
{
    FILE *pixel_file_id;        /* file identifier                          */
    struct font_entry *font_entry; /* font entry this refers to */
    int use_count;              /* count of "opens"                         */
    };

struct xfont {
    struct xfont *next;		/* list of fonts */
    char xname[STRSIZE];	/* 2700 font name */
    int glyph;			/* maximum glyph in this font so far */
    int usage;			/* usage since assignment */
};

/**********************************************************************/
/*************************  Global Variables  *************************/
/**********************************************************************/

int   FirstPage = -1000000;     /* first page to print (uses count0)   */
int   LastPage = 1000000;       /* last page to print                    */

char filename[STRSIZE];         /* DVI file name			   */

#ifdef	CREOPT
int   G_create = FALSE;         /* create an output file in spool area ?   */
#endif
int   G_errenc = FALSE;	        /* has an error been encountered?          */
char  G_Logname[STRSIZE];       /* name of log file, if created            */
int   G_logging = 0;            /* Are we logging warning messages?        */
int   G_logfile = FALSE;        /* Are these messages going to a log file? */
FILE *G_logfp;                  /* log file pointer (for errors)           */
char  G_progname[STRSIZE];      /* program name                            */
int   G_quiet = FALSE;          /* for quiet operation                     */
int   G_nowarn = FALSE;         /* don't print out warnings                */

int   hconv, vconv;		/* converts DVI units to pixels             */
int   den;			/* denominator specified in preamble        */
FILE *dvifp  = NULL;		/* DVI file pointer                         */
int   PreLoad = TRUE;	        /* preload the font descriptions?	     */
struct font_entry *prevfont=NULL;  /* font_entry pointer, previous character */
struct font_entry *fontptr;     /* font_entry pointer                       */
struct font_entry *hfontptr=NULL;/* font_entry pointer                      */
int   h;			/* current horizontal position              */
int   hh = -9999;		/* current h on device */
int   v;			/* current vertical position                */
int   vv = -9999;		/* current v on device */
int   mag;			/* magnification specified in preamble      */
int   ndone = 0;                /* number of pages converted */
int   nopen;			/* number of open PXL files                 */
int   num;			/* numerator specified in preamble          */
#ifdef CREOPT
char  outfname[256];            /* name of output file                      */
#endif CREOPT
FILE  *outfp = NULL;            /* output file                              */
struct font_entry *pfontptr = NULL; /* previous font_entry pointer          */
struct pixel_list pixel_files[MAXOPEN+1];
                                /* list of open PXL file identifiers        */
long  postambleptr;		/* Pointer to the postamble                 */
FILE *pxlfp;			/* PXL file pointer                         */
char *PXLpath;			/* PXL path name for search		    */
char *PXLvec[MAXnPXLvec];	/* " split into components at : */
int   nPXLvec;			/* number of components in " */
long  ppagep;	                /* previous page pointer		     */
char  rootname[STRSIZE];      /* DVI filename without extension */
#ifdef STATS
int   Stats = FALSE;          /* are we reporting stats ?                 */
int   Snbpxl = 0;             /* # of bytes of pixel data                 */
int   Sonbpx = 0;             /* "optimal" number of bytes of pixel data  */
int   Sndc = 0;               /* # of different characters typeset        */
int   Stnc = 0;               /* total # of chars typeset                 */
int   Snbpx0, Sndc0, Stnc0;   /* used for printing incremental changes per dvi page */
#endif

int usermag = 0;              /* user specified magnification */
int	loadpass;		/* in nth font load */
int	Complex = FALSE;	/* section too complex - print it */
int	Emitting = FALSE;	/* outputting typsetting instructions? */
int	SkipMode = FALSE;	/* in skip mode flag                     */
struct xfont *curxfont = NULL;	/* current font on 2700 */
struct xfont *headxfont = NULL;	/* list of 2700 fonts */
int	nxfonts = 0;		/* number of 2700 fonts so far */
long	total_font_data;	/* total bytes of font data */
struct xheader	xheader;	/* header for 2700 font */
char	qual_tab[256] = { 0 };	/* qualification table (unused) */
struct xchardesc lut_tab [256];	/* character lookup table in current font */
struct xchardesc space_glyph = { /* single pixel space */
    htoxs(2), 0, 0, 61, -5, 0
};
char	patt_tab[MAX_PAT];	/* patterns for 2700 characters */
int	next_free;		/* next free slot in patt_tab */
struct xfont *assigned_font[MAXFONTID];	/* fonts with assigned ID's */

/**********************************************************************/
/*******************************  main  *******************************/
/**********************************************************************/

main(argc, argv)
int argc;
char *argv[];

{
    struct stack_entry {  /* stack entry */
	int h, v, w, x, y, z;  /* what's on stack */
    };


    int command;	    /* current command			     */
    int count[10];          /* the 10 counters at begining of each page */
    long cpagep;	    /* current page pointer		     */
    long spagep, tpagep;	/* extent of this section */
    register int i;	    /* command parameter; loop index	     */
    int k;		    /* temporary parameter		     */
    char n[STRSIZE];	    /* command parameter		     */
    int PassNo = 0;         /* which pass over the DVI page are we on?  */
    int sp;		    /* stack pointer			     */
    struct stack_entry stack[STACKSIZE];   /* stack		     */
    char SpecialStr[STRSIZE]; /* "\special" strings                  */
    int val, val2;          /* temporarys to hold command information*/
    int w;		    /* current horizontal spacing	     */
    int x;		    /* current horizontal spacing	     */
    int y;		    /* current vertical spacing		     */
    int z;		    /* current vertical spacing		     */

    nopen = 0;
    (void) strcpy(G_progname, argv[0]);

    PXLpath = FONTAREA;       /* default font area */

    DecodeArgs( argc, argv );

    {				/* split PXLpath into vec */
	char *p;

	nPXLvec = 1;
	PXLvec[0] = PXLpath;

	while ((p = index(PXLpath, ':')) != 0 && nPXLvec < MAXnPXLvec) {
	    *p++ = '\0';
	    PXLvec[nPXLvec++] = p;
	}
    }

#ifdef apollo
    set_sbrk_size( 2048*1024 );
#endif

    if ((i = NoSignExtend(dvifp, 1)) != PRE)  {
	(void) fprintf(stderr,"\n");
	Fatal("%s: PRE doesn't occur first--are you sure this is a DVI file?\n\n",
	G_progname);
    }

    i = SignExtend(dvifp, 1);
    if (i != DVIFORMAT)  {
	(void) fprintf(stderr,"\n");
	Fatal("%s: DVI format = %d, can only process DVI format %d files\n\n",
	G_progname, i, DVIFORMAT);
    }

#ifdef CREOPT
    if( G_create )
        outfp = OpenOutput();
    else
#endif CREOPT
        outfp = stdout;

    /* it is important that these be the very first things output !!! */
    EMIT(outfp,"%c+X\n",ESC);
    /* set margins for A4 */
    EMIT(outfp,"%cm700,0,0,0,495\n",ESC);

    if (PreLoad) {
	ReadPostAmble(TRUE);
	(void) fseek(dvifp, (long) 14, 0);
    }
    else {
	num = NoSignExtend(dvifp, 4);
	den = NoSignExtend(dvifp, 4);
	mag = NoSignExtend(dvifp, 4);
#ifdef USEGLOBALMAG
	if( usermag > 0 && usermag != mag )
	    (void) fprintf(stderr, "DVI magnification of %d over-ridden by user mag of %d\n", mag, usermag );
#endif
	if( usermag > 0 ) mag = usermag;
#ifndef USEGLOBALMAG
	if( mag != 1000 )
	    (void) fprintf(stderr, "Magnification of %d ignored.\n", mag);
#endif
	hconv = DoConv(num, den, hconvRESOLUTION);
	vconv = DoConv(num, den, vconvRESOLUTION);
    }
    k = NoSignExtend(dvifp, 1);
    GetBytes(dvifp, n, k);

    PassNo = 0;
    spagep = ftell(dvifp);
    tpagep = 0;
    total_font_data = 0;
    nxfonts = 0;
    curxfont = 0;
    loadpass = 1;
    EMIT(outfp,"%c+F\n",ESC);
    
    while (TRUE)

	switch (command=NoSignExtend(dvifp, 1))  {

	case SET1:case SET2:case SET3:case SET4:
	    val = NoSignExtend(dvifp, command-SET1+1);
	    if (!SkipMode) SetChar(val, command, PassNo);
	    break;

	case SET_RULE:
	    val = NoSignExtend(dvifp, 4);
	    val2 = NoSignExtend(dvifp, 4);
            if (Emitting) SetRule(val, val2, 1);
	    break;

	case PUT1:case PUT2:case PUT3:case PUT4:
	    val = NoSignExtend(dvifp,command-PUT1+1);
	    if (!SkipMode) SetChar(val, command, PassNo);
	    break;

	case PUT_RULE:
	    val = NoSignExtend(dvifp, 4);
	    val2 = NoSignExtend(dvifp, 4);
            if (Emitting) SetRule(val, val2, 0);
	    break;

	case NOP:
	    break;

	case BOP:
	    cpagep = ftell(dvifp) - 1;
	    for (i=0; i<=9; i++)
		count[i] = NoSignExtend(dvifp, 4);
	    ppagep = NoSignExtend(dvifp, 4);

	    h = v = w = x = y = z = 0;
            hh = vv = -9999;
	    sp = 0;
	    fontptr = NULL;
            prevfont = NULL;

            if( count[0] < FirstPage || count[0] > LastPage )
                SkipMode = TRUE;
            else
                SkipMode = FALSE;

            if( !SkipMode ) {
                if( PassNo == 0) {
#ifdef STATS
		    if( Stats ) {
			Sndc0 = Sndc;
			Stnc0 = Stnc;
			Snbpx0 = Snbpxl;
		    }
#endif
		    if( !G_quiet ) {
			(void) fprintf(stderr, "[%d", count[0] );
			(void) fflush(stderr);
		    }
		}
                else if (cpagep == tpagep) { /* end of section */
		    PassNo = 0;
		    spagep = cpagep; /* start of next section */
		    for (i = 0; i < MAXFONTID; i++)
			assigned_font[i] = NULL;
		    total_font_data = 0;
		    nxfonts = 0;
		    curxfont = 0;
		    loadpass++;
		    EMIT(outfp,"%c+F\n",ESC);
		}
	    }

            Emitting = (PassNo != 0) && !SkipMode;

	    break;

	case EOP:
	    if (Complex) {	/* we've had to abandon font processing */
		if (spagep == cpagep) {	/* start of section is this page ! */
		    Fatal("Page too complex");
		}
		Complex = 0;
		tpagep = cpagep;
		(void) fseek(dvifp, spagep, 0);
		SkipMode = 0;
		PassNo = 1;
		FlushXFont(curxfont);
		curxfont = NULL;
		EMIT(outfp,"%c+P\n", ESC);
		/* set up font now, in case first command isn't a char */
		SetXFont(headxfont);
	    }
            if( !SkipMode ) {
                if (PassNo == 1) { /* end of page processing in second pass */
                    EMIT(outfp,"%c",FF);
		    hh = vv = -9999;
#ifdef STATS
                    if( Stats )
                        (void) fprintf(stderr," - %d total ch,  %d diff ch,  %d pxl bytes]\n",
				       Stnc-Stnc0, Sndc-Sndc0, Snbpxl-Snbpx0);   
                    else
#endif
                        if( !G_quiet ) {
			    (void) fprintf(stderr,"] ");
			    if( (++ndone % 10) == 0 )
				(void) fprintf(stderr,"\n");
			    (void) fflush(stderr);
			}
		}
	    }
	    break;

	case PUSH:
	    if (sp >= STACKSIZE)
		Fatal("stack overflow");
	    stack[sp].h = h;
	    stack[sp].v = v;
	    stack[sp].w = w;
	    stack[sp].x = x;
	    stack[sp].y = y;
	    stack[sp].z = z;
	    sp++;
	    break;

	case POP:
	    --sp;
	    if (sp < 0)
		Fatal("stack underflow");
	    h = stack[sp].h;
	    v = stack[sp].v;
	    w = stack[sp].w;
	    x = stack[sp].x;
	    y = stack[sp].y;
	    z = stack[sp].z;
	    break;

	case RIGHT1:case RIGHT2:case RIGHT3:case RIGHT4:
            val = SignExtend(dvifp,command-RIGHT1+1);
	    if (Emitting) MoveOver(val);
	    break;

	case W0:
            if (Emitting) MoveOver(w);
	    break;

	case W1:case W2:case W3:case W4:
	    w = SignExtend(dvifp,command-W1+1);
            if (Emitting) MoveOver(w);
	    break;

	case X0:
            if (Emitting) MoveOver(x);
	    break;

	case X1:case X2:case X3:case X4:
	    x = SignExtend(dvifp,command-X1+1);
	    if (Emitting) MoveOver(x);
	    break;

	case DOWN1:case DOWN2:case DOWN3:case DOWN4:
            val = SignExtend(dvifp,command-DOWN1+1);
	    if (Emitting) MoveDown(val);
	    break;

	case Y0:
            if (Emitting) MoveDown(y);
	    break;

	case Y1:case Y2:case Y3:case Y4:
	    y = SignExtend(dvifp,command-Y1+1);
            if (Emitting) MoveDown(y);
	    break;

	case Z0:
            if (Emitting) MoveDown(z);
	    break;

	case Z1:case Z2:case Z3:case Z4:
	    z = SignExtend(dvifp,command-Z1+1);
	    if (Emitting) MoveDown(z);
	    break;

	case FNT1:case FNT2:case FNT3:case FNT4:
            if (!SkipMode) {
                SetFntNum(NoSignExtend(dvifp,command-FNT1+1));
                }
	    break;

	case XXX1:case XXX2:case XXX3:case XXX4:
	    k = NoSignExtend(dvifp,command-XXX1+1);
            GetBytes(dvifp, SpecialStr, k);
            if(Emitting) DoSpecial(SpecialStr, k);
	    break;

	case FNT_DEF1:case FNT_DEF2:case FNT_DEF3:case FNT_DEF4:
            k = NoSignExtend(dvifp, command-FNT_DEF1+1);
	    if (PreLoad || HasBeenRead(k) )
	    {
		SkipFontDef (k);
	    }
	    else
	    {
		ReadFontDef (k);
	    }
	    break;

	case PRE:
	    Fatal("PRE occurs within file");
	    break;

	case POST:
	    if (PassNo == 0) {
		tpagep = 0;
		(void) fseek(dvifp, spagep, 0);
		PassNo = 1;
		FlushXFont(curxfont);
		curxfont = NULL;
		EMIT(outfp,"%c+P\n", ESC);
		/* set up font now, in case first command isn't a char */
		SetXFont(headxfont);
	    }
	    else
		AllDone();
	    break;

	case POST_POST:
 	    Fatal("POST_POST with no preceding POST");
	    break;

	default:
	    if (command >= FONT_00 && command <= FONT_63)
		{if (!SkipMode)
                     SetFntNum(command - FONT_00);}
	    else if (command >= SETC_000 && command <= SETC_127)
		{if (!SkipMode) SetChar(command, command, PassNo);}
	    else
		Fatal("%d is an undefined command", command);
	    break;

	}

}

/*-->AbortRun*/
/**********************************************************************/
/***************************  AbortRun  *******************************/
/**********************************************************************/

void
AbortRun(code)
int code;
{
    exit(code);
}


/*-->ActualFactor*/
/**********************************************************************/
/**************************  ActualFactor  ****************************/
/**********************************************************************/

float		/* compute the actual size factor given the approximation */
ActualFactor(unmodsize)
int unmodsize;  /* actually factor * 1000 */
{
    float realsize;	/* the actual magnification factor */

    realsize = (float)unmodsize / 1000.0;
    /* a real hack to correct for rounding in some cases--rkf */
    if(unmodsize==1095) realsize = 1.095445;	/*stephalf*/
    else if(unmodsize==1315) realsize=1.314534;	/*stepihalf*/
    else if(unmodsize==1577) realsize=1.577441;	/*stepiihalf*/
    else if(unmodsize==1893) realsize=1.892929;	/*stepiiihalf*/
    else if(unmodsize==2074) realsize=2.0736;	/*stepiv*/
    else if(unmodsize==2488) realsize=2.48832;  /*stepv*/
    else if(unmodsize==2986) realsize=2.985984;	/*stepiv*/
    /* the remaining magnification steps are represented with sufficient
	   accuracy already */
    return(realsize);
}


/*-->AllDone*/
/**********************************************************************/
/****************************** AllDone  ******************************/
/**********************************************************************/

void
AllDone()
{
    int t;
    struct font_entry *p;

    EMIT(outfp,"%c+X\n",ESC);
    /* just in case the exit code wants to open a file (profiling) ... */
    (void) fclose(dvifp);
    if( !G_quiet )
	(void) fprintf(stderr,"\n");

#ifdef CREOPT
    if( G_create ) {
        (void) fclose(outfp);
        if( !G_quiet )
	    (void) fprintf(stderr, "Output written on \"%s\"\n", outfname );
        }
#endif CREOPT

    if (G_errenc && G_logging == 1 && G_logfile)  {
	(void) fseek(G_logfp, (long)0, 0);
	while ((t=getc(G_logfp)) != EOF)
	    (void) putchar(t);
    }
    if (G_logging == 1 && G_logfile)
	(void) printf("Log file created\n");

#ifdef STATS
    if (Stats) {
        (void) fprintf( stderr, "Total chars   diff chars   pxl bytes\n" );
        (void) fprintf( stderr, "      #   %%        #   %%       #   %%\n" );
        (void) fprintf( stderr, "------- ---   ------ ---   ----- ---\n" );
        for( p=hfontptr; p!=NULL; p=p->next ) {
                (void) fprintf( stderr, "%7d%4d", p->ncts, (100*p->ncts + Stnc/2)/Stnc );
                (void) fprintf( stderr, "%9d%4d", p->ncdl, (100*p->ncdl + Sndc/2)/Sndc );
                (void) fprintf( stderr, "%8d%4d", p->nbpxl, (100*p->nbpxl + Snbpxl/2)/Snbpxl );
                }
        (void) fprintf(stderr, "\nTotal number of characters typeset: %d\n", Stnc);
        (void) fprintf(stderr, "Number of different characters downloaded: %d\n", Sndc);
        (void) fprintf(stderr, "Number of bytes of pxl data downloaded: %d\n", Snbpxl);
        (void) fprintf(stderr, "Optimal # of bytes of pxl data: %d\n", Sonbpx);
        }
#endif

    AbortRun(G_errenc);
}


/*-->CopyFile*/   /* copy a file straight through to output */
/*********************************************************************/
/***************************** CopyFile ******************************/
/*********************************************************************/

#ifdef	notdef
void
CopyFile( str )
char    *str;
{
        FILE    *spfp;
        int	t;

        if( (spfp=fopen(str,"r")) == NULL ) {
                (void) fprintf(stderr,"Unable to open file %s\n", str );
                return;
                }
        if( !G_quiet )
	    (void) fprintf(stderr," [%s", str);
        while( (t = getc(spfp)) != EOF ) {
                EMITC(t);
                }              
        (void) fclose(spfp);
        if( !G_quiet )
	    (void) fprintf(stderr,"]");
}
#endif

/*-->DecodeArgs*/
/*********************************************************************/
/***************************** DecodeArgs ****************************/
/*********************************************************************/

void
DecodeArgs( argc, argv )
int argc;
char *argv[];
{
    int argind;             /* argument index for flags              */
    char curarea[STRSIZE];  /* current file area		     */
    char curname[STRSIZE];  /* current file name		     */
    char *tcp, *tcp1;	    /* temporary character pointers	     */

    argind = 1;
    while (argind < argc) {
	tcp = argv[argind];
        if (*tcp == '-')
	    switch(isupper(*++tcp) ? (*tcp-'A')+'a' : *tcp) {

                case 'a':       /* a selects different pxl font area */
                    PXLpath = argv[++argind];
                    break;
#ifdef CREOPT
                case 'c':       /* create an output file in spool area */
                    G_create = TRUE;
                    break;
#endif CREOPT
#ifdef DEBUG
		case 'd':	/* d selects Debug output */
		    Debug = TRUE;
		    break;
#endif
                case 'f':       /* next arg is starting pagenumber */
                    if( ++argind >= argc || sscanf(argv[argind], "%d", &FirstPage) != 1 )
                        Fatal("Argument is not a valid integer\n", 0);
                    break;

		case 'l':	/* l prohibits logging of errors */
		    G_logging = -1;
		    break;
#ifdef USEGLOBALMAG
                case 'm':       /* specify magnification to use */
                    switch( tolower(*++tcp) ) {

                    case '\0':       /* next arg is a magnification to use */
                        if( ++argind >= argc || sscanf(argv[argind], "%d", &usermag) != 1 )
                            Fatal("Argument is not a valid integer\n", 0);
                        break; 
                    case '0': usermag = 1000; break;
                    case 'h': usermag = 1095; break;
                    case '1': usermag = 1200; break;
                    case '2': usermag = 1440; break;
                    case '3': usermag = 1728; break;
                    case '4': usermag = 2074; break;
                    case '5': usermag = 2488; break;
                    default: Fatal("%c is a bad mag step\n", *tcp);
                    }
                    break;
#endif

		case 'p':	/* p prohibits pre-font loading */
		    PreLoad = 0;
		    break;

                case 'q':       /* quiet operation */
                    G_quiet = TRUE;
                    break;
#ifdef STATS                   
                case 's':       /* print some statistics */
                    Stats = TRUE;
                    break;
#endif
                case 't':       /* next arg is ending pagenumber */
                    if( ++argind >= argc || sscanf(argv[argind], "%d", &LastPage) != 1 )
                        Fatal("Argument is not a valid integer\n", 0);
                    break;

                case 'w':       /* don't print out warnings */
                    G_nowarn = TRUE;
                    break;

		default:
		    (void) printf("%c is not a legal flag\n", *tcp);
		}

        else {

            tcp = rindex(argv[argind], '/');    /* split into directory + file name */
            if (tcp == NULL)  {
        	curarea[0] = '\0';
        	tcp = argv[argind];
                }
            else  {
        	(void) strcpy(curarea, argv[argind]);
                curarea[tcp-argv[argind]+1] = '\0';
        	tcp += 1;
                }
        
            (void) strcpy(curname, tcp);
            tcp1 = rindex(tcp, '.');   /* split into file name + extension */
            if (tcp1 == NULL) {
                (void) strcpy(rootname, curname);
                (void) strcat(curname, ".dvi");
                }
            else {
                *tcp1 = '\0';
                (void) strcpy(rootname, curname);
                *tcp1 = '.';
                }
        
            (void) strcpy(filename, curarea);
            (void) strcat(filename, curname);
        
            if ((dvifp=BINARYOPEN(filename,"r")) == NULL)  {
        	(void) fprintf(stderr,"\n");
        	(void) fprintf(stderr,"%s: can't find DVI file \"%s\"\n\n", G_progname, filename);
        	AbortRun(1);
                }
        
            (void) strcpy(G_Logname, curname);
            (void) strcat(G_Logname, ".log");
	    }
	argind++;
        }

    if (dvifp == NULL)  {
	(void) fprintf(stderr, 
                "\nusage: %s [-a area] [-c] [-h] [-o option] [-p] [-s] [-r] [-f n] [-t n] [-m{0|h|1|2|3|4|  mag] [-a fontarea] dvifile\n\n", 
                G_progname);
	AbortRun(1);
        }
}


/*-->DoConv*/
/*********************************************************************/
/********************************  DoConv  ***************************/
/*********************************************************************/

int DoConv(num, den, convResolution)
{
    register float conv;
    conv = ((float)num/(float)den) * 
#ifdef USEGLOBALMAG
/*	ActualFactor(mag) * why was this in as Actual Factor?  jls */
	((float) mag/1000.0) *
#endif
	((float)convResolution/254000.0);
    return((int) (1.0 / conv + 0.5));
}


/*-->DoSpecial*/
/*********************************************************************/
/*****************************  DoSpecial  ***************************/
/*********************************************************************/
#ifdef	notdef
typedef enum {None, String, Integer, Number, Dimension} ValTyp;

typedef struct {
        char    *Key;           /* the keyword string */
        char    *Val;           /* the value string */
        ValTyp  vt;             /* the value type */
        union {                 /* the decoded value */
            int  i;
            float n;
            } v;
        } KeyWord;

typedef struct {
        char    *Entry;
        ValTyp  Type;
        } KeyDesc;

#define PSFILE 0
KeyDesc KeyTab[] = {{"psfile", String},
                    {"hsize", Dimension},
                    {"vsize", Dimension},
                    {"hoffset", Dimension},
                    {"voffset", Dimension},
                    {"hscale", Number},
                    {"vscale", Number}};

#define NKEYS (sizeof(KeyTab)/sizeof(KeyTab[0]))
#endif
void
DoSpecial( str, n )          /* interpret a \special command, made up of keyword=value pairs */
char    *str;
int n;
{
#ifdef	notdef			/* don't do anything for now */
        char spbuf[STRSIZE]; 
        char *sf = NULL;
        KeyWord k;
        int i;

        str[n] = '\0';
        spbuf[0] = '\0';

        SetPosn(h, v, 0);
        EMITS("@beginspecial\n");

        while( (str=GetKeyStr(str,&k)) != NULL ) {      /* get all keyword-value pairs */
                              /* for compatibility, single words are taken as file names */
                if( k.vt == None && access(k.Key,0) == 0) {
                        if( sf ) Warning("  More than one \\special file name given. %s ignored", sf );
                        (void) strcpy(spbuf, k.Key);
                        sf = spbuf;
                        }
                else if( GetKeyVal( &k, KeyTab, NKEYS, &i ) && i != -1 ) {
                        if( i == PSFILE ) {
                                if( sf ) Warning("  More than one \\special file name given. %s ignored", sf );
                                (void) strcpy(spbuf, k.Val);
                                sf = spbuf;
                                }
                        else            /* the keywords are simply output as PS procedure calls */
                                EMIT(outfp, "%f @%s\n", k.v.n, KeyTab[i].Entry);
                        }
                else Warning("  Invalid keyword or value in \\special - \"%s\" ignored", k.Key );
                }

        EMITS("@setspecial\n");

        if( sf )
                CopyFile( sf );
        else
                Warning("  No special file name provided.");

        EMITS("@endspecial\n");
#endif	notdef
}


/*-->EmitChar*/
/**********************************************************************/
/****************************  EmitChar  ******************************/
/**********************************************************************/
/* The 2700 doesn't seem to have much idea about x-offsets, so for negative
 * offsets pad out the bit map with space, and for positive offsets supply
 * the offset to SetPosn as an extra parameter.
 */


void
EmitChar(ce)			/* output a character bitmap */
    struct char_entry *ce;
{
    int nbpl, nwpl;
    int xbytes_per_row;
    int bytes_needed;
    unsigned char *pattern;
    register int i, j, bit_n;
    register char *sl;
    struct xchardesc *cur_lut;

    /* check there's room in the pattern table */
    xbytes_per_row = (ce->height + 7) / 8;
    if (xbytes_per_row < 2) xbytes_per_row = 2;
    bytes_needed = xbytes_per_row * ce->width;
    if (ce->xOffset < 0)
	bytes_needed += - xbytes_per_row * ce->xOffset;
    if (bytes_needed + next_free > MAX_PAT) {
	FlushXFont(curxfont);
	curxfont = 0;
    }

    if (curxfont == 0) {
	if (nxfonts > MAX_LDFONTS) {
	    /* too many fonts in this section */
	    Complex++;
	    SkipMode++;
	    Emitting = 0;
	    return;
	}
	curxfont = (struct xfont *) malloc(sizeof(struct xfont));
	/* 
	 * initialize first characters in every font to be spaces
	 * to use for quick, small moves.  It seems character 32 is not
	 * usable, and ought to be a standard space.
	 */
	cur_lut = &lut_tab[MINCH];
	*cur_lut = space_glyph;
	cur_lut->width = 18;
	*++cur_lut = space_glyph;
	cur_lut->width = 1;
	*++cur_lut = space_glyph;
	cur_lut->width = 2;
	bzero(&patt_tab[0], 2);
	next_free = 2;
	curxfont->glyph = MINCH+3;
	/* make sure the 2700 name is unique, in case this is a clever version
	 * that only loads fonts it doesn't already have (3700?) */
	(void) sprintf(curxfont->xname, "dvi%u-%d-%d",
		       getpid(), loadpass, nxfonts++);
	curxfont->next = headxfont;
	headxfont = curxfont;
	/* header, qual table, lut entry for 3 spaces, 2 byte pattern */
	total_font_data += sizeof(struct xheader)
	    + 256*sizeof(qual_tab[0])
	    + 3*sizeof(lut_tab[0])
	    + 2;
    }

    total_font_data += sizeof(lut_tab[0]) + bytes_needed;
    if (total_font_data > MAX_FONT_DATA) {
	/* too much font data in this section */
	Complex++;
	SkipMode++;
	Emitting = 0;
	return;
    }

    /* set up lookup table entry - loc will be adjusted later */
    cur_lut = &lut_tab[curxfont->glyph];
    cur_lut->lo_loc = htoxs(next_free & 0xffff);
    cur_lut->hi_loc = next_free >> 16;
    /* don't use PixRound for this - we'd rather adjust to the right */
    cur_lut->width = ce->tfmw / hconv;
    cur_lut->orgy = (ce->yOffset - ((int)ce->height) - 1) / 2;
    cur_lut->blocking = 63 - xbytes_per_row;
    cur_lut->nbyte = htoxs(bytes_needed);

    pattern = (unsigned char *) &patt_tab[next_free];
    bzero((char *)pattern, bytes_needed);
    /* for negative xOffset, add white space to pattern */
    if (ce->xOffset < 0)
	pattern += - xbytes_per_row * ce->xOffset;
#ifdef	notdef
    if (ce->xOffset > 0) fprintf(stderr, "xOffset: %d\n", ce->xOffset);
#endif
    /* for positive xOffset, adjust position before setting position, and
     * compensate by increasing width of character */
    if (ce->xOffset > 0) {
	ce->kern = ce->xOffset;
	cur_lut->width += ce->xOffset;
    }
    ce->realw = cur_lut->width * hconv;
#ifdef	notdef
    if (ce->xOffset > 0) {
	fprintf(stderr, "Incoming map: \n");
	printpixelmap((char *)ce->where.address.pixptr, nwpl*4, ce->height);
    }
#endif
    /* convert raster from PK or PXL to 2700 (rows and columns reversed) */
    /* this is all a bit cryptic I'm afraid */
    if (fontptr->type == FONT_PXL) {
	/* this is a PXL font, just reverse rows and columns */
	nbpl = (ce->width + 7) >> 3;
	nwpl = (ce->width + 31) >> 5;
	for (i = 0; i < ce->height; i++) {
	    sl = (char *)(ce->where.address.pixptr + (ce->height-1-i)*nwpl);
	    for (j = 0; j < nbpl; j++, sl++) {
		for (bit_n = 7; bit_n >= 0; bit_n--) {
		    if ((*sl >> bit_n) & 1)
			pattern[xbytes_per_row * ((j << 3) + (7 - bit_n))
				+ (i >> 3)] |= 1 << (7 - (i & 07));
		}
	    }
	}
    }
    else {			/* PK font */
	/* this code is basically from PKTOPXL, modified to swap rows & cols */
	if (ce->flag.dyn_f == 14) { /* bitwise raster image */
	    bit_n = 0;
	    for (i = ce->height-1; i >= 0; i--) {
		for (j = 0; j < ce->width; j++) {
		    if (((char *)ce->where.address.pixptr)[bit_n >> 3] &
			(1 << (7 - (bit_n & 07))))
			pattern[xbytes_per_row * j + (i >> 3)] |=
			    1 << (7 - (i & 07));
		    bit_n++;
		}
	    }
	}
	else {			/* compressed raster */
	    unsigned char *pp;
	    int nybble = 0, repeat = 0;
	    int turning_on = ce->flag.turn_on;
	    int row, count;
	    bit_n = 0;

	    pp = (unsigned char *)ce->where.address.pixptr;
	    for (row = ce->height - 1; row >= 0; ) {
		count = PKPackedNum(&pp, &nybble, ce->flag.dyn_f, &repeat);
#ifdef	notdef
		fprintf(stderr,
			"(row %d) turn_on = %d, count = %d, repeat = %d\n",
			row, turning_on, count, repeat);
#endif
		while (count > 0) {
		    while (bit_n < ce->width && count > 0) {
			if (turning_on)
			    pattern[xbytes_per_row * bit_n + (row >> 3)] |=
				1 << (7 - (row & 07));
			count--;
			bit_n++;
		    }
		    if (bit_n == ce->width) {
			for (bit_n = 0; bit_n < ce->width; bit_n++) {
			    if (pattern[xbytes_per_row * bit_n + (row >> 3)] &
				(1 << (7 - (row & 07)))) {
				    for (i = 1; i <= repeat; i++) {
					pattern[xbytes_per_row * bit_n +
						((row-i) >> 3)] |=
						    1 << (7 - ((row-i) & 07));
				    }
				}
			}
			row -= repeat + 1;
			bit_n = 0;
			repeat = 0;
		    }
		}
		turning_on = !turning_on;
	    }
	}
    }
#ifdef	notdef
    if (ce->xOffset > 0) {
	fprintf(stderr, "Outgoing map: \n");
	printpixelmap(&patt_tab[next_free], xbytes_per_row,
		      bytes_needed/xbytes_per_row);
    }
#endif
    next_free += (bytes_needed + 1) & ~1; /* start on 16-bit boundary */

    ce->where.xfont = curxfont;
    ce->where.glyph = curxfont->glyph;

    if (++curxfont->glyph == DEL) { /* DEL ignored by printer */
	curxfont->glyph++;
    }
    if (curxfont->glyph > MAXCH) {
	FlushXFont(curxfont);
	curxfont = 0;
    }
    
#ifdef STATS
    Snbpxl += nbpl*ce->height;
    fontptr->nbpxl += nbpl*ce->height;
    Sonbpx += (ce->width*ce->height + 7) >> 3;
    Sndc += 1;
#endif
}

#ifdef	notdef
printpixelmap(map, row, lines)
    unsigned char *map;
    int row, lines;
{
    register int i, j, k;
    int c;
    
    for (i = 0; i < lines; i++) {
	for (j = 0; j < row; j++) {
	    c = map[i * row + j];
	    for (k = 7; k >= 0; k--) {
		if (c & (1 << k))
		    putc('*', stderr);
		else
		    putc(' ', stderr);
	    }
	}
	putc('\n', stderr);
    }
}
#endif

static long encode_buf = 0;
static int encode_count = 0;

void
FlushXFont(xfont)
    struct xfont *xfont;
{
    long patt_offset, remainder, patt_length, font_length, new_loc;
    int trail_length, i, cid;

    if (xfont == 0)
	return;
    /* first construct header */
    xheader.magic = 0xaaaa;
    xheader.rev = 1;
    xheader.flags = PORTRAIT|PROP_SP;
    /* don't want font name null terminated */
    for (i = 0; i < 20; i++)
	xheader.fname[i] = ' ';
    (void) strncpy(xheader.fname, xfont->xname, strlen(xfont->xname));
    xheader.lowchar = MINCH;
    xheader.highchar = xfont->glyph - 1;
    /* offset into font of pattern table */
    patt_offset = sizeof(struct xheader) + 256 +
	sizeof(struct xchardesc) * (xfont->glyph - MINCH);
    /* this is a rather empirical calculation of how many trailing 0x55
     * bytes are needed, found by looking through several fonts */
    remainder = (next_free + patt_offset) % 6;
    trail_length = 3*((remainder + 3)/4) + 9 - remainder;
    for (i = 0; i < trail_length; i++)
	patt_tab[next_free++] = 0x55;
    patt_length = next_free;
    font_length = patt_length + patt_offset;
    xheader.lo_length = htoxs(font_length & 0xffff);
    xheader.hi_length = font_length >> 16;
    if (xheader.hi_length)
	xheader.flags |= LONG_FONT;
    /* initialize encoding routine, and output header and (unused) qual tab */
    encode_buf = 0;
    encode_count = 0;
    Encode((unsigned char *)&xheader, sizeof(struct xheader));
    Encode((unsigned char *)qual_tab, 256*sizeof(qual_tab[0]));
    /* update offsets into font */
    for (cid = xheader.lowchar; cid <= xheader.highchar; cid++) {
	new_loc = xtohs(lut_tab[cid].lo_loc) +
	    (lut_tab[cid].hi_loc << 16) + patt_offset;
	lut_tab[cid].lo_loc = htoxs(new_loc & 0xffff);
	lut_tab[cid].hi_loc = new_loc >> 16;
    }
    /* output lookup table and pattern area */
    Encode((unsigned char *)&lut_tab[xheader.lowchar],
	   sizeof(struct xchardesc)*(xheader.highchar-xheader.lowchar+1));
    Encode((unsigned char *)patt_tab, patt_length);
    /* flush encode_buf */
    new_loc = 0;
    Encode((unsigned char *)&new_loc, 2);
}

/*
 * encode font data for the 2700.  Each 6 bits of font data is converted into
 * an ASCII character from '?' to '~'.  Three bytes of input form 4 bytes of
 * output.
 */
void
Encode(p, len)
    register unsigned char *p;
    int len;
{
    register int i;

    while (1) {
	for ( ; len > 0 && encode_count < 3; len--, encode_count++)
	    encode_buf = (encode_buf << 8) + *p++;
	if (len <= 0)
	    return;
	for (i = 18; i >= 0; i -= 6) {
	    EMITC(((encode_buf >> i) & 077) + '?');
	}
	encode_count = 0;
	encode_buf = 0;
    }
}

int
PKPackedNum(p, b, dynf, repeat_count)
    unsigned char **p;
    int *b;
    int dynf;
    int *repeat_count;
{
    int i, j;
    
    i = GetNyb(p, b);
    if (i == 0) {
	do {
	    i++;
	} while ((j = GetNyb(p, b)) == 0);
	for ( ; i > 0; i--)
	    j = (j << 4) + GetNyb(p, b);
	return (j - 15 + (13 - dynf)*16 + dynf);
    }
    else if (i <= dynf)
	return i;
    else if (i < 14)
	return (i - dynf - 1)*16 + GetNyb(p, b) + dynf + 1;
    else {
	*repeat_count = (i == 14) ? PKPackedNum(p, b, dynf, repeat_count) : 1;
	return PKPackedNum(p, b, dynf, repeat_count);
    }
}

int
GetNyb(p, b)
    unsigned char **p;
    int *b;
{
    if (*b) {
	*b = 0;
	return (*(*p)++ & 0xf);
    }
    else {
	*b = 1;
	return (**p >> 4);
    }
}

/*-->Fatal*/
/**********************************************************************/
/******************************  Fatal  *******************************/
/**********************************************************************/
/*VARARGS1*/

void
Fatal(fmt, a, b, c)/* issue a fatal error message */
char *fmt;	/* format */
char *a, *b, *c;	/* arguments */

{
    if (G_logging == 1 && G_logfile)
    {
	(void) fprintf(G_logfp, "%s: FATAL--", G_progname);
	(void) fprintf(G_logfp, fmt, a, b, c);
	(void) fprintf(G_logfp, "\n");
    }

    (void) fprintf(stderr,"\n");
    (void) fprintf(stderr, "%s: FATAL--", G_progname);
    (void) fprintf(stderr, fmt, a, b, c);
    (void) fprintf(stderr, "\n\n");
    if (G_logging == 1)
	(void) printf("Log file created\n");
#ifdef CREOPT
    if (G_create && outfp != NULL) {
        (void) fclose(outfp);
        unlink(outfname);
        }
#endif CREOPT
    AbortRun(1);
}


/*-->FindPostAmblePtr*/
/**********************************************************************/
/************************  FindPostAmblePtr  **************************/
/**********************************************************************/

void
FindPostAmblePtr(postambleptr)
long	*postambleptr;

/* this routine will move to the end of the file and find the start
    of the postamble */

{
    int     i;

    (void) fseek (dvifp, (long) 0, 2);   /* goto end of file */
    *postambleptr = ftell (dvifp) - 4;
    (void) fseek (dvifp, *postambleptr, 0);

    while (TRUE) {
	(void) fseek (dvifp, --(*postambleptr), 0);
	if (((i = NoSignExtend(dvifp, 1)) != 223) &&
	    (i != DVIFORMAT))
	    Fatal ("Bad end of DVI file");
	if (i == DVIFORMAT)
	    break;
    }
    (void) fseek (dvifp, (long) ((*postambleptr) - 4), 0);
    (*postambleptr) = NoSignExtend(dvifp, 4);
    (void) fseek (dvifp, *postambleptr, 0);
}


/*-->GetBytes*/
/**********************************************************************/
/*****************************  GetBytes  *****************************/
/**********************************************************************/

void
GetBytes(fp, cp, n)	/* get n bytes from file fp */
register FILE *fp;	/* file pointer	 */
register char *cp;	/* character pointer */
register int n;		/* number of bytes  */

{
    while (n--)
	*cp++ = NoSignExtend(fp, 1);
}


/*-->GetFontDef*/
/**********************************************************************/
/**************************** GetFontDef  *****************************/
/**********************************************************************/

void
GetFontDef()

/***********************************************************************
   Read the font  definitions as they  are in the  postamble of the  DVI
   file.
***********************************************************************/

{
    unsigned char   byte;

    while (((byte = NoSignExtend(dvifp, 1)) >= FNT_DEF1) &&
	(byte <= FNT_DEF4)) {
	switch (byte) {
	case FNT_DEF1:
	    ReadFontDef (NoSignExtend(dvifp, 1));
	    break;
	case FNT_DEF2:
	    ReadFontDef (NoSignExtend(dvifp, 2));
	    break;
	case FNT_DEF3:
	    ReadFontDef (NoSignExtend(dvifp, 3));
	    break;
	case FNT_DEF4:
	    ReadFontDef (NoSignExtend(dvifp, 4));
	    break;
	default:
	    Fatal ("Bad byte value in font defs");
	    break;
	}
    }
    if (byte != POST_POST)
	Fatal ("POST_POST missing after fontdefs");
}


/*-->GetKeyStr*/
/**********************************************************************/
/*****************************  GetKeyStr  ****************************/
/**********************************************************************/

        /* extract first keyword-value pair from string (value part may be null)
         * return pointer to remainder of string
         * return NULL if none found
         */
#ifdef	notdef
char    KeyStr[STRSIZE];
char    ValStr[STRSIZE];

char *GetKeyStr( str, kw )
char    *str;
KeyWord *kw;
{
        char *s, *k, *v, t;

        if( !str ) return( NULL );

        for( s=str; *s == ' '; s++ ) ;                  /* skip over blanks */
        if( *s == '\0' ) return( NULL );

        for( k=KeyStr;                          /* extract keyword portion */
             *s != ' ' && *s != '\0' && *s != '='; 
             *k++ = *s++ ) ;
        *k = '\0';
        kw->Key = KeyStr;
        kw->Val = v = NULL;
        kw->vt = None;

        for( ; *s == ' '; s++ ) ;                       /* skip over blanks */
        if( *s != '=' )                         /* look for "=" */
                return( s );

        for( s++ ; *s == ' '; s++ ) ;                   /* skip over blanks */
        if( *s == '\'' || *s == '\"' )          /* get string delimiter */
                t = *s++;
        else
                t = ' ';
        for( v=ValStr;                          /* copy value portion up to delim */
             *s != t && *s != '\0';
             *v++ = *s++ ) ;
        if( t != ' ' && *s == t ) s++;
        *v = '\0';
        kw->Val = ValStr;
        kw->vt = String;

        return( s );
}


/*-->GetKeyVal*/
/**********************************************************************/
/*****************************  GetKeyVal  ****************************/
/**********************************************************************/

        /* get next keyword-value pair
         * decode value according to table entry
         */

int GetKeyVal( kw, tab, nt, tno)
KeyWord *kw; 
KeyDesc tab[];
int     nt;
int     *tno;
{
        int i;
        char c = '\0';

        *tno = -1;

        for(i=0; i<nt; i++)
                if( IsSame(kw->Key, tab[i].Entry) ) {
                        *tno = i;
                        switch( tab[i].Type ) {
                                case None: 
                                        if( kw->vt != None ) return( FALSE );
                                        break;
                                case String:
                                        if( kw->vt != String ) return( FALSE );
                                        break;
                                case Integer:
                                        if( kw->vt != String ) return( FALSE );
                                        if( sscanf(kw->Val,"%d%c", &(kw->v.i), &c) != 1
                                            || c != '\0' ) return( FALSE );
                                        break;
                                case Number:
                                case Dimension:
                                        if( kw->vt != String ) return( FALSE );
                                        if( sscanf(kw->Val,"%f%c", &(kw->v.n), &c) != 1
                                            || c != '\0' ) return( FALSE );
                                        break;
                                }
                        kw->vt = tab[i].Type;
                        return( TRUE );
                        }

        return( TRUE );
}
#endif

/*-->HasBeenRead*/
/**********************************************************************/
/***************************  HasBeenRead  ****************************/
/**********************************************************************/

int
HasBeenRead(k)
int k;
{
    struct font_entry *ptr;

    ptr = hfontptr;
    while ((ptr!=NULL) && (ptr->k!=k))
	ptr = ptr->next;
    return( ptr != NULL );
}


/*-->IsSame*/
/**********************************************************************/
/*******************************  IsSame  *****************************/
/**********************************************************************/

int IsSame(a, b)        /* compare strings, ignore case */
char *a, *b;
{
        for( ; *a != '\0'; )
                if( tolower(*a++) != tolower(*b++) ) return( FALSE );
        return( *a == *b ? TRUE : FALSE );
}


/*-->lcase*/
/**********************************************************************/
/****************************  lcase  *********************************/
/**********************************************************************/

void lcase(s)
char *s;
{
        char *t;
        for(t=s; *t != '\0'; t++)
	  *t = tolower(*t);
        return;
}


/*-->MoveDown*/
/**********************************************************************/
/****************************  MoveDown  ******************************/
/**********************************************************************/

void
MoveDown(a)
int a;
{
    v += a;
}


/*-->MoveOver*/
/**********************************************************************/
/****************************  MoveOver  ******************************/
/**********************************************************************/

void
MoveOver(b)
int b;
{
    h += b;
}


/*-->NoSignExtend*/
/**********************************************************************/
/***************************  NoSignExtend  ***************************/
/**********************************************************************/

int
NoSignExtend(fp, n)	/* return n byte quantity from file fd */
register FILE *fp;	/* file pointer    */
register int n;		/* number of bytes */

{
    register int x;	/* number being constructed */

    x = getc(fp);
    while (--n)  {
	x <<= 8;
	x |= getc(fp);
    }
    return(x);
}


/*-->OpenFontFile*/
/**********************************************************************/
/************************** OpenFontFile  *****************************/
/**********************************************************************/

void
OpenFontFile()
/***********************************************************************
    The original version of this dvi driver reopened the font file  each
    time the font changed, resulting in an enormous number of relatively
    expensive file  openings.   This version  keeps  a cache  of  up  to
    MAXOPEN open files,  so that when  a font change  is made, the  file
    pointer, pxlfp, can  usually be  updated from the  cache.  When  the
    file is not found in  the cache, it must  be opened.  In this  case,
    the next empty slot  in the cache  is assigned, or  if the cache  is
    full, the least used font file is closed and its slot reassigned for
    the new file.  Identification of the least used file is based on the
    counts of the number  of times each file  has been "opened" by  this
    routine.  On return, the file pointer is always repositioned to  the
    beginning of the file.

***********************************************************************/
{
    register int i,least_used,current;
    int firsttime = 1;

#ifdef DEBUG
    if (Debug)
	(void) printf("Open Font file\n");
#endif
    if (pfontptr == fontptr)
        return;                 /* we need not have been called */

    for (current = 1;
	(current <= nopen) &&
	    (pixel_files[current].font_entry != fontptr);
	++current)
	;                       /* try to find file in open list */

    if (current <= nopen)       /* file already open */
    {
	if( (pxlfp = pixel_files[current].pixel_file_id) != NO_FILE )
	        (void) fseek(pxlfp,(long)0,0);	/* reposition to start of file */
    }
    else while (1) {		/* file not in open list */
        if (firsttime && nopen < MAXOPEN) /* just add it to list */
            current = ++nopen;
	else {			/* list full -- find least used file, */
				/* close it, and reuse slot for new file */
	    FILE *fid;
	    
	    least_used = 1;
            for (i = 2; i <= nopen; ++i)
	        if (pixel_files[least_used].use_count >
                    pixel_files[i].use_count)
		    least_used = i;
	    fid = pixel_files[least_used].pixel_file_id;
            if (fid != NO_FILE) {
                struct font_entry *fp = pixel_files[least_used].font_entry;

		fp->font_file_id = NULL;
#ifdef STATS
		if (Stats)
		    (void) fprintf(stderr, "PXL file %s closed.\n", fp->name);
#endif
 	        (void) fclose( fid );
	    }
	    current = least_used;
        }
	firsttime = 0;
	/* it seems we have to use open/fdopen on Suns to get the errno */
	if ((i = open(fontptr->name, O_RDONLY)) < 0) {
	    if (errno == EMFILE) {
		--nopen;
		continue;
	    }
	    else {
		perror("");
		Warning("PXL file %s could not be opened",fontptr->name);
		pxlfp = NO_FILE;
            }
	}
	else if ((pxlfp = fdopen(i, "r")) == NULL) {
	    Warning("Could not fdopen PXL file %s", fontptr->name);
	    pxlfp = NO_FILE;
	}
        else {
#ifdef STATS
            if (Stats) 
                (void) fprintf(stderr, "PXL file %s opened.\n", fontptr->name);
#endif
            }
	pixel_files[current].pixel_file_id = pxlfp;
	pixel_files[current].font_entry = fontptr;
	pixel_files[current].use_count = 0;
	break;
    }
    pfontptr = fontptr;			/* make previous = current font */
    fontptr->font_file_id = pxlfp;	/* set file identifier */
    pixel_files[current].use_count++;	/* update reference count */
}

#ifdef CREOPT
/*-->OpenOutput*/   /* generate a unique file name and open it */
/**********************************************************************/
/*************************** OpenOutput *******************************/
/**********************************************************************/


FILE*
OpenOutput()
{
        FILE*   fp;
        long t;
        int  n = 0;
        char *p, *pp, b[256];
        int nd;

        time( &t );
        t = t % 100000;
        (void) strcpy( outfname, SPOOLFILE );
        (void) sprintf( b, "%s.%s.%x", logname(), rootname, t );
        if( (nd=strlen(b)-MAXFLEN) > 0 ) {
               for(pp=(p=rindex(b,'.')); p && *p != '\0'; *(pp-nd) = *p++, pp++) ;
               *(pp-nd) = '\0';
               }
        (void) strcat( outfname, b );

        while( access(outfname,0) == 0 ) {
                n += 1;
                if( n > 10 ) 
                        Fatal( "Unable to create a unique output file name: %s\n", outfname );
                (void) strcpy( outfname, SPOOLFILE );
                (void) sprintf( b, "%s.%s.%x.%d", logname(), rootname, t, n );
                if( (nd=strlen(b)-MAXFLEN) > 0 ) {
                        for(pp=(p=rindex(b,'.')); p && *p != '\0'; *(pp-nd) = *p++, pp++) ;
                        *(pp-nd) = '\0';
                        }
                (void) strcat( outfname, b );
                }

        if( (fp=fopen(outfname,"w")) == NULL )
                Fatal("Unable to create output file: %s\n", outfname);

        return( fp );
}
#endif CREOPT

/*-->PixRound*/
/**********************************************************************/
/*****************************  PixRound  *****************************/
/**********************************************************************/

int
PixRound(x, conv)	/* return rounded number of pixels */
    register int x;		/* in DVI units     */
    int conv;			/* conversion factor */
{
    int sign = 0;
    if (x < 0) {
	sign++;
	x = -x;
    }
    x = (int)((x + (conv >> 1)) / conv);
    if (sign)
	x = -x;
    return x;
}


/*-->PutInt*/
/**********************************************************************/
/*****************************  PutInt  *******************************/
/**********************************************************************/

void
PutInt(n)               /* output an integer followed by a space */
register int n;
{
    char buf[10];
    register char *b;

    if( n == 0 )
        EMITC('0'); 
    else {
        if( n < 0 ) {
            EMITC('-');
            n = -n;
            }
    
        for(b=buf;  n>0;  ) {
            *b++ = digit[n%10];
            n /= 10;
            }
    
        for( ; b>buf; )
            EMITC(*--b);
        }

    EMITC(' ');
}


/*-->PutOct*/
/**********************************************************************/
/*****************************  PutOct  *******************************/
/**********************************************************************/

void
PutOct(n)               /* output an 3 digit octal number preceded by a "\" */
register int n;
{                  
    EMITC( '\\' ); 
    EMITC( digit[(n&0300)>>6] );
    EMITC( digit[(n&0070)>>3] ); 
    EMITC( digit[n&0007] );
}


/*-->ReadFontDef*/
/**********************************************************************/
/****************************  ReadFontDef  ***************************/
/**********************************************************************/

int
ReadFontDef(k)
int k;
{
    int t, i;
    register struct font_entry *tfontptr;/* temporary font_entry pointer   */
    register struct char_entry *tcharptr;/* temporary char_entry pointer  */
    int nmag;
    char nname[128];

    if ((tfontptr = NEW(struct font_entry)) == NULL)
	Fatal("can't malloc space for font_entry");
    tfontptr->next = hfontptr;
    tfontptr->font_file_id = NULL;
    fontptr = hfontptr = tfontptr;

    tfontptr->ncdl = 0;
#ifdef STATS
    tfontptr->nbpxl = 0;
    tfontptr->ncts = 0;
#endif

    tfontptr->k = k;
    tfontptr->c = NoSignExtend(dvifp, 4); /* checksum */
    tfontptr->s = NoSignExtend(dvifp, 4); /* space size */
    tfontptr->d = NoSignExtend(dvifp, 4); /* design size */
    tfontptr->a = NoSignExtend(dvifp, 1); /* area length for font name */
    tfontptr->l = NoSignExtend(dvifp, 1); /* device length */
    GetBytes(dvifp, tfontptr->n, tfontptr->a+tfontptr->l);
    tfontptr->n[tfontptr->a+tfontptr->l] = '\0';
    tfontptr->font_space = tfontptr->s/6; /* never used */
    tfontptr->font_mag = (int)((ActualFactor((int)(((float)tfontptr->s/
    			(float)tfontptr->d)*1000.0 + 0.5)) * 
#ifdef USEGLOBALMAG
			ActualFactor(mag) *
#endif
			(float)RESOLUTION * 5.0) + 0.5);

    if ((t = findfile(PXLvec,nPXLvec,"",
			 tfontptr->n,tfontptr->font_mag,tfontptr->name,
			 nname, &nmag)) == 0)
	Fatal("no font %s.%d",tfontptr->n,mag);
      
    if (tfontptr != pfontptr)
	OpenFontFile();
    if ( pxlfp == NO_FILE ) {                /* allow missing pxl files */
        tfontptr->magnification = 0;
        tfontptr->designsize = 0;
        for (i = FIRSTPXLCHAR; i <= LASTPXLCHAR; i++) {
	    tcharptr = &(tfontptr->ch[i]);
	    tcharptr->width = 0;
	    tcharptr->height = 0;
	    tcharptr->xOffset= 0;
	    tcharptr->yOffset = 0;
	    tcharptr->where.isloaded = 0;
	    tcharptr->where.address.fileOffset = NONEXISTANT;
	    tcharptr->tfmw = 0;
            }
        return;
        }

    tfontptr->type = (t == -1) ? FONT_PXL : FONT_PK;
    
    if (tfontptr->type == FONT_PXL) { /* PXL file */
	if ((t = NoSignExtend(pxlfp, 4)) != PXLID)
	    Fatal("PXL ID = %d, can only process PXL ID = %d files",
		  t, PXLID);
	(void) fseek(pxlfp, (long)-20, 2);
	t = NoSignExtend(pxlfp, 4);
	if ((tfontptr->c != 0) && (t != 0) && (tfontptr->c != t))
	    Warning("font = \"%s\",\n\
-->font checksum = %d,\n\
-->dvi checksum = %d",
		    tfontptr->name, tfontptr->c, t);
	tfontptr->magnification = NoSignExtend(pxlfp, 4);
	tfontptr->designsize = NoSignExtend(pxlfp, 4);

	(void) fseek(pxlfp, (long) (NoSignExtend(pxlfp, 4) * 4), 0);

	for (i = FIRSTPXLCHAR; i <= LASTPXLCHAR; i++) {
	    tcharptr = &(tfontptr->ch[i]);
	    tcharptr->width = NoSignExtend(pxlfp, 2);
	    tcharptr->height = NoSignExtend(pxlfp, 2);
	    tcharptr->xOffset= SignExtend(pxlfp, 2);
	    tcharptr->yOffset = SignExtend(pxlfp, 2);
	    tcharptr->where.isloaded = 0;
	    tcharptr->where.address.fileOffset = NoSignExtend(pxlfp, 4) * 4;
	    tcharptr->tfmw = ((float)NoSignExtend(pxlfp, 4) *
			      (float)tfontptr->s) /
				  (float)(1<<20);
	    tcharptr->kern = 0;
	}
    }
    else {			/* PK file */
	int hppp, vppp;

	if (NoSignExtend(pxlfp, 1) != PK_PRE)
	    Fatal("Pre command missing from PK file");
	if ((t = NoSignExtend(pxlfp, 1)) != PK_ID)
	    Fatal("PK ID = %d, can only process PK ID = %d files",
		  t, PK_ID);
	/* throw away comment */
	for (i = NoSignExtend(pxlfp, 1); i > 0; i--)
	    (void) NoSignExtend(pxlfp, 1);
	tfontptr->designsize = NoSignExtend(pxlfp, 4);
	t = NoSignExtend(pxlfp, 4);
	if ((tfontptr->c != 0) && (t != 0) && (tfontptr->c != t))
	    Warning("font = \"%s\",\n\
-->font checksum = %d,\n\
-->dvi checksum = %d",
		    tfontptr->name, tfontptr->c, t);
	hppp = NoSignExtend(pxlfp, 4);
	vppp = NoSignExtend(pxlfp, 4);
	if (hppp != vppp)
	    Warning("font = \"%s\" - aspect ratio not 1:1", tfontptr->name);
	tfontptr->magnification = hppp * 72.27 * (float)5/65536 + 0.5;
	/* don't read all character info now - just mark them as pk data */
	for (i = FIRSTPXLCHAR; i < LASTPXLCHAR; i++) {
	    tcharptr = &(tfontptr->ch[i]);
	    tcharptr->where.isloaded = 0;
	    tcharptr->where.address.fileOffset = 0;
	}
	tfontptr->offset = ftell(pxlfp);
    }
}


/*-->ReadPostAmble*/
/**********************************************************************/
/**************************  ReadPostAmble  ***************************/
/**********************************************************************/

void
ReadPostAmble(load)
int     load;
/***********************************************************************
    This  routine  is  used  to  read  in  the  postamble  values.    It
    initializes the magnification and checks  the stack height prior  to
    starting printing the document.
***********************************************************************/
{
    FindPostAmblePtr (&postambleptr);
    if (NoSignExtend(dvifp, 1) != POST)
	Fatal ("POST missing at head of postamble");
#ifdef DEBUG
    if (Debug)
	(void) fprintf (stderr, "got POST command\n");
#endif
    ppagep = NoSignExtend(dvifp, 4);
    num = NoSignExtend(dvifp, 4);
    den = NoSignExtend(dvifp, 4);
    mag = NoSignExtend(dvifp, 4);
#ifdef USEGLOBALMAG
    if( usermag > 0 && usermag != mag )
        (void) fprintf(stderr, "DVI magnification of %d over-ridden by user mag of %d\n", mag, usermag );
#endif
    if( usermag > 0 ) mag = usermag;
#ifndef USEGLOBALMAG
    if( mag != 1000 )
	(void) fprintf(stderr, "Magnification of %d ignored.\n", mag);
#endif
    hconv = DoConv(num, den, hconvRESOLUTION);
    vconv = DoConv(num, den, vconvRESOLUTION);

    (void) NoSignExtend(dvifp, 4);	/* height-plus-depth of tallest page */
    (void) NoSignExtend(dvifp, 4);	/* width of widest page */
    if (NoSignExtend(dvifp, 2) >= STACKSIZE)
	Fatal ("Stack size is too small");
    (void) NoSignExtend(dvifp, 2);	/* this reads the number of pages in */
    /* the DVI file */
#ifdef DEBUG
    if (Debug)
	(void) fprintf (stderr, "now reading font defs");
#endif
    if (load) GetFontDef ();
}


/*-->SetChar*/
/**********************************************************************/
/*****************************  SetChar  ******************************/
/**********************************************************************/

LoadAChar(ptr)
register struct char_entry *ptr;
{
    long *pr;
    register int nints;

    if (ptr->where.address.fileOffset == NONEXISTANT) {
	ptr->where.address.pixptr = NULL;
        ptr->where.isloaded = loadpass;
	return;
        }

    if (ptr->where.isloaded == 0) {
	/* haven't had this character before */
	OpenFontFile();
	if (fontptr->type == FONT_PXL) { /* this is a PXL file */
	    (void) fseek(pxlfp, ptr->where.address.fileOffset, 0);
	    nints = ((ptr->width + 31) >> 5) * ptr->height;
	    pr = (long *)malloc((unsigned)(nints*sizeof(long)) );
	    if (pr == NULL)
		Fatal("Unable to allocate memory for char\n");
	    (void) fread((char *)pr, 4, nints, pxlfp);
	    ptr->where.address.pixptr = pr;
	}
	else {			/* this is a PK file */
	    int packet_length, raster_length;

	    if (ptr->where.address.fileOffset == 0) {
		int car;
		FlagByte flag;

		/* look for char in PK file */
		(void) fseek(pxlfp, fontptr->offset, 0);
		flag = SkipSpecials();
		while (flag.code != PK_POST &&
		       ptr->where.address.fileOffset == 0) {
		    fontptr->offset = ftell(pxlfp) - 1;
		    if (flag.extended) {
			if (flag.length_prefix == 3) {
			    /* long character preamble */
			    packet_length = NoSignExtend(pxlfp, 4);
			    car = NoSignExtend(pxlfp, 4);
			}
			else {
			    /* extended short character preamble */
			    packet_length = (flag.length_prefix << 16) +
				NoSignExtend(pxlfp, 2);
			    car = NoSignExtend(pxlfp, 1);
			}
		    }
		    else {
			/* short character preamble */
			packet_length = (flag.length_prefix << 8) +
			    NoSignExtend(pxlfp, 1);
			car = NoSignExtend(pxlfp, 1);
		    }
		    if (car > 127 || car < 0)
			Warning("Character %d in font %s out of range",
				car, fontptr->name);
		    else
			fontptr->ch[car].where.address.fileOffset =
			    fontptr->offset;
#ifdef	notdef
		    fprintf(stderr, "font %s: flag %d car %d pl %d dyn_f %d\n",
			    fontptr->name, flag.code, car, packet_length,
			    flag.dyn_f);
#endif	notdef
		    /* just make a note of where this character is, and then
		       skip past it */
		    if (packet_length <= 0)
			Fatal("Bad packet length %d in %s",
			      packet_length, fontptr->name);
		    (void) fseek(pxlfp, (long)packet_length, 1);
		    flag = SkipSpecials();
		}
	    }
	    if (ptr->where.address.fileOffset == 0)
		Fatal("Character missing from font");
	    (void) fseek(pxlfp, ptr->where.address.fileOffset, 0);
	    ptr->flag.code = NoSignExtend(pxlfp, 1);
	    if (ptr->flag.extended) {
		if (ptr->flag.length_prefix == 3) {
		    /* read long preamble */
		    packet_length = NoSignExtend(pxlfp, 4);
		    (void) NoSignExtend(pxlfp, 4); /* car */
		    ptr->tfmw = ((float)NoSignExtend(pxlfp, 4) *
				 (float)fontptr->s) /
				     (float)(1<<20);
		    (void) NoSignExtend(pxlfp, 4); /* hor_esc */
		    (void) NoSignExtend(pxlfp, 4); /* ver_esc */
		    ptr->width = NoSignExtend(pxlfp, 4);
		    ptr->height = NoSignExtend(pxlfp, 4);
		    ptr->xOffset = SignExtend(pxlfp, 4);
		    ptr->yOffset = SignExtend(pxlfp, 4);
		    raster_length = packet_length - 24;
		}
		else {
		    /* read extended short preamble */
		    packet_length = ((ptr->flag.length_prefix) << 16) +
			NoSignExtend(pxlfp, 2);
		    (void) NoSignExtend(pxlfp, 1); /* car */
		    ptr->tfmw = (NoSignExtend(pxlfp, 1) << 16);
		    ptr->tfmw += NoSignExtend(pxlfp, 2);
		    ptr->tfmw = ((float)ptr->tfmw * (float)fontptr->s) /
			(float)(1<<20);
		    (void) NoSignExtend(pxlfp, 2); /* hor_esc */
		    ptr->width = NoSignExtend(pxlfp, 2);
		    ptr->height = NoSignExtend(pxlfp, 2);
		    ptr->xOffset = SignExtend(pxlfp, 2);
		    ptr->yOffset = SignExtend(pxlfp, 2);
		    raster_length = packet_length - 13;
		}
	    }
	    else {
		/* read short preamble */
		packet_length = ((ptr->flag.length_prefix) << 8) 
		  + NoSignExtend(pxlfp, 1);
		(void) NoSignExtend(pxlfp, 1); /* car */
		ptr->tfmw = (NoSignExtend(pxlfp, 1) << 16);
		ptr->tfmw += NoSignExtend(pxlfp, 2);
		ptr->tfmw = ((float)ptr->tfmw * (float)fontptr->s) /
		    (float)(1<<20);
		(void) NoSignExtend(pxlfp, 1); /* hor_esc */
		ptr->width = NoSignExtend(pxlfp, 1);
		ptr->height = NoSignExtend(pxlfp, 1);
		ptr->xOffset = SignExtend(pxlfp, 1);
		ptr->yOffset = SignExtend(pxlfp, 1);
		raster_length = packet_length - 8;
	    }
	    pr = (long *)malloc((unsigned)raster_length);
	    if (pr == NULL)
		Fatal("Unable to allocate memory for char\n");
	    (void) fread((char *)pr, raster_length, 1, pxlfp);
	    ptr->where.address.pixptr = pr;
#ifdef	notdef
	    fprintf(stderr, "PK Char: t=%d, w=%d, h=%d, x=%d, y=%d\n",
		    ptr->tfmw, ptr->width, ptr->height,
		    ptr->xOffset, ptr->yOffset);
#endif
	}
    }
    ptr->where.isloaded = loadpass;

    EmitChar(ptr);
        /* we should really free the space used by the PXL data after this
           point, but it is not large, and besides, we may want to be
           more clever in the future, about sending bitmaps.  So keep
           the data around */
}

FlagByte
SkipSpecials()
{
    int i;
    FlagByte flag;

    do {
	flag.code = NoSignExtend(pxlfp, 1);
	if (flag.code >= 240) {
	    switch (flag.code) {
	    case 240: case 241: case 242: case 243:
		for (i = NoSignExtend(pxlfp, flag.code-240+1); i > 0; i--)
		    NoSignExtend(pxlfp, 1);
		break;
	    case 244:
		(void) NoSignExtend(pxlfp, 4);
		break;
	    case 245: case 246:
		break;
	    default:
		Fatal("Unexpected %d !", flag);
	    }
	}
    } while (flag.code >= 240 && flag.code != PK_POST);
    return flag;
}

void
SetChar(c, command, PassNo)
int c, command, PassNo;
{
    register struct char_entry *ptr;  /* temporary char_entry pointer */

    ptr = &(fontptr->ch[c]);

    if (PassNo == 0) {
	if (ptr->where.isloaded != loadpass)
	    LoadAChar(ptr);
	return;
    }

    SetPosn(h,v,ptr->kern);
    SetXFont(ptr->where.xfont);
    if (fontptr->font_file_id != NO_FILE) {      /* ignore missing fonts */
	EMITC(ptr->where.glyph);
        hh += ptr->realw;
    }

    if (command <= SET4)
        h += ptr->tfmw;

#ifdef STATS
    Stnc += 1;
    fontptr->ncts += 1;
#endif
}


/*-->SetFntNum*/
/**********************************************************************/
/****************************  SetFntNum  *****************************/
/**********************************************************************/

void
SetFntNum(k)
int k;

/*  this routine is used to specify the font to be used in printing future
    characters */

{
    fontptr = hfontptr;
    while ((fontptr!=NULL) && (fontptr->k!=k))
	fontptr = fontptr->next;
    if (fontptr == NULL)
	Fatal("font %d undefined", k);
}

void
SetXFont(xfont)
    struct xfont *xfont;
{
    register int i;
    int minused = 0;

    if (xfont == 0)
	return;
    if (curxfont == xfont) {
	curxfont->usage++;
	return;
    }
    for (i = 0; i < MAXFONTID && assigned_font[i]; i++) {
	if (assigned_font[i] == xfont) {
	    EMIT(outfp,"%c%d",ESC,i);
	    curxfont = xfont;
	    curxfont->usage++;
	    return;
	}
	if (assigned_font[i]->usage < assigned_font[minused]->usage)
	    minused = i;
    }
    if (i >= MAXFONTID)
	i = minused;
    EMIT(outfp,"%c+%d%s\n%c%d",ESC,i,xfont->xname,ESC,i);
    assigned_font[i] = curxfont = xfont;
    xfont->usage = 1;
}

/*-->SetPosn*/
/**********************************************************************/
/*****************************  SetPosn  ******************************/
/**********************************************************************/

void
SetPosn(x, y, kern)		/* output a positioning command */
int x, y;
{
    int rx,ry;
    rx = PixRound(x-hh, hconv) - kern;
    ry = PixRound(y-vv, vconv);
#ifdef USERELPOS
    if (ry == 0) { /* use relative movement if just moving horizontally */
	if (rx != 0) {
	    if (rx > 0) {
		if (rx < 4) {
		    if (rx & 2)
			EMITC(MINCH+2);
		    if (rx & 1)
			EMITC(MINCH+1);
		}
		else
		    EMIT(outfp, "%crr%d ", ESC, rx);
	    }
	    else
		EMIT(outfp, "%crl%d ", ESC, -rx);
	    hh += rx*hconv;
	}
    }
    else if (rx == 0) {
	if (ry > 0)
	    EMIT(outfp, "%crd%d ", ESC, ry);
	else
	    EMIT(outfp, "%cru%d ", ESC, -ry);
	vv += ry*vconv;
    }
    else {
#endif
	EMIT(outfp, "%ca%d,%d\n", ESC,
	     (LEFT+(rx=PixRound(x,hconv))),
	     (TOP-(ry=PixRound(y,vconv))));
	/* must know "real" position on device for relative positioning */
	hh = rx*hconv;
	vv = ry*vconv;
#ifdef USERELPOS
    }
#endif
}


/*-->SetRule*/
/**********************************************************************/
/*****************************  SetRule  ******************************/
/**********************************************************************/

void
SetRule(a, b, Set)
int a, b;
BOOLEAN Set;

{	    /*	 this routine will draw a rule */

    if( a > 0 && b > 0 ) {
	if (a > b)
	    EMIT(outfp,"%cy%d,%d,%d,%d\n",ESC,
		 LEFT+PixRound(h,hconv),TOP-PixRound(v,vconv),
		 PixRound(a,vconv),PixRound(b,hconv));
	else
	    EMIT(outfp,"%cx%d,%d,%d,%d\n",ESC,
		 LEFT+PixRound(h,hconv),TOP-PixRound(v,vconv),
		 PixRound(b,hconv),PixRound(a,vconv));
    }
    if (Set)
	h += b;
}


/*-->SetString*/
/**********************************************************************/
/*****************************  SetString  ****************************/
/**********************************************************************/
#ifdef	notdef
void
SetString(firstch, PassNo)              /* read and set a consecutive string of chars */
int firstch, PassNo;
{
    char s[256];
    register char *sp;
    register int  c;
    register struct char_entry *ptr;
    int len;

    /* read entire string of chars */

    for(c = firstch, sp = s;  c >= SETC_000 && c <= SETC_127; ) {
        *sp++ = c;
        c = NoSignExtend(dvifp, 1);
        }
    (void) fseek(dvifp, (long) -1, 1);	/* backup one character */

    len = sp - s;               /* NULL's are valid chars, so cant use for string termination */

    /* ensure that all characters are loaded, */

    for(sp = s; sp < s+len; sp++) {
        ptr = &(fontptr->ch[*sp]);
        if(ptr->where.isloaded != loadpass)
	    LoadAChar(ptr);
    }

    /* emit the instructions */

    if( PassNo == 0 ) return;
    SetPosn(h, v, 0);
    for( sp=s;  sp < s+len;  sp++) {
	ptr = &(fontptr->ch[*sp]);
	if( fontptr->font_file_id != NO_FILE ) {     /* ignore missing fonts */
	    SetXFont(ptr->where.xfont);
	    EMITC(ptr->where.glyph);
	    hh += ptr->realw;
	}
	h += ptr->tfmw;
    }

#ifdef STATS
    Stnc += len;
    fontptr->ncts += len;
#endif
}
#endif

/*-->SignExtend*/
/**********************************************************************/
/****************************  SignExtend  ****************************/
/**********************************************************************/

int
SignExtend(fp, n)   /* return n byte quantity from file fd */
register FILE *fp;  /* file pointer    */
register int n;     /* number of bytes */

{
    int n1;         /* number of bytes	    */
    register int x; /* number being constructed */

    x = getc(fp);   /* get first (high-order) byte */
    n1 = n--;
    while (n--)  {
	x <<= 8;
	x |= getc(fp);
    }

    /* NOTE: This code assumes that the right-shift is an arithmetic, rather
    than logical, shift which will propagate the sign bit right.   According
    to Kernighan and Ritchie, this is compiler dependent! */

    x<<=32-8*n1;
    x>>=32-8*n1;  /* sign extend */

#ifdef DEBUG
    if (Debug)
    {
	(void) fprintf(stderr,"\tSignExtend(fp,%d)=%X\n",n1,x);
    }
#endif
    return(x);
}


/*-->SkipFontDef*/
/**********************************************************************/
/****************************  SkipFontDef  ***************************/
/**********************************************************************/

void
SkipFontDef(k)
int k;
{
    int a, l;
    char n[STRSIZE];

    (void) NoSignExtend(dvifp, 4);
    (void) NoSignExtend(dvifp, 4);
    (void) NoSignExtend(dvifp, 4);
    a = NoSignExtend(dvifp, 1);
    l = NoSignExtend(dvifp, 1);
    GetBytes(dvifp, n, a+l);
}


/*-->Warning*/
/**********************************************************************/
/*****************************  Warning  ******************************/
/**********************************************************************/
/*VARARGS1*/

void
Warning(fmt, a, b, c)  /* issue a warning */
char *fmt;	/* format   */
char *a, *b, *c;	/* arguments */
{
    if (G_logging == 0)
    {
        if (G_logfile)
	        G_logfp=fopen(G_Logname,"w+");
        else {
                G_logfp=stderr;
                if( G_nowarn ) return;
                }
	G_logging = 1;
	if (G_logfp == NULL) G_logging = -1;
    }

    G_errenc = TRUE;
    if (G_logging == 1)
    {
	(void) fprintf(G_logfp, fmt, a, b, c);
	(void) fprintf(G_logfp,"\n");
    }
}


