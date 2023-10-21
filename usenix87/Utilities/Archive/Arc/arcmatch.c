/*  ARC - Archive utility - ARCMATCH

System V Version 1.0 based upon:
    Version 2.17, created on 12/17/85) at 20:32:18

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains service routines needed to maintain an archive.
*/
#include "arc.h"

INT match(n,t)                         /* test name against template */
char *n;                               /* name to test */
char *t;                               /* template to test against */
{
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

INT rempath(nargs,arg)                 /* remove paths from filenames */
INT nargs;                             /* number of names */
char *arg[];                           /* pointers to names */
{
    char *i, *rindex();                /* string index, reverse indexer */
    INT n;                             /* index */

    for(n=0; n<nargs; n++)             /* for each supplied name */
    {    i=rindex(arg[n],'/');         /* search for end of path */
         if(i)                         /* if path was found */
              arg[n] = i+1;            /* then skip it */
    }
}
