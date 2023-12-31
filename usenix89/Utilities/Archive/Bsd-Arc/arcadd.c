static char *RCSid = "$Header: arcadd.c,v 1.2 86/07/15 07:52:37 turner Exp $";

/*
 * $Log:	arcadd.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.2  86/07/15  07:52:37  turner
 * 
 * 
 * Revision 1.1  86/06/26  14:59:37  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCADD

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =3.39), created on $tag(
TED_DATE DB =02/05/86) at $tag(
TED_TIME DB =22:21:53))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to add files to an archive.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"

INT addarc(num,arg,move,update,fresh)      /* add files to archive */
INT num;                               /* number of arguments */
char *arg[];                           /* pointers to arguments */
INT move;                              /* true if moving file */
INT update;                            /* true if updating */
INT fresh;                             /* true if freshening */
{
    char *d, *dir();                   /* directory junk */
    char *NameList;	/* Any pointer.  Used to pass file names around */
    char buf[STRLEN];                 /* pathname buffer */
    char **path = NULL;                /* pointer to pointers to paths */
    char **name = NULL;                /* pointer to pointers to names */
 INT nfiles = 0;                    /* number of files in lists */
 INT notemp;                        /* true until a template works */
 INT nowork = 1;                    /* true until files are added */
    char *i, *rindex();                /* string indexing junk */
    char *malloc(), *realloc();         /* memory allocators */
 INT m, n;                          /* indices */
    unsigned INT coreleft();           /* remaining memory reporter */
    INT addbunch();

    if(num<1)                          /* if no files named */
    {    num = 1;                      /* then fake one */
         arg[0] = "*.*";               /* add everything */
    }

    for(n=0; n<num; n++)               /* for each template supplied */
    {
         strcpy(buf,arg[n]);           /* get ready to fix path */
         if(!(i=rindex(buf,'\\')))
              if(!(i=rindex(buf,'/')))
                   if(!(i=rindex(buf,':')))
                        i = buf-1;
         i++;                          /* pointer to where name goes */

         notemp = 1;                   /* reset files flag */
         NameList = (char *) malloc(sizeof(char *));
         for(d=dir(arg[n],0,NameList); d; d=dir(NULL,0,NameList))
         {    notemp = 0;              /* template is giving results */
              nfiles++;                /* add each matching file */
              path = (char **)realloc(path,nfiles*sizeof(char **));
              name = (char **)realloc(name,nfiles*sizeof(char **));
              strcpy(i,d);             /* put name in path */
              path[nfiles-1] = malloc(strlen(buf)+1);
              strcpy(path[nfiles-1],buf);
              name[nfiles-1] = d;      /* save name */

#if MSDOS
              if(coreleft()<5120)
              {    nfiles = addbunch(nfiles,path,name,move,update,fresh);
                   nowork = nowork && !nfiles;
                   while(nfiles)
                   {    free(path[--nfiles]);
                        free(name[nfiles]);
                   }
                   free(path); free(name);
                   path = name = NULL;
              }
#endif
         }
         if(notemp && warn)
              printf("No files match: %s\n",arg[n]);
    }

    if(nfiles)
    {    nfiles = addbunch(nfiles,path,name,move,update,fresh);
         nowork = nowork && !nfiles;
         while(nfiles)
         {    free(path[--nfiles]);
              free(name[nfiles]);
         }
         free(path); free(name);
    }

    if(nowork && warn)
         printf("No files were added.\n");
}

