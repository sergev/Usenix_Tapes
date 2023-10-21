/*  ARC - Archive utility - ARCEXT

System V Version 1.0 based upon:
    Version 2.18, created on 02/03/86 at 22:55:19

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to extract files from
         an archive.
*/
#include "arc.h"
INT extarc(num,arg,prt)                /* extract files from archive */
INT num;                               /* number of arguments */
char *arg[];                           /* pointers to arguments */
INT prt;                               /* true if printing */
{
    struct heads hdr;                  /* file header */
    INT save;                          /* true to save current file */
    INT did[MAXARG];                   /* true when argument was used */
    char *i, *rindex();                /* string index */
    char **name, *malloc();            /* name pointer list, allocator */
    INT n;                             /* index */
    INT extfile();

    name = (char **)malloc(num*sizeof(char *));  /* get storage for name pointers */

    for(n=0; n<num; n++)               /* for each argument */
    {    did[n] = 0;                   /* reset usage flag */
         if(!(i=rindex(arg[n],'/')))  /* find start of name */
              i = arg[n]-1;
         name[n] = i+1;
    }

    openarc(0);                        /* open archive for reading */

    if(num)                            /* if files were named */
    {    while(readhdr(&hdr,arc))      /* while more files to check */
         {    save = 0;                /* reset save flag */
              for(n=0; n<num; n++)     /* for each template given */
              {    if(match(hdr.name,name[n]))
                   {    save = 1;      /* turn on save flag */
                        did[n] = 1;    /* turn on usage flag */
                        break;         /* stop looking */
                   }
              }

              if(save)                 /* extract if desired, else skip */
                   extfile(&hdr,arg[n],prt);
              else fseek(arc,hdr.size,1);
         }
    }

    else while(readhdr(&hdr,arc))      /* else extract all files */
         extfile(&hdr,"",prt);

    closearc(0);                       /* close archive after reading */

    if(note)
    {    for(n=0; n<num; n++)          /* report unused args */
         {    if(!did[n])
              {    printf("File not found: %s\n",arg[n]);
                   nerrs++;
              }
         }
    }

    free(name);
}

static INT extfile(hdr,path,prt)       /* extract a file */
struct heads *hdr;                     /* pointer to header data */
char *path;                            /* pointer to path name */
INT prt;                               /* true if printing */

{
    FILE *f, *fopen();                 /* extracted file, opener */
    char buf[STRLEN];                  /* input buffer */
    char fix[STRLEN];                  /* fixed name buffer */
    char *i, *rindex();                /* string index */

    if(prt)                            /* printing is much easier */
    {    unpack(arc,stdout,hdr);       /* unpack file from archive */
         printf("\f");                 /* eject the form */
         return;                       /* see? I told you! */
    }

    strcpy(fix,path);                  /* note path name template */
    if(!(i=rindex(fix,'/')))           /* find start of name */
        i = fix-1;
    strcpy(i+1,hdr->name);             /* replace template with name */

    if(note)
         printf("Extracting file: %s\n",fix);

    if(warn)
    {    if(f=fopen(fix,"r"))        /* see if it exists */
         {    fclose(f);
              printf("WARNING: File %s already exists!",fix);
              while(1)
              {    printf("  Overwrite it (y/n)? ");
                   fgets(buf,STRLEN,stdin);
                   *buf = toupper(*buf);
                   if(*buf=='Y' || *buf=='N')
                        break;
              }
              if(*buf=='N')
              {    printf("%s not extracted.\n",fix);
                   fseek(arc,hdr->size,1);
                   return;
              }
         }
    }

    if(!(f=fopen(fix,"w")))
    {    if(warn)
         {    printf("Cannot create %s\n",fix);
              nerrs++;
         }
         fseek(arc,hdr->size,1);
         return;
    }

    /* now unpack the file */

    unpack(arc,f,hdr);                 /* unpack file from archive */
    setstamp(f,hdr->date,hdr->time);   /* set the proper date/time stamp */
    fclose(f);                         /* all done writing to file */
}
