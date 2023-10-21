/* pathalias -- by steve bellovin, as told to peter honeyman */

/*#define STRCHR	/* have strchr, not index -- probably system v */
/*#define UNAME		/* have uname() -- probably system v or 8th ed. */
/*#define MEMSET	/* have memset() -- probably system v or 8th ed. */

/* #define GETHOSTNAME	/* have gethostname() -- probably 4.2bsd */
/* #define BZERO	/* have bzero() -- probably 4.2bsd */

/* #define DEBUG /* Useful if you like core dumps (keeps temp files too) */

/* location of temporary files for nodes and links */
/* estimated size about 2000 blocks (1.0mbytes), but will increase */
#define NTEMPFILE "/usr/spool/tmp/tmpnodes"
#define LTEMPFILE "/usr/spool/tmp/tmplinks"

/* This is the maximum size of a site's name.  It rapidly increases the */
/* the temp file's size, so keep it just over the maximum length. */
/* Although long names are unnecessary, some sites use them. */
/* I have kept this as small as possible since I want the temp file to */
/* fit on an RS04, but if you don't have one you may make it larger */

#define MAXNAME 40

/* default place for dbm output of makedb (or use -o file at run-time) */
#define	ALIASDB	"/usr/spool/news/lib/palias"

/* location of the printit phase of this program */
#define PRINTIT "/usr/lib/news/printit"
#define PRINTITC "/usr/lib/news/printit -c"

/**************************************************************************
 *									  *
 * +--------------------------------------------------------------------+ *
 * |									| *
 * |			END OF CONFIGURATION SECTION			| *
 * |									| *
 * |				EDIT NO MORE				| *
 * |									| *
 * +--------------------------------------------------------------------+ *
 *									  *
 **************************************************************************/

#ifdef MAIN
#ifndef lint
static char	*c_sccsid = "@(#)config.h	8.1 (down!honey) 86/01/19";
#endif /*lint*/
#endif /*MAIN*/

#ifdef STRCHR
#define index strchr
#define rindex strrchr
#else
#define strchr index
#define strrchr rindex
#endif

#ifdef BZERO
#define strclear(s, n)	((void) bzero((s), (n)))
#else /*!BZERO*/

#ifdef MEMSET
extern char	*memset();
#define strclear(s, n)	((void) memset((s), 0, (n)))
#else /*!MEMSET*/
extern void	strclear();
#endif /*MEMSET*/

#endif /*BZERO*/

extern char	*malloc();
extern char	*strcpy(), *index(), *rindex();
