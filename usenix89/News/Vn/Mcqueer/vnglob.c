/*
** vn news reader.
**
** vnglob.c - global variables - see string.c also
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>
#include "config.h"
#include "vn.h"
#include "head.h"

/*
	global data structure
*/
NODE **Newsorder;		/* .newsrc file order */

char *Editor,*Ps1,*Mailer,*Printer,*Poster;

char Erasekey, Killkey;		/* user keys from stty */
char *Newsrc, *Orgdir;		/* .newsrc file, and original pwd */
char *Onews;			/* temp. file for backing up .newsrc */
char *Savefile = DEF_SAVE;	/* file in which to save articles */
char *Savedir;			/* default directory for saved articles */
char *Ccfile;			/* author_copy file, stored /bin/mail fmt */

int Rot;	/* rotation */
int Headflag;	/* header printing flag */
int Digest;	/* if non-zero, digest article */

char *Ku, *Kd, *Kl, *Kr;	/* Cursor movement capabilities */

/* character translation arrays for commands */
char Cxitop[128], Cxitor[128], Cxrtoi[128], Cxptoi[128];

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
	article filtration options.
*/
char *Wopt[NUMFILTER];		/* regular expressions for -w options */
char *Topt[NUMFILTER];		/* regular expressions for -t options */
char *Negwopt[NUMFILTER];	/* regular expressions for negated -w options */
char *Negtopt[NUMFILTER];	/* regular expressions for negated -t options */

int Nwopt, Ntopt, Nnwopt, Nntopt;

int Nounsub, Listfirst;
/*
	current page
*/
PAGE Page;
