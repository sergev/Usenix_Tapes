/*
 *	tpscript.h
 *	header definitions for tpscript.c:
 *		Troff post-processor for postscript devices
 */

#include	<stdio.h>
#include	"dev.h"
#include	"pscript.h"

#define	ERR_WARN	1	/* an input error - continue processing */
#define	ERR_FATAL	2	/* an input error - abort processing */
#define	ERR_SNARK	3	/* a program error - abort processing */
#define	UNDEFINED	-1

#define	BMASK		0xFF	/* low order byte */

#define	NASCII		128	/* no. ascii characters */
#define	NUNPRINT	32	/* no. unprintable characters */
#define	NASCPRINT	(NASCII-NUNPRINT)	/* no. printable ascii chars */
#define	MAXCHARS	255	/* max no. character codes */

#define	DEF_FONT	1	/* default initial font */
#define DEF_SIZE	10	/* default initial point size */
#define	DEF_DEV		"alw"	/* default device - apple laser-writer */

extern int	errno;
extern char	*sys_errlist[];

extern	FILE	*Debug ;
extern	char	*ifile;		/* current input file name */
extern	int	lineno,			/* line no. in current input file */
	npages;			/* no. pages printed so far */
extern	char	device[100],
	errbuf[100];		/* tmp buffer for error messages */
extern	int
	hpos,		/* current horizontal position */
	vpos;		/* current vertical position (rel. TOP pg.) */
extern	int	
	res,		/* resolution in THINGS/inch */
	hor_res,		/* min horizontal movement (in THINGS) */
	vert_res,		/* min vertical movement (in THINGS) */
	respunits;
float 	rotation;		/* page orientation (degrees) */
extern	int	currtfont,	/* current font number selected by troff */
	papertype; 		/* paper type (different imageable regions) */

/* font parameters */
struct	fontparam {
	int	fp_size;	/* character point size */
	float	fp_height,	/* character height (points) */
		fp_slant;	/* character slant (degrees) */
	struct fontdesc *
		fp_font;	/* font style */
};
extern	struct	fontparam
	tfp,		/* current troff font parameters */
	pfp;		/* current postscript font parameters */

#define	NOFONTDESC	((struct fontdesc *)NULL )

/* table of font descriptions */
struct fontdesc {
	char	*f_intname;	/* postscript name */
	char	*f_extname;	/* troff name */
	int	f_nent;		/* no. entries for this font */
	char	*f_widthtab;	/* character width tables */
	char	*f_codetab;	/* code table for this font */
	char	*f_fitab;	/* font index for this font */
	bool	f_mounted;	/* whether font is mounted or not */
};

extern struct fontdesc	*fontd,
			*spcfnt1,	/* special font */
			*spcfnt2;	/* special font 2 */

/* font mount table - array of pointers to font descriptions */
extern	struct fontdesc	**fontmount;

/* mapping between troff font names and builtin font names
 * This should go in the internal name part of the font description
 * itself, but there is only 10 bytes allocated (see dev.h).
 */
struct fontmap {
	char	*fm_extname;	/* Troff font name */
	char	*fm_intname;	/* Postscript font name */
};
extern	struct	fontmap fontmap[];

#define	NFONT	(sizeof(fontmap)/sizeof(fontmap[0]))

extern	struct dev	dev;

extern	short	*chartab;	/* char's index in charname array */
extern	char	*charname;	/* special character names */
extern	int	ncharname;		/* no. special character names */
extern	int	nfonts;		/* no. of fonts mounted */
extern	int	nfontmount;		/* no. of font mount positions */


#define	GETWIDTH(fdp, index)	\
  ((tfp.fp_size * ((fdp)->f_widthtab[ (index) ] & BMASK) + (respunits>>1)) / respunits)

	/*
	 * this is the width that the printer will have moved following
	 * the last printed character, if troff then says to move a
	 * different amount we will shift the difference
	 */
extern	int	width_pending;

extern	bool	word_started;	/* we are in middle of word string */

#define	CLOSEWORD()	{if ( word_started == TRUE ) {	\
				fputs( ")s", postr );	\
				word_started = FALSE;	\
				}			\
			}


extern	int		strcmp();
extern	char		*emalloc();
extern	struct fontdesc *findfont();
extern	struct fontmap	*getfmap();

