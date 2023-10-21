/*      Comment out the following
#include        "empdef.h"
#include        <stdio.h>
#include        <varargs.h>
#include        <values.h>
*/

char    fmtbuf[256];

/*
        All these routines have been replaced with printf() and sprintf().
        The only thing that is still needed is the buffer, fmtbuf[], to
        hold the formatted output string.  This stuff is left here for
        interested souls trying to get a historical perspective on things.
*/
/*      Comment out the following
pr(string)
char    *string;
{
        write(1, string, fmtlen);
        if( proto ) {
                write(proto, string, fmtlen);
        }
}

prfmt(fmtbuf,string)
char    *string;
{
        register char   *cp;

        cp = string;
        while( *cp++ ) continue;
        fmtlen = cp - string - 1;
        pr(string);
}

char *
fmt(fmtbuf,format, va_alist)
char    *format;
va_dcl
{
        FILE    siop;
        va_list ap;

        siop._cnt= MAXINT;
        siop._base = siop._ptr = (unsigned char *)fmtbuf;
        siop._flag = _IOWRT;
        siop._file = _NFILE;
        va_start(ap);
        _doprnt(format, ap, &siop);
        va_end(ap);
        *siop._ptr = '\0';
        fmtlen = siop._ptr - (unsigned char *)fmtbuf;
        return(fmtbuf);
}
*/
