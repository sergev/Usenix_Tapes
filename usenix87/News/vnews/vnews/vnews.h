#ifndef EXTERN
#define EXTERN extern
#endif

#ifdef HCURSES		/* Mark Horton's version of curses */
#define CURSES
#endif
#ifdef ACURSES		/* Ken Arnold's version of curses */
#define CURSES
#endif
#ifdef CURSES
#include <curses.h>
#define ROWS LINES
int hasscroll = 0;
#else
#endif CURSES

#include <errno.h>
#include "rparams.h"
#include "artfile.h"
#include "newsrc.h"

#ifdef CURSES
#undef LINES
#endif

#define ARTWLEN	(ROWS-2)/* number of lines used to display article */
#ifdef STATTOP
#define SECPRLEN 81	/* length of secondary prompter */
#else
#define SECPRLEN 100	/* length of secondary prompter */
#endif
#define INTR	'\7'	/* interrupt character */
/* prun flags */
#define CWAIT	0001	/* type "continue?" and wait for return */
#define BKGRND	0002	/* run process in the background */
/* values of curflag */
#define CURP1	1	/* cursor after prompt */
#define CURP2	2	/* cursor after secondary prompt */
#define CURHOME	3	/* cursor at home position */
/* types of article lists */
#define SVNEWSGROUP 0	/* a newsgroup was saved (must be zero) */
#define SVARTICLE   1	/* a sigle article was saved */
#if BSDREL >= 42
#define sigset signal
#endif

struct window {
	int w_artlin;			/* line to appear at top of window */
	int w_svartlin;			/* line currently at top of window */
	int (*w_dump)();		/* routine to dump window to screen */
	int w_force;			/* set to force window update */
};


struct svartlist {
	int al_type;			/* type of article list */
	int al_tfoffset;			/* offset into temp file where saved */
};

struct artinfo {
	ARTNO i_artnum;		/* number in spool directory */
	long  i_subtime;	/* date/time article posted, in internal GMT */
	long  i_basetime;	/* subtime of base article */
	DPTR  i_dptr;		/* artfile entry */
	long  i_nlines;		/* lines: header (number of lines) */
	char  i_title[37];	/* subject */
	char  i_from[31];	/* sender of message */
};

extern int errno;

/* terminal handler stuff */
extern int _junked;
#define clearok(xxx, flag) _junked = flag
extern int COLS;
extern int ROWS;
extern int hasscroll;
extern int needbeep;

EXTERN int ngrp;			/* set on entry to new group	*/
EXTERN char filename[BUFLEN];		/* file containing current art	*/

EXTERN FILE *tfp;			/* temporary file */
EXTERN long artbody;			/* offset of body into article */
EXTERN long artlength;			/* article length in bytes */
EXTERN int quitflg;			/* if set, then quit */
EXTERN int hupflag;			/* set when hangup signal received */
EXTERN int erased;			/* current article has been erased */
EXTERN int artlines;			/* # lines in article body */
EXTERN int artread;			/* entire article has been read */
EXTERN int hdrstart;			/* beginning of header */
EXTERN int hdrend;			/* end of header */
EXTERN int lastlin;			/* number of lines in tempfile */
EXTERN int tflinno;			/* next line in tempfile */
EXTERN int tfoffset;			/* offset of article in temp file */
EXTERN char secpr[SECPRLEN];		/* secondary prompt */
EXTERN char prompt[30];			/* prompter */
EXTERN short hdronly;			/* if set, only print article header */
EXTERN short curflag;			/* where to locate cursor */
EXTERN char timestr[20];		/* current time */
EXTERN int ismail;			/* true if user has mail */
EXTERN char *mailf;			/* user's mail file */
EXTERN int atend;			/* set if at end of article */
EXTERN char cerase;			/* erase character */
EXTERN char ckill;			/* kill character */
EXTERN int ospeed;			/* terminal speed */
EXTERN int pstatus;			/* status return from process */
EXTERN int indexpg;			/* set if on index page */
EXTERN struct window *curwin;		/* current window */
EXTERN struct window *nextwin;		/* new window */
#ifdef DIGPAGE
EXTERN int endsuba;			/* end of sub-article in digest */
#endif
#ifdef MYDEBUG
EXTERN FILE *debugf;			/* file to write debugging info on */
#endif
extern struct window artwin;		/* window displaying article */
extern struct window indexwin;		/* window containing article index */
extern struct window helpwin;		/* help windown */
extern struct window emptywin;		/* blank screen */
extern struct window hdrwin;		/* window containing header */

EXTERN struct artinfo *thisng;		/* articles in this newsgroup */
EXTERN int numthisng;			/* # of articles in above array */
EXTERN int curind;			/* how far we've gotten into thisng */
EXTERN struct artinfo *curart;		/* how far we've gotten into thisng */


EXTERN long pngsize;			/* Printing ngsize		*/
EXTERN int  news;
EXTERN struct arthead h;		/* header of current article	*/
EXTERN int dgest;
EXTERN FILE *fp;			/* current article to be printed*/
EXTERN int debugging;			/* debugging turned on */
