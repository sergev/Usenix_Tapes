#
/* file ned.x.c- external definitions for new RAND editor */
/*   Walt Bilofsky - 14 January 1977 */

#include "ned.defs"

int tabstops[NTABS] {
  -1, 0,  8, 16, 24, 32, 40, 48, 56, 64,
 72, 80, 88, 96,104,112,120,128,136, BIGTAB,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,};

/* table of motions for control chars
	UP  1	up
	DN  2	down		 : 4 or less change cursorline
	RN  3	carriage return
	HO  4	home
	RT  5	right
	LT  6	left
	TB  7	tab
	BT  8	backtab
*/
char cntlmotions[32]
{
	0,   0,  HO,   0,   0,   0,   0,   0,
       LT,  TB,  RN,   0,   0,  DN,   0,   0,
	0,   0,   0,   0,   0,	 0,   0,   0,
	0,   0,  UP,   0,  RT,  BT,   0,   0,
};

char lread1 -1;		/* last char read, set to -1 when char is used*/

/* parameters for line motion buttons */

int defplline 10;		/* default plus a line		*/
int defmiline 10;		/* default minus a line 	*/
int defplpage 1;		/* default plus a page		*/
int defmipage 1;		/* default minus a page 	*/
int deflport 16;		/* default port left		*/
int defrport 16;		/* default port right		*/
int definsert 1;		/* default insert		*/
int defdelete 1;		/* default delete		*/
int defpick   1;		/* default pick 		*/
char deffile[]	   "/re.std";	/* default filename		*/

/* initialize cline to be empty */
int lcline 0;
int icline 20;			/* initial increment for expansion */
int clineno -1;
char fcline 0;

/* file	to remember button pushes */
int ttyfile -1;
int inputfile 0;

char pwbuf[100];

int slowsw 0;
