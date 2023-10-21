/*
$define(arc,$ifdef(xarc,off,on))#      macro switch for ARC only code
$define(xarc,$ifdef(xarc,on,off))#     macro switch for XARC only code
 */
/*  ARC - Archive utility - ARCEXT

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =2.18), created on $tag(
TED_DATE DB =02/03/86) at $tag(
TED_TIME DB =22:55:19))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to extract files from
         an archive.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"
#if ARC					/* $emit($arc)# */
INT extarc(num,arg,prt)                    /* extract files from archive */
INT num;                               /* number of arguments */
char *arg[];                           /* pointers to arguments */
INT prt;                               /* true if printing */
#endif					/* $emit($xarc)# */
#if XARC
INT extarc()                               /* extract files from archive */
#endif					/* $emit(on)# */
{
    struct heads hdr;                  /* file header */
#if ARC					/* $emit($arc)# */
 INT save;                          /* true to save current file */
 INT did[MAXARG];                  /* true when argument was used */
    char *i, *rindex();                /* string index */
    char **name, *malloc();             /* name pointer list, allocator */
 INT n;                             /* index */
    INT extfile();

#if MSDOS
    name = malloc(num*sizeof(char *));  /* get storage for name pointers */
#endif
#if BSD
    name = (char **)malloc(num*sizeof(char *));  /* get storage for name pointers */
#endif

    for(n=0; n<num; n++)               /* for each argument */
    {    did[n] = 0;                   /* reset usage flag */
         if(!(i=rindex(arg[n],'\\')))  /* find start of name */
              if(!(i=rindex(arg[n],'/')))
                   if(!(i=rindex(arg[n],':')))
                        i = arg[n]-1;
         name[n] = i+1;
    }

#endif					/* $emit(on)# */

    openarc(0);                        /* open archive for reading */

#if ARC					/* $emit($arc)# */
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
#endif					/* $emit($xarc)# */
#if XARC
    while(readhdr(&hdr,arc))           /* extract all files */
         extfile(&hdr);
#endif					/* $emit(on)# */

    closearc(0);                       /* close archive after reading */
#if ARC					/* $emit($arc)# */

    if(note)
    {    for(n=0; n<num; n++)          /* report unused args */
         {    if(!did[n])
              {    printf("File not found: %s\n",arg[n]);
                   nerrs++;
              }
         }
    }

    free(name);
#endif					/* $emit(on)# */
}

#if ARC					/* $emit($arc)# */
static INT extfile(hdr,path,prt)           /* extract a file */
struct heads *hdr;                     /* pointer to header data */
char *path;                            /* pointer to path name */
INT prt;                               /* true if printing */
#endif					/* $emit($xarc)# */
#if XARC
static INT extfile(hdr)                    /* extract a file */
#endif					/* $emit(on)# */
			/* $define(use,$ife($arc,on,fix,hdr->name))# */
#if ARC
#define USE fix
#else
#define USE hdr->name
#endif

{
    FILE *f, *fopen();                 /* extracted file, opener */
    char buf[STRLEN];                 /* input buffer */
#if ARC					/* $emit($arc)# */
    char fix[STRLEN];                 /* fixed name buffer */
    char *i, *rindex();                /* string index */

    if(prt)                            /* printing is much easier */
    {    unpack(arc,stdout,hdr);       /* unpack file from archive */
         printf("\f");                 /* eject the form */
         return;                       /* see? I told you! */
    }

    strcpy(fix,path);                  /* note path name template */
    if(!(i=rindex(fix,'\\')))          /* find start of name */
         if(!(i=rindex(fix,'/')))
              if(!(i=rindex(fix,':')))
                   i = fix-1;
    strcpy(i+1,hdr->name);             /* replace template with name */
#endif					/* $emit(on)# */

    if(note)
         printf("Extracting file: %s\n",USE);

    if(warn)
    {    if(f=fopen(USE,"r"))        /* see if it exists */
         {    fclose(f);
              printf("WARNING: File %s already exists!",USE);
              while(1)
              {    printf("  Overwrite it (y/n)? ");
                   fgets(buf,STRLEN,stdin);
                   *buf = toupper(*buf);
                   if(*buf=='Y' || *buf=='N')
                        break;
              }
              if(*buf=='N')
              {    printf("%s not extracted.\n",USE);
                   fseek(arc,hdr->size,1);
                   return;
              }
         }
    }

    if(!(f=fopen(USE,"w")))
    {    if(warn)
         {    printf("Cannot create %s\n",USE);
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
