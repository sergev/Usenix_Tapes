static char *RCSid = "$Header: arclst.c,v 1.2 86/07/15 07:53:15 turner Exp $";

/*
 * $Log:	arclst.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.2  86/07/15  07:53:15  turner
 * 
 * 
 * Revision 1.1  86/06/26  15:00:23  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCLST

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =2.34), created on $tag(
TED_DATE DB =02/03/86) at $tag(
TED_TIME DB =22:56:57))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to list the contents
         of an archive.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"

INT lstarc(num,arg)                        /* list files in archive */
INT num;                               /* number of arguments */
char *arg[];                           /* pointers to arguments */
{
    struct heads hdr;                  /* header data */
 INT list;                          /* true to list a file */
 INT did[MAXARG];                  /* true when argument was used */
    long tnum, tlen, tsize;            /* totals */
 INT n;                             /* index */
    INT lstfile();

    tnum = tlen = tsize = 0;           /* reset totals */

    for(n=0; n<num; n++)               /* for each argument */
         did[n] = 0;                   /* reset usage flag */
    rempath(num,arg);                  /* strip off paths */

    if(!kludge)
    {    printf("Name          Length  ");
         if(bose)
              printf("  Stowage    SF   Size now");
         printf("  Date     ");
         if(bose)
              printf("  Time    CRC");
         printf("\n");

         printf("============  ========");
         if(bose)
              printf("  ========  ====  ========");
         printf("  =========");
         if(bose)
              printf("  ======  ====");
         printf("\n");
    }

    openarc(0);                        /* open archive for reading */

    if(num)                            /* if files were named */
    {    while(readhdr(&hdr,arc))      /* process all archive files */
         {    list = 0;                /* reset list flag */
              for(n=0; n<num; n++)     /* for each template given */
              {    if(match(hdr.name,arg[n]))
                   {    list = 1;      /* turn on list flag */
                        did[n] = 1;    /* turn on usage flag */
                        break;         /* stop looking */
                   }
              }

              if(list)                 /* if this file is wanted */
              {    if(!kludge)
                        lstfile(&hdr); /* then tell about it */
                   tnum++;             /* update totals */
                   tlen += hdr.length;
                   tsize += hdr.size;
              }

              fseek(arc,hdr.size,1);   /* move to next header */
         }
    }

    else while(readhdr(&hdr,arc))      /* else report on all files */
    {    if(!kludge)
              lstfile(&hdr);
         tnum++;                       /* update totals */
         tlen += hdr.length;
         tsize += hdr.size;
         fseek(arc,hdr.size,1);        /* skip to next header */
    }

    closearc(0);                       /* close archive after reading */

    if(!kludge)
    {    printf("        ====  ========");
         if(bose)
              printf("            ====  ========");
         printf("\n");
    }

    printf("Total %6ld  %8ld  ",tnum,tlen);
    if(bose)
         printf("          %3ld%%  %8ld  ",100L - (100L*tsize)/tlen,tsize);
    printf("\n");

    if(note)
    {    for(n=0; n<num; n++)          /* report unused args */
         {    if(!did[n])
              {    printf("File not found: %s\n",arg[n]);
                   nerrs++;
              }
         }
    }
}

static INT lstfile(hdr)                    /* tell about a file */
struct heads *hdr;                     /* pointer to header data */
{
 INT yr, mo, dy;                    /* parts of a date */
 INT hh, mm, ss;                    /* parts of a time */

    static char *mon[] =               /* month abbreviations */
    {    "Jan",    "Feb",    "Mar",    "Apr",
         "May",    "Jun",    "Jul",    "Aug",
         "Sep",    "Oct",    "Nov",    "Dec"
    };

    yr = (hdr->date >> 9) & 0x7f;      /* dissect the date */
    mo = (hdr->date >> 5) & 0x0f;
    dy = hdr->date & 0x1f;

    hh = (hdr->time >> 11) & 0x1f;     /* dissect the time */
    mm = (hdr->time >> 5) & 0x3f;
    ss = (hdr->time & 0x1f) * 2;

    printf("%-12s  %8ld  ",hdr->name,hdr->length);

    if(bose)
    {    switch(hdrver)
         {
         case 1:
         case 2:
              printf("   --   ");
              break;
         case 3:
              printf(" Packed ");
              break;
         case 4:
              printf("Squeezed");
              break;
         case 5:
         case 6:
         case 7:
              printf("crunched");
              break;
         case 8:
              printf("Crunched");
              break;
         default:
              printf("Unknown!");
         }

         printf("  %3d%%",100L - (100L*hdr->size)/hdr->length);
         printf("  %8ld  ",hdr->size);
    }

    printf("%2d %3s %02d", dy, mon[mo-1], (yr+80)%100);

    if(bose)
         printf("  %2d:%02d%c  %04x",
              (hh>12?hh-12:hh), mm, (hh>12?'p':'a'),
              (unsigned INT)hdr->crc);

    printf("\n");
}
