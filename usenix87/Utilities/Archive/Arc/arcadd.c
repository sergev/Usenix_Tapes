/*  ARC - Archive utility - ARCADD

System V Version 1.0 based upon:
    Version 3.39, created on 02/05/86 at 22:21:53

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to add files to an archive.
*/
#include "arc.h"

INT addarc(num,arg,move,update,fresh)  /* add files to archive */
INT num;                               /* number of arguments */
char *arg[];                           /* pointers to arguments */
INT move;                              /* true if moving file */
INT update;                            /* true if updating */
INT fresh;                             /* true if freshening */
{
    char *buf;                         /* pathname buffer */
    char *i, *rindex();                /* string indexing junk */
    INT n;                             /* indices */
    struct heads hdr;                  /* file header data storage */
    INT addfile();

    openarc(1); /* open archive for changes */

    for (n=0; n<num; ++n) {
        if (i=rindex(buf=arg[n], '/')) buf=i+1;
        addfile(arg[n],buf,update,fresh);
    }

    /* now we must copy over all files that follow our additions */

    while (readhdr(&hdr,arc))           /* while more entries to copy */
    {    writehdr(&hdr,new);
         filecopy(arc,new,hdr.size);
    }
    hdrver = 0;                        /* archive EOF type */
    writehdr(&hdr,new);                /* write out our end marker */
    closearc(1);                       /* close archive after changes */

    if (move) for (n=0; n<num; ++n)    /* if this was a move */
        if (unlink(arg[n]) && warn) {
            printf("Cannot unsave %s\n",arg[n]);
            ++nerrs;
         }
}

static INT addfile(path,name,update,fresh) /* add named file to archive */
char *path;                                /* path name of file to add */
char *name;                                /* name of file to add */
INT update;                                /* true if updating */
INT fresh;                                 /* true if freshening */
{
    struct heads nhdr;                 /* data regarding the new file */
    struct heads ohdr;                 /* data regarding an old file */
    FILE *f, *fopen();                 /* file to add, opener */
    long starts, ftell();              /* file locations */
    INT c;                             /* one char of file */
    INT upd = 0;                       /* true if replacing an entry */

    if (!(f=fopen(path,"r"))) {
        if (warn)
         {    printf("Cannot read file: %s\n",path);
              nerrs++;
         }
    }

    strcpy(nhdr.name,name);            /* save name */
    nhdr.size = 0;                     /* clear out size storage */
    nhdr.crc = 0;                      /* clear out CRC check storage */
    getstamp(f,&nhdr.date,&nhdr.time);

    /* position archive to spot for new file */

    if (arc)                           /* if adding to existing archive */
    {    starts = ftell(arc);          /* where are we? */
         while (readhdr(&ohdr,arc))    /* while more files to check */
         {    if (!strcmp(ohdr.name,nhdr.name))
              {    upd = 1;            /* replace existing entry */
                   if (update || fresh) /* if updating or freshening */
                   {    if (nhdr.date<ohdr.date
                        || (nhdr.date==ohdr.date && nhdr.time<=ohdr.time))
                        {    fseek(arc,starts,0);
                             fclose(f);
                             return;   /* skip if not newer */
                        }
                   }
              }

              if (strcmp(ohdr.name,nhdr.name)>=0)
                   break;              /* found our spot */

              writehdr(&ohdr,new);     /* entry preceeds update; keep it */
              filecopy(arc,new,ohdr.size);
              starts = ftell(arc);     /* now where are we? */
         }

         if (upd)                      /* if an update */
         {    if (note)
                 { printf("Updating file: %-12s  ",name); fflush(stdout);}
              fseek(arc,ohdr.size,1);
         }
         else if (fresh)               /* else if freshening */
         {    fseek(arc,starts,0);     /* then do not add files */
              fclose(f);
              return;
         }
         else                          /* else adding a new file */
         {    if (note)
                 { printf("Adding file:   %-12s  ",name); fflush(stdout);}
              fseek(arc,starts,0);     /* reset for next time */
         }
    }

    else                               /* no existing archive */
    {    if (fresh)                    /* cannot freshen nothing */
         {    fclose(f);
              return;
         }
         else if (note)                 /* else adding a file */
            { printf("Adding file:   %-12s  ",name); fflush(stdout);}
    }

    starts = ftell(new);               /* note where header goes */
    hdrver = ARCVER;                   /* anything but end marker */
    writehdr(&nhdr,new);               /* write out header skeleton */
    pack(f,new,&nhdr);                 /* pack file into archive */
    fseek(new,starts,0);               /* move back to header skeleton */
    writehdr(&nhdr,new);               /* write out real header */
    fseek(new,nhdr.size,1);            /* skip over data to next header */
    fclose(f);                         /* all done with the file */
}
