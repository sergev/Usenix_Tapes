/* dataman - simple data manager for reading data from text files */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 09:14:15 by jimmc (Jim McBeath) */
/* last edit 11-Sept-85 22:31:00 by tlr (Terry L. Ridder) */

/* This module is a very simple data manager which allows a simple
   interface to a text file.  The text file is the database.  This
   module has routines to read that database (it is assumed that
   the writing is done by another program, e.g. a text editor).
   The format of the data file is as follows:

   A file consists of a sequence of records.  All records are ascii
   text.  Records are separated by blank lines.  Lines within each
   record contain the data for that record.  The first line of a
   record must begin with an integer.  This integer is the index
   number of that record, and is used to reference the record.  All
   other text on the line with the index number is ignored, so
   can be used as a comment line.

   The remaining lines in a record are data items for that record.
   A data item consists of a key and a payload.  The key is any
   string of printing characters followed by a colon (i.e. the
   key can't contain a colon!).  All remaining text on the line,
   after the colon, is the payload for that data item.

   Both index numbers and key strings must be unique; non-unique
   items will not be referenceable.  Index numbers need not be in
   any order.  Key strings should be short to increase speed.
   (Note that this is not designed to be a particularly fast
   system anyway!)
*/

extern char *malloc();

/*..........*/

#include "geneal.h"

struct toplevel *initIndex();

#ifdef vms
#define index strchr
#endif

int dataDebug  = 0;			/* set this flag to give debug output */
int dataStatus = 0;			/* status after most recent operation */
char *dataErrStrs[] = {
    "successful data operation",	/* 0 */
    "can't open data file",		/* 1 */
    "no more memory",			/* 2 */
    "can't start an index table",	/* 3 */
    "invalid pointer to getData",	/* 4 */
    "no file open in getData",		/* 5 */
    "no index table in getData",	/* 6 */
    "no such index number",		/* 7 */
    "key not found",			/* 8 */
    };
#define ERET(n) { dataStatus = n; return 0; }

/*..........*/

struct dpoint *				/* a pointer to our internal struct */
					/* returns 0 on error */
initDataFile(fn)			/* init the data file to be used */
char *fn;				/* the filename to look up */
{
FILE *fp;
struct dpoint *pp;
struct toplevel *qq;
int iflag,n;
char linebuf[256];

fp = fopen(fn, "r");			/* get his data file */
if (!fp) 
{
    ERET(1)				/* can't do anything if no file */
}
pp = (struct dpoint *)malloc(sizeof(struct dpoint));
if (!pp) 
{					/* if no memory for us */
    fclose(fp);				/* dump the file */
    ERET(2)				/* error return */
}
pp->ff = fp;			/* put file pointer into our data block */
qq = initIndex();			/* start up an index table */
if (qq == 0) 
{					/* if can't start up an index table */
    fclose(fp);
    free((char *)pp);
    ERET(3)
}
pp->xx = qq;				/* save pointer to index table */
if (dataDebug) 
{
    printf("initDataFile: index table is at %X\n", qq);
}
iflag = 1;				/* note next line can be index line */
while (!feof(fp)) 
{					/* scan through the file */
    fgets(linebuf, 255, fp);		/* read a line */
    if (iflag) 
    {					/* if we can look for an index line */
	if (sscanf(linebuf, "%d", &n) == 1) 
	{				/* and if we got a number */
	    if (dataDebug) 
	    {
		printf("initDataFile: index %d at loc %d\n",
				n, ftell(fp));
	    }
	    setIndex(qq, n, (int)ftell(fp)); /* remember start of next line */
	    iflag = 0;			/* have to get a blank line now */
	}
    }
    else 
    {					/* see if this is a blank line */
	if (linebuf[0] != '\n') 
	{
	    iflag++;			/* note blank line */
	}
    }
}
dataStatus = 0;
return pp;				/* return pointer to our structure */
}

/*..........*/

char *				/* returns a pointer to the data string */
				/*  return NULL if no data */
getData(pp, indexn, key)	/* get a data item */
struct dpoint *pp;		/* pointer to our structure */
int indexn;			/* the index of the record of interest */
char *key;				/* the key string */
{
int l;
char *cc,*index();
static char linebuf[256];

if (!pp) 
{
    ERET(4)				/* check for valid pointer */
}
if (!(pp->ff)) 
{
    ERET(5)				/* error return if no file open */
}
if (!(pp->xx)) 
{
    ERET(6)				/* error if no index table pointer */
}
l = getIndex(pp->xx, indexn);		/* get the lseek value for that index */
if (l == 0) 
{
    ERET(7)			/* error if no lseek value for that index */
}
fseek(pp->ff, (long)l, 0);		/* seek to the start of that line */
while (!feof(pp->ff)) 
{					/* read lines until eof (or blank) */
    fgets(linebuf, 255, pp->ff);	/* read in a line */
    if (linebuf[0] == '\n') 
    {
	break;				/* stop on a blank line */
    }
    cc = index(linebuf, ':');		/* search for the colon */
    if (!cc) 
    {
	continue;		/* if no colon, take it as a comment line */
    }
    *cc = 0;				/* replace colon with null */
    if (strcmp(linebuf, key) == 0) 
    {
	cc++;				/* point to start of payload */
	*(cc + strlen(cc) - 1) = 0;	/* remove \n from the end */
	dataStatus = 0;
	return cc;			/* return pointer to payload */
    }
}				/* continue - read next line and compare */
ERET(8)					/* eof or blank line, key not found */
}

/* end */
