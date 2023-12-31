static char *RCSid = "$Header: arctst.c,v 1.1 86/06/26 15:01:02 turner Exp $";

/*
 * $Log:	arctst.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.1  86/06/26  15:01:02  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCTST

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =2.12), created on $tag(
TED_DATE DB =02/03/86) at $tag(
TED_TIME DB =23:00:40))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to test archive integrity.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"

INT tstarc()                               /* test integrity of an archive */
{
    struct heads hdr;                  /* file header */
    long arcsize, ftell();             /* archive size */

    openarc(0);                        /* open archive for reading */
    fseek(arc,0L,2);                   /* move to end of archive */
    arcsize = ftell(arc);              /* see how big it is */
    fseek(arc,0L,0);                   /* return to top of archive */

    printf ("Testing archive...\n");
    while(readhdr(&hdr,arc))
    {    if(ftell(arc)+hdr.size>arcsize)
         {    printf("Archive truncated in file %s\n",hdr.name);
              nerrs++;
              break;
         }

         else
         {    printf("    File: %-12s  ",hdr.name);
              fflush(stdout);
              if(unpack(arc,NULL,&hdr))
                   nerrs++;
              else printf("okay\n");
         }
    }

    if(nerrs<1)
         printf("No errors detected\n");
    else if(nerrs==1)
         printf("One error detected\n");
    else printf("%d errors detected\n",nerrs);
}
