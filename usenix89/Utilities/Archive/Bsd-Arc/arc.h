/*
 * $Header: arc.h,v 1.2 86/07/15 07:52:34 turner Exp $
 */

/*
 * $Log:	arc.h,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.2  86/07/15  07:52:34  turner
 * 
 * 
 * Revision 1.1  86/06/26  15:01:23  turner
 * initial version
 * 
 * 
 */

#define	ST	0	/* Atari 520ST or 1040 		 */
#define BSD	1	/* BSD4.2 on a vax		 */
#define MSDOS	0	/* MSDOS on an IBM PC or Wannabe */

#if ST
#define EXTERN
#define INT short
#endif

#if BSD
#include <ctype.h>	/* for isupper etc.                  */
#define EXTERN
#define INT short
#define envfind getenv
#endif

#if MSDOS
#define EXTERN extern
#define INT int
#endif

/*
 * added macro def's in C format 6/26/86 jmt 
 */
#include "arcm.h"

/*  ARC - Archive utility - ARC Header

    Version 2.14, created on 02/03/86 at 22:48:29

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description: 
         This is the header file for the ARC archive utility.  It defines
         global parameters and the references to the external data.


    Language:
         Computer Innovations Optimizing C86
 */

#include "arcs.h"

EXTERN INT keepbak;             /* true if saving the old archive */
EXTERN INT warn;                /* true to print warnings */
EXTERN INT note;                /* true to print comments */
EXTERN INT bose;                /* true to be verbose */
EXTERN INT nocomp;              /* true to suppress compression */
EXTERN INT kludge;              /* kludge flag */
EXTERN char arctemp[STRLEN];       /* arc temp file prefix */
EXTERN char *password;          /* encryption password pointer */
EXTERN INT nerrs;               /* number of errors encountered */

EXTERN char hdrver;                      /* header version */

EXTERN FILE *arc;                        /* the old archive */
EXTERN FILE *new;                        /* the new archive */

EXTERN char arcname[STRLEN];           /* storage for archive name */
EXTERN char bakname[STRLEN];           /* storage for backup copy name */
EXTERN char newname[STRLEN];           /* storage for new archive name */
EXTERN unsigned INT arcdate;    /* archive date stamp */
EXTERN unsigned INT arctime;    /* archive time stamp */
