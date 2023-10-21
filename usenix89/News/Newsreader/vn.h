#include "tune.h"

#define TRUE 1
#define FALSE 0

#define NARGOPT "lprxfuMs"

#define FIL_AUTHOR 'w'
#define FIL_TITLE 't'

/*
	newsrc states
*/
#define NEWS_ON ':'
#define NEWS_OFF '!'

/* bit flags for state of newsgroup */
#define FLG_SCAN 1
#define FLG_SUB 2
#define FLG_PAGE 4
#define FLG_WRIT 8
#define FLG_SPEC 16

#define LIST_SEP " 	,"
#define ED_MARK '>'
#define ART_MARK '*'
#define ART_WRITTEN '_'
#define ART_UNWRITTEN ' '

#define FPFIX "Re: "

#define ANFORM ":%s - ? for help:\n"
#define ANFLINES 1
#define NOFORM "can't open article %s\n"
#define NEWGFORM "groups not mentioned in %s:\n"
#define SAVFORM "save file (%s) ? "
#define UDKFORM "undefined key (x%x) - ? for help"

/*
	page display format and dependent parameters
*/
#define HFORMAT "\n%s (page %d of %d):"
#define DHFORMAT "\n%s (DIGEST EXTRACTION):"
#define TFORMAT "%s ~ %s %s"
#define AFORMAT "\n%c%c%d) "	/* begin with newline - see show routine */
#define CFORMAT "page %d of %d (%d shown), newsgroup %d of %d"
#define RECBIAS 2	/* lines before articles - depends on HFORMAT */
#define AFLEN 5		/* min. char. in article id - depends on AFORMAT */
#define WRCOL 1		/* column of written mark.  depends on AFORMAT */
#define INFOLINE 0	/* HFORMAT TFORMAT leaves for use */

/*
	command characters - don't use numerics
	ALTSAVE is a hack to avoid having to use ctl-s - XON/XOFF.
	Wanted to preserve "s" pneumonic and lower / control /cap
	convention.
*/
#define DIGEST 'd'
#define UP 'k'
#define DOWN 'j'
#define FORWARD '\012'
#define BACK '\010'
#define READ 'r'
#define ALTREAD ' '
#define READALL 'R'
#define READSTRING '\022'
#define SAVE 's'
#define SAVEALL 'S'
#define SAVESTRING '\023'
#define ALTSAVE '\024'
#define PRINT 'p'
#define PRINTALL 'P'
#define PRINTSTRING '\020'
#define MARK 'x'
#define UNMARK 'X'
#define REDRAW '\014'
#define QUIT 'q'
#define SSTAT '#'
#define GRPLIST '%'
#define ORGGRP 'o'
#define ORGSTAT 'O'
#define UPDATE 'w'
#define UNSUBSCRIBE 'u'
#define UPALL 'W'
#define UPSEEN '\027'
#define UNESC '!'
#define NEWGROUP 'n'
#define HEADTOG 'h'
#define SETROT 'z'
#define HELP '?'
#define HELP_HEAD "[...] = effect of optional number preceding command\n\
pipes are specified by filenames beginning with |\n\
articles specified as a list of numbers, title search string, or\n\
	* to specify marked articles.  ! may be used to negate any\n"

#define HHLINES 5	/* lines (CRs + 1) contained in HELP_HEAD */

/*
	state flags for handling breaks / values for sig_set calls.
	BRK_IN, BRK_SESS, BRK_READ and BRK_OUT are the states.  All
	but BRK_INIT are used as calls to sig_set.  BRK_RFIN indicates
	a return from BRK_READ to BRK_SESS (no jump location passed),
*/
#define BRK_INIT 0		/* initial value, indicating uncaught signals */
#define BRK_IN 1		/* in NEWSRC / article scanning phase */
#define BRK_SESS 2		/* in page interactive session */
#define BRK_READ 3		/* reading articles */
#define BRK_RFIN 4		/* finished reading, return to old mode */
#define BRK_OUT 5		/* NEWSRC updating phase */

#define BRK_PR "really quit ? "
#define BRK_MSG "\nQUIT (signal %d)"

/*
	filter structure for article screening:

	rex - regular expression
	hcomp - header line to compare against regular expression
	neg - negation flag indicating "not matching"
*/
typedef struct
{
	char *rex,hcomp,neg;
} FILTER;

/*
	newsgroup structure (node of hash table)
	nd_name - name of newsgroup (key to reach node by)
	pnum - page number
	pages - number of pages for news display
	rdnum - articles read
	orgrd - original articles read number
	pgshwn - pages shown mask
	pgrd - article number on highest conecutively shown page
	art - articles in group
	state - status
*/
typedef struct
{
	char *nd_name;
	int pnum,pages,art,rdnum,orgrd,pgrd;
	unsigned long pgshwn;
	unsigned state;
} NODE;

/*
	newsgroup information for page display
	name - of group
	group - pointer to table entry
	artnum - number of articles
*/
typedef struct
{
	char *name;
	NODE *group;
	int artnum;
} HEAD;

/*
	article information - id (spool) number, title string, mark, written.
*/
typedef struct
{
	int art_id;
	char art_mark;
	char art_written;
	char art_t[MAX_C-AFLEN];
} BODY;

typedef struct
{
	HEAD h;
	BODY *b;
} PAGE;
