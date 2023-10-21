/*  ARC - Archive utility - ARCCVT

System V Version 1.0 based upon:
    Version 1.16, created on 02/03/86 at 22:53:02

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to convert archives to use
         newer file storage methods.
*/
#include "arc.h"

static char tempname[STRLEN];          /* temp file name */

INT cvtarc(num,arg)                    /* convert archive */
INT num;                               /* number of arguments */
char *arg[];                           /* pointers to arguments */
{
    struct heads hdr;                  /* file header */
    INT cvt;                           /* true to convert current file */
    INT did[MAXARG];                   /* true when argument was used */
    INT n;                             /* index */
    char *makefnam();                  /* filename fixer */
    FILE *fopen();                     /* file opener */
    INT cvtfile();

    if(arctemp)                   /* use temp area if specified */
         sprintf(tempname,"%s.cvt",arctemp);
    else makefnam("$ARCTEMP.cvt",arcname,tempname);

    openarc(1);                        /* open archive for changes */

    for(n=0; n<num; n++)               /* for each argument */
         did[n] = 0;                   /* reset usage flag */
    rempath(num,arg);                  /* strip off paths */

    if(num)                            /* if files were named */
    {    while(readhdr(&hdr,arc))      /* while more files to check */
         {    cvt = 0;                 /* reset convert flag */
              for(n=0; n<num; n++)     /* for each template given */
              {    if(match(hdr.name,arg[n]))
                   {    cvt = 1;       /* turn on convert flag */
                        did[n] = 1;    /* turn on usage flag */
                        break;         /* stop looking */
                   }
              }

              if(cvt)                  /* if converting this one */
                   cvtfile(&hdr);      /* then do it */
              else                     /* else just copy it */
              {    writehdr(&hdr,new);
                   filecopy(arc,new,hdr.size);
              }
         }
    }

    else while(readhdr(&hdr,arc))      /* else convert all files */
         cvtfile(&hdr);

    hdrver = 0;                        /* archive EOF type */
    writehdr(&hdr,new);                /* write out our end marker */
    closearc(1);                       /* close archive after changes */

    if(note)
    {    for(n=0; n<num; n++)          /* report unused args */
         {    if(!did[n])
              {    printf("File not found: %s\n",arg[n]);
                   nerrs++;
              }
         }
    }
}

static INT cvtfile(hdr)                /* convert a file */
struct heads *hdr;                     /* pointer to header data */
{
    long starts, ftell();              /* where the file goes */
    FILE *tmp, *fopen();               /* temporary file */

    if(!(tmp=fopen(tempname,"w+")))
         abort("Unable to create temporary file %s",tempname);

    if(note)
     { printf("Converting file: %-12s   reading, ",hdr->name); fflush(stdout);}

    unpack(arc,tmp,hdr);               /* unpack the entry */
    fseek(tmp,0L,0);                   /* reset temp for reading */
    starts = ftell(new);               /* note where header goes */
    hdrver = ARCVER;                   /* anything but end marker */
    writehdr(hdr,new);                 /* write out header skeleton */
    pack(tmp,new,hdr);                 /* pack file into archive */
    fseek(new,starts,0);               /* move back to header skeleton */
    writehdr(hdr,new);                 /* write out real header */
    fseek(new,hdr->size,1);            /* skip over data to next header */
    fclose(tmp);                       /* all done with the file */
    if(unlink(tempname) && warn)
    {    printf("Cannot unsave %s\n",tempname);
         nerrs++;
    }
}
