static char *RCSid = "$Header: arcio.c,v 1.2 86/07/15 07:53:11 turner Exp $";

/*
 * $Log:	arcio.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.2  86/07/15  07:53:11  turner
 * 
 * 
 * Revision 1.1  86/06/26  15:00:21  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCIO

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =2.30), created on $tag(
TED_DATE DB =02/03/86) at $tag(
TED_TIME DB =22:56:00))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the file I/O routines used to manipulate
         an archive.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"

INT readhdr(hdr,f)                     /* read a header from an archive */
struct heads *hdr;                     /* storage for header */
FILE *f;                               /* archive to read header from */
{
#if BSD | ST
        unsigned char dummy[28];
	INT i,j,k;
#endif
    char name[FNLEN];                  /* filename buffer */
    INT try = 0;                       /* retry counter */
    static INT first = 1;              /* true only on first read */

    if(!f)                             /* if archive didn't open */
         return 0;                     /* then pretend it's the end */
    if(feof(f))                        /* if no more data */
         return 0;                     /* then signal end of archive */

    if(fgetc(f)!=ARCMARK)             /* check archive validity */
    {    if(warn)
         {    printf("An entry in %s has a bad header.\n",arcname);
              nerrs++;
         }

         while(!feof(f))
         {    try++;
              if(fgetc(f)==ARCMARK)
              {    ungetc(hdrver=fgetc(f),f);
                   if(hdrver>=0 && hdrver<=ARCVER)
                        break;
              }
         }

         if(feof(f) && first)
              abort("%s is not an archive",arcname);

         if(warn)
              printf("  %d bytes skipped.\n",try);

         if(feof(f))
              return 0;
    }

    hdrver = fgetc(f);                 /* get header version */
    if(hdrver<0)
         abort("Invalid header in archive %s",arcname);
    if(hdrver==0)
         return 0;                     /* note our end of archive marker */
    if(hdrver>ARCVER)
    {    fread(name,sizeof(char),FNLEN,f);
         printf("I don't know how to handle file %s in archive %s\n",
              name,arcname);
         printf("I think you need a newer version of ARC.\n");
         exit(1);
    }

    /* amount to read depends on header type */

    if(hdrver==1)                      /* old style is shorter */
    {    fread(hdr,sizeof(struct heads)-sizeof(long),1,f);
         hdrver = 2;                   /* convert header to new format */
         hdr->length = hdr->size;      /* size is same when not packed */
    }
    else {
#if MSDOS
	fread(hdr,sizeof(struct heads),1,f);
#endif
#if BSD | ST
	fread(dummy,27,1,f);

	for(i=0;i<13;hdr->name[i]=dummy[i],i++);
	hdr->size = (long)((dummy[16]<<24) + (dummy[15]<<16) + (dummy[14]<<8)
	    + dummy[13]);
	hdr->date = (short)((dummy[18]<<8) + dummy[17]);
	hdr->time = (short)((dummy[20]<<8) + dummy[19]);
	hdr->crc  = (short)((dummy[22]<<8) + dummy[21]);
	hdr->length = (long)((dummy[26]<<24) + (dummy[25]<<16)
	    + (dummy[24]<<8) + dummy[23]);
#endif
    }

    first = 0; return 1;               /* we read something */
}

INT writehdr(hdr,f)                        /* write a header to an archive */
struct heads *hdr;                     /* header to write */
FILE *f;                               /* archive to write to */
{
    unsigned char dummy[28];

    fputc(ARCMARK,f);                 /* write out the mark of ARC */
    fputc(hdrver,f);                   /* write out the header version */
    if(!hdrver)                        /* if that's the end */
         return;                       /* then write no more */
#if MSDOS
    fwrite(hdr,sizeof(struct heads),1,f);
#endif
#if BSD | ST
/*
 * put out the hdr in the brain damaged unaligned half back *sswards
 * way HAL does it
 */
    fwrite(hdr->name,1,13,f);
    fwrite(&hdr->size,sizeof(long),1,f);
    fwrite(&hdr->date,sizeof(INT),1,f);
    fwrite(&hdr->time,sizeof(INT),1,f);
    fwrite(&hdr->crc ,sizeof(INT),1,f);
    fwrite(&hdr->length,sizeof(long),1,f);
#endif

    /* note the newest file for updating the archive timestamp */

    if(hdr->date>arcdate
    ||(hdr->date==arcdate && hdr->time>arctime))
    {    arcdate = hdr->date;
         arctime = hdr->time;
    }
}

INT filecopy(f,t,size)                     /* bulk file copier */
FILE *f, *t;                           /* from, to */
long size;                             /* number of bytes */
{
 INT len;                           /* length of a given copy */
 INT putc_tst();

    while(size--)                      /* while more bytes to move */
         putc_tst(fgetc(f),t);
}

INT putc_tst(c,t)                          /* put a character, with tests */
char c;                                /* character to output */
FILE *t;                               /* file to write to */
{
    if(t)
#if MSODS | ST
         if(fputc(c,t)==EOF)
              abort("Write fail (disk full?)");
#endif
#if BSD
/*
 * for reasons beyond me BSD unix returns EOF 
 */
	fputc(c,t);
#endif
}
