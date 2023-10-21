static char *RCSid = "$Header: arcdel.c,v 1.2 86/07/15 07:52:53 turner Exp $";

/*
 * $Log:	arcdel.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.2  86/07/15  07:52:53  turner
 * 
 * 
 * Revision 1.1  86/06/26  15:00:04  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCDEL

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =2.09), created on $tag(
TED_DATE DB =02/03/86) at $tag(
TED_TIME DB =22:53:27))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to delete entries
         in an archive.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"

INT delarc(num,arg)                        /* remove files from archive */
INT num;                               /* number of arguments */
char *arg[];                           /* pointers to arguments */
{
    struct heads hdr;                  /* header data */
 INT del;                           /* true to delete a file */
 INT did[MAXARG];                  /* true when argument used */
 INT n;                             /* index */

    if(!num)                           /* she must specify which */
         abort("You must tell me which files to delete!");

    for(n=0; n<num; n++)               /* for each argument */
         did[n] = 0;                   /* reset usage flag */
    rempath(num,arg);                  /* strip off paths */

    openarc(1);                        /* open archive for changes */

    while(readhdr(&hdr,arc))           /* while more entries in archive */
    {    del = 0;                      /* reset delete flag */
         for(n=0; n<num; n++)          /* for each template given */
         {    if(match(hdr.name,arg[n]))
              {    del = 1;            /* turn on delete flag */
                   did[n] = 1;         /* turn on usage flag */
                   break;              /* stop looking */
              }
         }

         if(del)                       /* skip over unwanted files */
         {    fseek(arc,hdr.size,1);
              if(note)
                   printf("Deleting file: %s\n",hdr.name);
         }
         else                          /* else copy over file data */
         {    writehdr(&hdr,new);      /* write out header and file */
              filecopy(arc,new,hdr.size);
         }
    }

    hdrver = 0;                        /* special end of archive type */
    writehdr(&hdr,new);                /* write out archive end marker */
    closearc(1);                       /* close archive after changes */

    if(note)
    {    for(n=0; n<num; n++)          /* report unused arguments */
         {    if(!did[n])
              {    printf("File not found: %s\n",arg[n]);
                   nerrs++;
              }
         }
    }
}
