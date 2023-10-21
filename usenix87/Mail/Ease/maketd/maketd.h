#define TRUE	1
#define FALSE	0
#define SPC	'\040'

/* globals */
extern char	*prgnm;			/* the progs called name */
extern int	alldep;			/* -a all - /usr/include too */
extern int	usestdout;		/* -d use stdout for makefile */
extern FILE	*makefd;		/* file desc. for output file */
extern char	*makename;		/* -m default file for edit */
extern int	backedup;		/* for interupt recovery */
extern char	backupfn[];		/* backup file name */
extern int	replace;		/* -r replace depends in Makefile */
extern int	shortincl;		/* -x do abreviations */
extern int	verbose;		/* -v verbage for the debugger */
extern char	usage[];		/* for error output */

extern void	err();			/* error msg, exit cleanly */
extern void	errrec();		/* exit cleanly - also signal trap */
extern char	lastlnch();		/* last char in line (not \n) */
