#ifndef GETARGS_INCLUDED
#define GETARGS_INCLUDED
/*
*
* Getargs.h
*
*!AU: Michael A. Shiels
*!CD: 22-May-86
*!FR: Dr. Dobb's May 1985
*
*/

#define ARG_INTEGER   0
#define ARG_TBOOLEAN  1
#define ARG_SBOOLEAN  2
#define ARG_FBOOLEAN  3
#define ARG_CHARACTER 4
#define ARG_STRING    5
 
typedef struct
{
		int				arg;			/* Command Line Switch      */
		int				type;			/* Variable Type            */
		int				*variable;		/* Pointer to Variable      */
		char			*errmsg;		/* Pointer to Error Message */
}
ARG ;

extern int	ARG_ICase;
extern int	ARG_Switch;

#ifdef LINT_ARGS

int	getargs( int, char **, ARG *, int );

#else

extern int	getargs();

#endif	/* LINT_ARGS */

#endif	/* GETARGS_INCLUDED */