INT addbunch(nfiles,path,name,move,update,fresh) /* add a bunch of files */
INT nfiles;                            /* number of files to add */
char **path;                           /* pointers to pathnames */
char **name;                           /* pointers to filenames */
INT move;                              /* true if moving file */
INT update;                            /* true if updating */
INT fresh;                             /* true if freshening */
{
    char buf[STRLEN];                 /* pathname buffer */
 INT m, n;                          /* indices */
    char *d;                           /* swap pointer */
    struct heads hdr;                  /* file header data storage */
    INT addfile();
    INT izadir();		   /* used to weed-out directories */

    for(n=0; n<nfiles-1; n++)          /* sort the list of names */
    {    for(m=n+1; m<nfiles; m++)
         {    if(strcmp(name[n],name[m])>0)
              {    d = path[n];
                   path[n] = path[m];
                   path[m] = d;
                   d = name[n];
                   name[n] = name[m];
                   name[m] = d;
              }
         }
    }

    for(n=0; n<nfiles-1; )             /* consolidate the list of names */
    {    if(!strcmp(path[n],path[n+1]) /* if duplicate names */
         || !strcmp(path[n],arcname)   /* or this archive */
         || izadir(path[n])            /* or directory */
         || !strcmp(path[n],newname)   /* or the new version */
         || !strcmp(path[n],bakname))  /* or its backup */
         {    free(path[n]);           /* then forget the file */
              free(name[n]);
              for(m=n; m<nfiles-1; m++)
              {    path[m] = path[m+1];
                   name[m] = name[m+1];
              }
              nfiles--;
         }
         else n++;                     /* else test the next one */
    }

    if(!strcmp(path[n],arcname)        /* special check for last file */
    || !strcmp(path[n],newname)        /* courtesy of Rick Moore */
    || izadir(path[n])
    || !strcmp(path[n],bakname))
    {    free(path[n]);
         free(name[n]);
         nfiles--;
    }

    if(!nfiles)                        /* make sure we got some */
         return 0;

    for(n=0; n<nfiles-1; n++)          /* watch out for duplicate names */
         if(!strcmp(name[n],name[n+1]))
              abort("Duplicate filenames:\n  %s\n  %s",path[n],path[n+1]);

    openarc(1);                        /* open archive for changes */

    for(n=0; n<nfiles; n++)            /* add each file in the list */
         addfile(path[n],name[n],update,fresh);

    /* now we must copy over all files that follow our additions */

    while(readhdr(&hdr,arc))           /* while more entries to copy */
    {    writehdr(&hdr,new);
         filecopy(arc,new,hdr.size);
    }
    hdrver = 0;                        /* archive EOF type */
    writehdr(&hdr,new);                /* write out our end marker */
    closearc(1);                       /* close archive after changes */

    if(move)                           /* if this was a move */
    {    for(n=0; n<nfiles; n++)       /* then delete each file added */
         {    if(unlink(path[n]) && warn)
              {    printf("Cannot unsave %s\n",path[n]);
                   nerrs++;
              }
         }
    }

    return nfiles;                     /* say how many were added */
}

static INT addfile(path,name,update,fresh) /* add named file to archive */
char *path;                            /* path name of file to add */
char *name;                            /* name of file to add */
INT update;                            /* true if updating */
INT fresh;                             /* true if freshening */
{
    struct heads nhdr;                 /* data regarding the new file */
    struct heads ohdr;                 /* data regarding an old file */
    FILE *f, *fopen();                 /* file to add, opener */
    long starts, ftell();              /* file locations */
 INT c;                             /* one char of file */
 INT upd = 0;                       /* true if replacing an entry */

    if(!(f=fopen(path,"r")))
    {    if(warn)
         {    printf("Cannot read file: %s\n",path);
              nerrs++;
         }
         return;
    }

    strcpy(nhdr.name,name);            /* save name */
    nhdr.size = 0;                     /* clear out size storage */
    nhdr.crc = 0;                      /* clear out CRC check storage */
    getstamp(f,&nhdr.date,&nhdr.time);

    /* position archive to spot for new file */

    if(arc)                            /* if adding to existing archive */
    {    starts = ftell(arc);          /* where are we? */
         while(readhdr(&ohdr,arc))     /* while more files to check */
         {    if(!strcmp(ohdr.name,nhdr.name))
              {    upd = 1;            /* replace existing entry */
                   if(update || fresh) /* if updating or freshening */
                   {    if(nhdr.date<ohdr.date
                        || (nhdr.date==ohdr.date && nhdr.time<=ohdr.time))
                        {    fseek(arc,starts,0);
                             fclose(f);
                             return;   /* skip if not newer */
                        }
                   }
              }

              if(strcmp(ohdr.name,nhdr.name)>=0)
                   break;              /* found our spot */

              writehdr(&ohdr,new);     /* entry preceeds update; keep it */
              filecopy(arc,new,ohdr.size);
              starts = ftell(arc);     /* now where are we? */
         }

         if(upd)                       /* if an update */
         {    if(note)
                 { printf("Updating file: %-12s  ",name); fflush(stdout);}
              fseek(arc,ohdr.size,1);
         }
         else if(fresh)                /* else if freshening */
         {    fseek(arc,starts,0);     /* then do not add files */
              fclose(f);
              return;
         }
         else                          /* else adding a new file */
         {    if(note)
                 { printf("Adding file:   %-12s  ",name); fflush(stdout);}
              fseek(arc,starts,0);     /* reset for next time */
         }
    }

    else                               /* no existing archive */
    {    if(fresh)                     /* cannot freshen nothing */
         {    fclose(f);
              return;
         }
         else if(note)                 /* else adding a file */
            { printf("Adding file:   %-12s  ",name); fflush(stdout);}
    }

    starts = ftell(new);               /* note where header goes */
    hdrver = ARCVER;                  /* anything but end marker */
    writehdr(&nhdr,new);               /* write out header skeleton */
    pack(f,new,&nhdr);                 /* pack file into archive */
    fseek(new,starts,0);               /* move back to header skeleton */
    writehdr(&nhdr,new);               /* write out real header */
    fseek(new,nhdr.size,1);            /* skip over data to next header */
    fclose(f);                         /* all done with the file */
}
