/*  Global Defines  */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

/*  This data relates to the working directory and the portion of that
    directory currently displayed on the screen.
*/

extern int tfiles;		/* Number of files in this directory */

/*  The files of a directory are partitioned into pages.  Each page
    represents a display on the terminal screen.  */

#define	maxnfpp	26		/* Maximum number of files per page, a-z */
extern int nfpp;		/* Number of files per page */ 

extern int tpages,		/* Total pages this directory */
cpage;				/* Current page this display */
extern int pageoff;		/* Page offset for multicolumn display */
extern int column, colfield;

/*  File information about working dir */

extern struct stat wd_stb;	/* its inode */

/* Path name of working directory */
#define MPLEN	STRMAX - 1	/* Max path len (chars) */
#define LPLEN	STRMAX - 20	/* Limit path len */

extern int wdfile;		/* its file number */
extern char wdname [STRMAX];	/* its full path name */

/*#define	max_dir_size	(sizeof dirbuf - sizeof dirbuf [0]) */

/* extern char *d_namep [mfiles];	/* pointers to all the file names */

#ifndef	NDIR
extern char **d_namep;
#endif

/* filename returns a pointer to the file assoc. with arg on cpage */
#ifndef	NDIR
#define	filename(arg)	(d_namep [arg + (cpage+pageoff) * nfpp - nfpp])
#else
extern struct direct **d_dirp;
#define	filename(arg)	(d_dirp[arg + (cpage+pageoff) * nfpp - nfpp]->d_name)
#endif

extern int dir_status;			/* its logical status */
#define	loaded		0
#define	unloaded	1
#define	toobig		2
#define	protected	3

/*  scratch */
extern struct stat scr_stb;

/* Useful macros */

#define	DOT	"."
#define DOTDOT	".."
#define	SLASH	"/"
#define dirsize sizeof (struct direct)

#ifndef	NDIR
#define	dnamesize	DIRSIZ
#endif

#define	ISROOT(arg)	(arg [1] == 0)
