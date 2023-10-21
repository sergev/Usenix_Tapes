
/*
	The include file for DISPLIB

	This version is for 4.[123]bsd UNIXs using termcap.

   Donated into the public domain for any use whatever by anyone.
   Please retain this header.
  
   This display library is a seriously modified version of a
   display library posted to USENET several years ago. Many thanks.
  
   * **************************************************************** *
   * Gary K. Sloane/Computer Sciences Corporation/San Diego, CA 92110 *
   * DDD: (619) 225-8401                                              *
   * MILNET: sloane@nosc.ARPA                                         *
   * UUCP: [ihnp4,akgua,decvax,dcdwest,ucbvax]!sdcsvax!noscvax!sloane *
   * **************************************************************** *
*/  

#ifndef DISPLIB
#define DISPLIB

/******************************************************************************/
/* DISPLIB terminal definition structure used by all display library routines */
/* and external termcap variables.                                            */
/******************************************************************************/
typedef struct termdef {
      char tcapbuf[512]; /* array where following pointers point to */
      char *BL;          /* audible bell                 */
      char *CD;          /* clear to end of display      */
      char *CE;          /* clear to end of line         */
      char *CL;          /* clear display                */
      char *CM;          /* cursor motion string         */
      char *IS;          /* initialize terminal          */
      char  PC;          /* pad character                */
      char *SE;          /* end standout mode            */
      char *SO;          /* start standout mode          */
      char *TE;          /* initialize CM mode           */
      char *TI;          /* terminate CM mode            */
      int scrlen;        /* screen length                */
      int scrwid;        /* screen width -1              */
      int curline;       /* current line                 */
      int curcol;        /* current column               */
      } TERMDEF;

/* displib functions that return something */
char *date2ymd();
int stoi();
int power();

/* termcap(3x) definitions */
char *tgoto(), *tgetstr();
extern char *UP;         /* cursor up for tgoto()        */
extern char *BC;         /* backspace for tgoto()        */
extern char PC;          /* pad character for tputs()    */
extern short ospeed;     /* tty speed for tputs()        */

/* tty control structure defined in terminit.c */
extern TERMDEF termctrl;

/* #defines for error() modes */
#define BEEP   0
#define NOBEEP 1

/* defines for the edits and editdate routines */
#define NORMAL 0
#define COLON  1
#define INVERSE 2

#endif DISPLIB
