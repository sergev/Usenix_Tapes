static char *RCSid = "$Header: arcdos.c,v 1.2 86/07/15 07:53:02 turner Exp $";

/*
 * $Log:	arcdos.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.2  86/07/15  07:53:02  turner
 * 
 * 
 * Revision 1.1  86/06/26  15:00:15  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCDOS

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =1.43), created on $tag(
TED_DATE DB =11/09/85) at $tag(
TED_TIME DB =22:24:44))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains certain DOS level routines that assist
         in doing fancy things with an archive, primarily reading and
         setting the date and time last modified.

         These are, by nature, system dependant functions.  But they are
         also, by nature, very expendable.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"
#if MSDOS
#include "fileio2.h"                   /* needed for filehand */
#endif
#if BSD
#include <sys/types.h>
#include <sys/stat.h>
#endif

INT getstamp(f,date,time)                  /* get a file's date/time stamp */
FILE *f;                               /* file to get stamp from */
unsigned INT *date, *time;             /* storage for the stamp */
{
#if MSDOS
    struct {INT ax,bx,cx,dx,si,di,ds,es;} reg;

    reg.ax = 0x5700;                   /* get date/time */
    reg.bx = filehand(f);              /* file handle */
    if(sysint21(&reg,&reg)&1)          /* DOS call */
         printf("Get timestamp fail (%d)\n",reg.ax);

    *date = reg.dx;                    /* save date/time */
    *time = reg.cx;
#endif
#if BSD
    struct stat *buf;
    int day,hr,min,sec,yr,imon;
    static char mon[4],*mons[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul",
					"Aug","Sep","Oct","Nov","Dec"};

    buf = (struct stat *)malloc(sizeof(struct stat));
    fstat(f->_file,buf); 
/*
 * assume the UGLY ibm format for date and time
 */
    sscanf(ctime(&(buf->st_mtime)),"%*4s%3s%d%d:%d:%d%d"
	,mon,&day,&hr,&min,&sec,&yr);
    for(imon = 0; imon < 12 && strcmp(mon,mons[imon]); imon++);

    *date = (unsigned INT)(((yr-1980)<<9)+((imon+1)<<5)+day);
    *time = (unsigned INT)((hr<<11)+(min<<5)+sec/2);
#endif
}

INT setstamp(f,date,time)                  /* set a file's date/time stamp */
FILE *f;                               /* file to set stamp on */
unsigned INT date, time;               /* desired date, time */
{
#if MSDOS
    struct {INT ax,bx,cx,dx,si,di,ds,es;} reg;

    fflush(f);                         /* force any pending output */

    reg.ax = 0x5701;                   /* set date/time */
    reg.bx = filehand(f);              /* file handle */
    reg.cx = time;                     /* desired time */
    reg.dx = date;                     /* desired date */
    if(sysint21(&reg,&reg)&1)          /* DOS call */
         printf("Set timestamp fail (%d)\n",reg.ax);
#endif
}

static INT filehand(stream)            /* find handle on a file */
struct bufstr *stream;                 /* file to grab onto */
{
#if MSDOS
    return stream->bufhand;            /* return DOS 2.0 file handle */
#endif
}

INT izadir(filename)			/* Is filename a directory? */
char *filename;
{
#if MSDOS
    return 0;
#else
struct stat buf;

    if (stat(filename,&buf)!=0) return 0;   /* Ignore if stat fails since we */
    else return (buf.st_mode & S_IFDIR);    /* trap for bad file elsewhere.  */

#endif
}
