/*
 *	pscript.h
 *	Some common definitions for programs that produce postscript
 */

#ifndef	PSCRIPT_DEFINED
#define	PSCRIPT_DEFINED	1

#ifndef	VERBOSE
#if	lint
#define	VERBOSE	1	/* if linting, check all the code */
#else	lint
#define	VERBOSE	0
#endif	lint
#endif	VERBOSE

#if	UNSW
#undef	AUSAM
#define	AUSAM	1
#endif	UNSW

#ifdef	AUSAM
#undef	AUSAM
#define	AUSAM	1
#else
#define	AUSAM	0
#endif	AUSAM

#ifdef	BSD
#undef	BSD
#define	BSD	1
#define	index	strchr
#define	rindex	strrchr
#include	<strings.h>	/* declarations for string functions */
#else
#define	BSD	0
#include	<string.h>	/* declarations for string functions */
#endif	BSD

#ifdef	SYSV
#undef	SYSV
#define	SYSV	1
#else
#define	SYSV	0
#endif	SYSV

#ifdef	V7
#undef	V7
#define	V7	1
#else
#define	V7	0
#endif	V7

#if	AUSAM + BSD + SYSV + V7 != 1
#include	"Must define 1 only of AUSAM, BSD, SYSV, V7"
#endif

#include	<stdio.h>

/* LaserWriter page types */
#define	PT_DEFAULT	0
#define	PT_LETTER	1
#define	PT_LEGAL	2
#define	PT_NOTE		3
#ifdef	ALW
#define	PT_A4		4	/* currently only on Apple LaserWriter */
#endif	ALW

/* Postscript defaults */
#define	PD_PAGETYPE	PT_LETTER	/* page size is "letter" */
#define	PD_ROTATION	0		/* page is portrait mode */
#define	PD_PFREQUENCY	60		/* pixel frequency (pixels/inch) */
#define	PD_PROTATION	45		/* pixel grid rotation (degrees) */

#define	ALW_RES		300	/* LaserWriter resolution is 300 dots/inch */

#define	PU_INCH		72	/* postscript units / inch */
#define	PU_CM		28.3465	/* postscript units / cm */

#define	TRUE		(bool)1
#define	FALSE		(bool)0

typedef int	bool;

extern FILE *	postr;		/* output stream */

void	perror(),
	exit();

#endif	PSCRIPT_DEFINED
