/*  ARC - Archive utility - ARCDOS

System V Version 1.0 based upon:
    Version 1.43, created on 11/09/85 at 22:24:44

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains certain DOS level routines that assist
         in doing fancy things with an archive, primarily reading and
         setting the date and time last modified.

         These are, by nature, system dependant functions.  But they are
         also, by nature, very expendable.
*/
#include "arc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

INT getstamp(f,date,time)              /* get a file's date/time stamp */
FILE *f;                               /* file to get stamp from */
unsigned INT *date, *time;             /* storage for the stamp */
{
    struct stat buf;
    struct tm *tmbuf;

    fstat(fileno(f),&buf);
    tmbuf=localtime(&buf.st_mtime);
    
    *date = ((tmbuf->tm_year-80)<<9) + ((tmbuf->tm_mon+1)<<5) + tmbuf->tm_mday;
    *time = (tmbuf->tm_hour<<11) + (tmbuf->tm_min<<5) + (tmbuf->tm_sec>>1);;
}

INT setstamp(f,date,time)              /* set a file's date/time stamp */
FILE *f;                               /* file to set stamp on */
unsigned INT date, time;               /* desired date, time */
{
}
