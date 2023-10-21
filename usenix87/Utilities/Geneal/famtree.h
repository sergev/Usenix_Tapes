/* famtree.h - header file for family tree stuff */
/* last edit 15-Sep-84 21:05:02 by jimmc (Jim McBeath) */
/* last edit 10-Sept-85 08:09:00 by tlr (Terry L. Ridder) */

extern char *xalloc();

#define XALLOC(item, count, msg) (item *)xalloc(sizeof(item)*count,msg)

struct fsq {		/* a family square */
    char *pname;	/* name of the family */
    int pnum;		/* index number of the parents (marriage) */
    int fnum;		/* index number of the father of the family */
    int mnum;		/* index number of the mother of the family */
    int chloc;		/* where the child of interest lies in child list */
    int ccount;		/* number of children in the family */
    int *clist;		/* list of child indexes for the family */
    char **cnlist;	/* list of names for children */
    char **cblist;	/* list of birthdates for children */
    int cnum;		/* number of the child with desc. tree */
    struct fsq *ffamily;	/* pointer to father's familysq */
    struct fsq *mfamily;	/* pointer to mother's familysq */
    int lines;		/* number of lines in this square */
    int cols;		/* number of cols in this square */
    int agens;		/* number of generations previous to this one */
    int alines;		/* number of lines for all ancestor families */
    int acolmax;	/* max columns for all ancestor families */
    int chline;		/* the line where the child tree comes out */
    };

typedef struct fsq Fsq, *Fsqp;

/* end */
