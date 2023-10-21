static char *RCSid = "$Header: arcrun.c,v 1.2 86/07/15 07:53:55 turner Exp $";

/*
 * $Log:	arcrun.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.2  86/07/15  07:53:55  turner
 * 
 * 
 * Revision 1.1  86/06/26  15:00:40  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCRUN

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =1.17), created on $tag(
TED_DATE DB =02/03/86) at $tag(
TED_TIME DB =22:59:06))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to "run" a file
         which is stored in an archive.  At present, all we really do
         is (a) extract a temporary file, (b) give its name as a system
         command, and then (c) delete the file.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"

INT runarc(num,arg)                        /* run file from archive */
INT num;                               /* number of arguments */
char *arg[];                           /* pointers to arguments */
{
    struct heads hdr;                  /* file header */
 INT run;                           /* true to run current file */
 INT did[MAXARG];                  /* true when argument was used */
 INT n;                             /* index */
    char *makefnam();                  /* filename fixer */
    char buf[STRLEN];                 /* filename buffer */
    FILE *fopen();                     /* file opener */
    INT runfile();

    for(n=0; n<num; n++)               /* for each argument */
         did[n] = 0;                   /* reset usage flag */
    rempath(num,arg);                  /* strip off paths */

    openarc(0);                        /* open archive for reading */

    if(num)                            /* if files were named */
    {    while(readhdr(&hdr,arc))      /* while more files to check */
         {    run = 0;                 /* reset run flag */
              for(n=0; n<num; n++)     /* for each template given */
              {    if(match(hdr.name,makefnam(arg[n],".*",buf)))
                   {    run = 1;       /* turn on run flag */
                        did[n] = 1;    /* turn on usage flag */
                        break;         /* stop looking */
                   }
              }

              if(run)                  /* if running this one */
                   runfile(&hdr);      /* then do it */
              else                     /* else just skip it */
                   fseek(arc,hdr.size,1);
         }
    }

    else while(readhdr(&hdr,arc))      /* else run all files */
         runfile(&hdr);

    closearc(0);                       /* close archive after changes */

    if(note)
    {    for(n=0; n<num; n++)          /* report unused args */
         {    if(!did[n])
              {    printf("File not found: %s\n",arg[n]);
                   nerrs++;
              }
         }
    }
}

static INT runfile(hdr)                    /* run a file */
struct heads *hdr;                     /* pointer to header data */
{
    FILE *tmp, *fopen();               /* temporary file */
    char buf[STRLEN], *makefnam();    /* temp file name, fixer */
    char sys[STRLEN];                 /* invocation command buffer */
    char *dir, *gcdir();               /* directory stuff */

/*  Replaced $ARCTEMP as the "system" call didn't approve.  */
/*  makefnam("$ARCTEMP",hdr->name,buf); */
    sprintf(buf,"%s.RUN",arctemp);

/*  if(!strcmp(buf,"$ARCTEMP.BAS"))
 *       strcpy(sys,"BASICA $ARCTEMP");
 *
 *  else if(!strcmp(buf,"$ARCTEMP.BAT")
 *       || !strcmp(buf,"$ARCTEMP.COM")
 *       || !strcmp(buf,"$ARCTEMP.EXE"))
 *       strcpy(sys,"$ARCTEMP");
 *
 *  else
 *  {    if(warn)
 *       {    printf("File %s is not a .BAS, .BAT, .COM, or .EXE\n",
 *                 hdr->name);
 *            nerrs++;
 *       }
 *       fseek(arc,hdr->size,1);
 *       return;
 *  }
 */

    if(warn)
         if(tmp=fopen(buf,"r"))
              abort("Temporary file %s already exists",buf);
    if(!(tmp=fopen(buf,"w")))
         abort("Unable to create temporary file %s",buf);

    if(note)
         printf("Invoking file: %s\n",hdr->name);

    dir = gcdir("");                   /* see where we are */
    unpack(arc,tmp,hdr);               /* unpack the entry */
    fclose(tmp);                       /* release the file */
    chmod(buf,"700");                  /* make it exec'able */
    system(buf);                       /* try to invoke it */
    chdir(dir); free(dir);             /* return to whence we started */
    if(unlink(buf) && warn)
    {    printf("Cannot unsave temporary file %s\n",buf);
         nerrs++;
    }
}
