#include <stdio.h>
#include "config.h"
#include "vn.h"

/*
	global data structure
*/
NODE *Newsorder [HASHMAX];	/* .newsrc file order */
char *Editor,*Ps1,*Mailer,*Printer,*Poster;

char Erasekey, Killkey;		/* user keys from stty */
char *Newsrc, *Orgdir;		/* .newsrc file, and original pwd */
char *Onews;			/* temp. file for backing up .newsrc */
char *Savefile = DEF_SAVE;	/* file in which to save articles */
char *Savedir;			/* default directory for saved articles */

int Rot;	/* rotation */
int Headflag;	/* header printing flag */
int Digest;	/* if non-zero, digest article */

/*
	cur_page - current page displayed;
	lrec - last record
	l_allow - lines allowed for article display
	c_allow - columns allowed
	ncount = newsorder index
	nfltr - number of filters
*/
int Cur_page, Lrec, L_allow, C_allow, Ncount, Nfltr;

/*
	article filters from options
*/
FILTER Filter [NUMFILTER];

/*
	current page
*/
PAGE Page;
