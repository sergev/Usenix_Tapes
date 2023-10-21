
/* treeprint.c */

/* author:  Rob Peck  5/13/86 */

/* part of "fastdir" command  */


#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "exec/memory.h"
#define SPRINTF 1
#define FPRINTF 0

extern int leftright;
extern char *blanks;
extern char linebuffer[];
extern int (*fprintf)();

struct tnode    /* for quick alphabetizing */
{
        struct tnode *left;
        struct tnode *right;
        char name[36];  /* space to store the name (30 + ' (dir)') max */
        char dirflag;   /* nonzero if it is a directory */
};
/* This SPECIAL VERSION of strcpy does NOT copy the trailing null */

mystrcpy( to, from )
register char *to, *from;
{
    char *leadingfrom;
    leadingfrom = from + 1;
    do {
        *to++ = *from++;
    } while( *leadingfrom++ );
}


/* treeprint -- recursive output, to stdout or to a file, from the 
 * 		linked node list that tree() creates.  tree() allocates
 * 		memory;  treeprint() deallocates it as it runs
 */

treeprint(t,tfh)		/* print the tree recursively */
struct tnode *t;
struct FileHandle *tfh; 	/* if theres a file open, use it */
{
	int actual;
	if(t->right != NULL) treeprint(t->right,tfh);
	if(leftright == 0)
	{
	    mystrcpy(&linebuffer[2], blanks);	/* blank the string */
	    mystrcpy(&linebuffer[2], &(t->name[0])); /* install the name */
 	    mystrcpy(&linebuffer[35],blanks);	/* blank out rest. */
	    leftright = 1;
	}
	else
	{
	    mystrcpy(&linebuffer[35], &(t->name[0])); /* install the name */
	    leftright = 0;
	    if(tfh == 0)
	    {
	        printf("%ls\n", linebuffer);
	    }
	    else
	    {
	        actual = Write(tfh, linebuffer, 66);
		actual = Write(tfh, "\n", 1);
	    }
	}
	if(t->left != NULL) treeprint(t->left,tfh);
	FreeMem(t, sizeof(struct tnode));
	return(0);
}

dotreeprint(root, tfh, ds)
    struct tnode *root;
    struct FileHandle *tfh;
    LONG *ds;			/* points to the datestamp */
{
    char dummy[50];		/* When call for FPRINTF, dummy is not used. */
    
    if(tfh)
    {
	DateToAscii(FPRINTF, dummy, ds, tfh);
    }	
    treeprint(root, tfh);    /* call does all except the final line */
    if(leftright == 1) 
    {
	if(tfh)	/* if there is a file open, thats where the output goes. */
	{
	    Write(tfh, linebuffer, 66);
	    Write(tfh, "\n", 1);
	}
	else	/* otherwise print it to stdout */
	{
	    printf("%ls\n",linebuffer);
	}
    }
}

/* This routine can accept either "FPRINTF", which expects whereto
 * to be a file handle, or "SPRINTF", which expects whereto to be the
 * address of a string.
 */

DateToAscii(routine, whereto, ds, tfh)
	LONG routine;		/* which routine to use   */
	char *whereto;
	struct FileHandle *tfh;

	LONG *ds;		/* pointer to a datestamp */
{
	char dateout[50];	/* only need about 41, but this is ok */
	int actual;
	LONG n ;	/* number of days past 1/1/78 */
   	int m, d, y ;	/* month, day, year */
	int h, mn, sec; /* hours, minutes, seconds */
	n = ds[0] - 2251 ;
   	y = (4 * n + 3) / 1461 ;
   	n -= 1461 * y / 4 ;
   	y += 84 ;		/* was 1984 */
   	m = (5 * n + 2) / 153 ;
   	d = n - (153 * m + 2) / 5 + 1 ;
   	m += 3 ;
   	if (m > 12) 
	{
      		y++ ;
      		m -= 12 ;
   	}
	h = ds[1] / 60;	/* 60 minutes per hour */
	mn = ds[1] - h * 60; /* remainder is the minutes */
	sec = ds[2] / TICKS_PER_SECOND;
	switch(routine)
	{
	   case SPRINTF:
		sprintf(whereto, 
	"<dir> Creation Date: %02ld/%02ld/%02ld %02ld:%02ld:%02ld.\n",m,d,y,h,mn,sec);
	   	break;
	   case FPRINTF:
		sprintf(dateout, 
	"<dir> Creation Date: %02ld/%02ld/%02ld %02ld:%02ld:%02ld.\n",m,d,y,h,mn,sec);
		actual = Write(tfh, dateout, 41);
	   	break;
	   default:
		break;
	}	
	return(0);
}


