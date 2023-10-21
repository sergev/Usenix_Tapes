/* include srtunq.h first */

#define MXABR 26			/* upper case chars used */
extern char	*abrvtbl[];		/* translation table strings */
extern int	abrvlen[];		/* string lengths (for speed) */
extern SRTUNQ	abrv;			/* include file abrevs */

/* abbreviation fucntions */
extern int	lngsrt();		/* compare function for abrevs */
extern char	*hincl();		/* optimizer for include paths */
extern void	srchincl();		/* find [A-Z] makefile defines */
extern void	abrvsetup();		/* create abrvs, write them */
extern int	findabr();		/* find longest abrv */
