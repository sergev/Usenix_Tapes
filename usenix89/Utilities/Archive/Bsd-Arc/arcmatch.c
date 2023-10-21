static char *RCSid = "$Header: arcmatch.c,v 1.2 86/07/15 07:53:42 turner Exp $";

/*
 * $Log:	arcmatch.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.2  86/07/15  07:53:42  turner
 * 
 * 
 * Revision 1.1  86/06/26  15:00:34  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCMATCH

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =2.17), created on $tag(
TED_DATE DB =12/17/85) at $tag(
TED_TIME DB =20:32:18))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains service routines needed to maintain an archive.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"

INT match(n,t)                         /* test name against template */
char *n;                               /* name to test */
char *t;                               /* template to test against */
{

#if MSDOS
    upper(n); upper(t);                /* avoid case problems */
#endif

    /* first match name part */

    while((*n && *n!='.') || (*t && *t!='.'))
    {    if(*n!=*t && *t!='?')         /* match fail? */
         {    if(*t!='*')              /* wildcard fail? */
                   return 0;           /* then no match */
              else                     /* else jump over wildcard */
              {    while(*n && *n!='.')
                        n++;
                   while(*t && *t!='.')
                        t++;
                   break;              /* name part matches wildcard */
              }
         }
         else                          /* match good for this char */
         {    n++;                     /* advance to next char */
              t++;
         }
    }

    if(*n && *n=='.') n++;             /* skip extension delimiters */
    if(*t && *t=='.') t++;

    /* now match name part */

    while(*n || *t)
    {    if(*n!=*t && *t!='?')         /* match fail? */
         {    if(*t!='*')              /* wildcard fail? */
                   return 0;           /* then no match */
              else return 1;           /* else good enough */
         }
         else                          /* match good for this char */
         {    n++;                     /* advance to next char */
              t++;
         }
    }

    return 1;                          /* match worked */
}

INT rempath(nargs,arg)                     /* remove paths from filenames */
INT nargs;                             /* number of names */
char *arg[];                           /* pointers to names */
{
    char *i, *rindex();                /* string index, reverse indexer */
 INT n;                             /* index */

    for(n=0; n<nargs; n++)             /* for each supplied name */
    {    if(!(i=rindex(arg[n],'\\')))  /* search for end of path */
              if(!(i=rindex(arg[n],'/')))
                   i=rindex(arg[n],':');
         if(i)                         /* if path was found */
              arg[n] = i+1;            /* then skip it */
    }
}
