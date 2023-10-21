#include "hd.h"

/* Open a showfile */
/* Open with rwmode of "r" or "w" just like fopen */
/* Smode GREPMODE opens for grep;  Smode MAKEMODE opens for Make;  */

FILE *showopen (rwmode, smode) char *rwmode; int smode; 
{

    char buf[STRMAX + 20];
    FILE *tfile;
    register char *fname;

    fname = smode == GREPMODE ? GREPOUT : MAKERROR;
    tfile = fopen (fname, rwmode);
    if (tfile == NULL && smode == GREPMODE) 
    {
	if (access (fname, 0) == 0) 
	{
	    myperror (fname);
	    return NULL;
	}
	strcpy (buf, HOME);
	strcat (buf, "/");
	strcat (buf, fname);
	tfile = fopen (buf, rwmode);
    }
    if (tfile == NULL) 
    {
	myperror (fname);
    }
    return tfile;
}
