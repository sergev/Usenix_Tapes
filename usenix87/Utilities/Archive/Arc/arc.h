/*
System V Version 1.0.
*/
#include <stdio.h>

#include <ctype.h>	/* for isupper etc.                  */
#define EXTERN
#define INT short
#define envfind getenv
#define index strchr
#define rindex strrchr

#include "arcm.h"

/*  ARC - Archive utility - ARC Header

    Version 2.14, created on 02/03/86 at 22:48:29

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description: 
         This is the header file for the ARC archive utility.  It defines
         global parameters and the references to the external data.

 */

#include "arcs.h"

EXTERN INT keepbak;             /* true if saving the old archive */
EXTERN INT warn;                /* true to print warnings */
EXTERN INT note;                /* true to print comments */
EXTERN INT bose;                /* true to be verbose */
EXTERN INT nocomp;              /* true to suppress compression */
EXTERN char arctemp[STRLEN];    /* arc temp file prefix */
EXTERN char *password;          /* encryption password pointer */
EXTERN INT nerrs;               /* number of errors encountered */

EXTERN char hdrver;             /* header version */

EXTERN FILE *arc;               /* the old archive */
EXTERN FILE *new;               /* the new archive */

EXTERN char arcname[STRLEN];    /* storage for archive name */
EXTERN char bakname[STRLEN];    /* storage for backup copy name */
EXTERN char newname[STRLEN];    /* storage for new archive name */
EXTERN unsigned INT arcdate;    /* archive date stamp */
EXTERN unsigned INT arctime;    /* archive time stamp */
