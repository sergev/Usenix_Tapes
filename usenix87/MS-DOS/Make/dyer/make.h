/* #define	VAXVMS	1 */		/* uncomment for VAX/VMS */
/* #define	MSDOS	1 */		/* uncomment for MSDOS */

#ifdef VAXVMS
#define ESCCHAR	`\\`		/* ok to use backslash on VMS */
#endif

#ifdef MSDOS
#define	ESCCHAR	'`'		/* since pathname char is backslash (yech) */
#endif

#define	MACCHAR '#'		/* macro-definition char */
#define	COMCHAR	'!'		/* comment char */
#define	DEFMAC	"="		/* macro-definition token */
#define	DEPEND	":"		/* dependency-definition token */

#define	DEBUG	if(0)
#define	STRSIZ	512
#define	MAXMODS	50

/* file attributes */
#define	REBUILT	0x01		/* file has been reconstructed */
#define	ROOTP	0x02		/* file was named on left side of DEPEND */


struct date_str {
	unsigned ds_low, ds_high;
};
typedef struct date_str *DATE;


struct node {
	struct filenode *nfile;	/* this node's file */
	struct node *nnext;	/* the next node */
};
typedef struct node NODE;


struct filenode {
	char *fname;		/* the filename */
	char *fmake;		/* remake string for file */
	DATE fdate;		/* 32 bit last-modification date */
	NODE *fnode;		/* files this file depends on */
	char fflag;		/* magic flag bits */
	struct filenode *fnext;	/* the next file */
};
typedef struct filenode FILENODE;


struct macro {
	char *mname;		/* the macro's name */
	char *mvalue;		/* the macro's definition */
	struct macro *mnext;	/* the next macro */
};
typedef struct macro MACRO;


extern MACRO *mroot;
extern FILENODE *froot;
extern DATE bigbang;		/* Far, far in the past */
extern DATE endoftime;		/* Far, far in the future */
char *gmacro();
FILENODE *filenode(), *gfile();
char *token();
