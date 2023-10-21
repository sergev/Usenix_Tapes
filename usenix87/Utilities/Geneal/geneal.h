/* geneal.h - general include stuff for geneal modules */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 05:07:41 by jimmc (Jim McBeath) */
/* last edit 10-Sept-85 07:19:00 by tlr (Terry L. Ridder) */

#include <stdio.h>

#define TBLSIZ 100

struct tblblock {
    int repnum;			/* number of indexes represented by this blk */
    int count;			/* count of entries in this level */
    int tdata[TBLSIZ];		/* the actual data for this level */
    };

struct toplevel {
    int numlevs;		/* how deep the table is at it's deepest */
    struct tblblock *data;	/* pointer to the top level */
    };

struct dpoint {
    FILE *ff;
    struct toplevel *xx;
    };

/* global variables */
struct dpoint *gendp;		/* data file index number */
extern int indexes;		/* set means output index numbers w names */

/* declare functions */
char *index();

char *getData();
char *strsav();
char *tprintf();

char *fgstr();			/* originally char *famgetstr()		*/
char *tname();			/* originally char *famgettname() 	*/
char *bname();			/* originally char *famgetbname() 	*/
char *fname();			/* originally char *famgetfname() 	*/
char *fgbirth();		/* originally char *famgetbirth() 	*/
char *fgdeath();		/* originally char *famgetdeath() 	*/
char *fgbegend();		/* originally char *famgetbirthdeath() 	*/
char *fgmar();			/* originally char *famgetmarriage() 	*/

/* end */
