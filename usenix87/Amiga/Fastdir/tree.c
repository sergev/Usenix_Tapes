

/* tree.c */

/* Author:  Rob Peck  5/13/86 */

/* part of "fastdir" command */



#include "exec/types.h"
#include "libraries/dos.h"
#include "exec/memory.h"


#define MAX(x,y) (x > y ? x : y)

#define FASTDIR 1

#ifdef FASTDIR
extern struct FileInfoBlock *m;
#endif FASTDIR

extern int maxlen;

struct tnode    /* for quick alphabetizing */
{
        struct tnode *left;
        struct tnode *right;
        char name[36];  /* space to store the name (30 + ' (dir)') max */
        char dirflag;   /* nonzero if it is a directory */
};


/* tolower(c) --  Convert characters to lower case              */

int tolower( b )
        int b;
{
        if( (b >= 'A') && (b <= 'Z') )
                return(b - 'A' + 'a');
        else
                return( b );
}

/* tree -- Install a new named node at or below the one now pointed to
 * 	   RECURSIVELY CALLS ITSELF.... be careful not to run out of
 * 	   stack space if the strings nest too deeply.			
 */

struct tnode *tree(t, name)
	struct tnode *t;
	char *name;
{
	int cond;	/* Condition code from return */
	struct tnode *allocated;

	if(t == NULL)	/* If prior invocation has reached a null pointer,
			 * a new node must be allocated.
			 */
	{
	    t = (struct tnode *)
                AllocMem(sizeof(struct tnode),MEMF_CLEAR);

	    if(t == 0)
	    {
		/* NOT ENOUGH MEMORY TO FINISH THE TREE */
		/* Make sure to STOP!			*/

		return(0);	
	    }
    	    strcpy(  &(t->name[0]), name );
#ifdef FASTDIR
    	    if (m->fib_DirEntryType > 0)
    	    {
	    	strcat( &(t->name[0]), " (dir)" );
    	    }
#endif FASTDIR
    	    maxlen = MAX(maxlen, strlen( &(t->name[0])));

	    return(t);
	}		
	/* compare two strings and decide where to install the new one */

	cond = compLC(&(t->name[0]), name );

	if(cond < 0)
	{
	    if (t->left != NULL) /* search for a NULL entry */
	    { 
		return(tree(t->left, name)); 
	    }
	    else		 /* try to install at the null entry found */
	    { 
		allocated = tree(0, name);  
		/* If allocated == 0, then it passes a NULL all the
		 * way back to the caller, telling him the system has
		 * just run out of memory trying to build the chains.
		 * So it should consider the chain complete and try
		 * to output it, thereby freeing the memory along the way.
		 */  
		t->left = allocated;
		return(allocated);
	    }
	}
	else if(cond > 0)
	{
	    if (t->right != NULL) /* search for a NULL entry */
	    { 
		return(tree(t->right, name)); 
	    }
	    else		 /* try to install at the null entry found */
	    { 
		allocated = tree(0, name);
		t->right = allocated;
		return(allocated);
	    }
	}
	else 
	{
	/* If it EVER gets here, means entry of a duplicate item into tree.
	 * Is treated as an error.  Upper and lower case are the same and only
	 * get listed once.  Original application was for 'fastdir' and  
	 * AmigaDOS cannot install duplicate file names into a single 
	 * directory.
	 */
	return(0);
	}
}
 
strcpy( to, from )
register char *to, *from;
{
    do { 
        *to++ = *from;
    } while( *from++ );
}


/* compLC --    Compare two strings, ignoring case.
 *              returns -1 if s<t, 1 if s>t, 0 if s=t.
 */

int compLC(s,t)
register char *s, *t;
{
register char test;
    while(1)
    {
        test = tolower(*s)-tolower(*t); /* compare two characters */

        if (test < 0)
        {
           return(-1);        /* first less than second */
        }
        else
        {
            if (test > 0)
            {
                return(1);   /* first greater than second */
            }
            else             /* first equals second, so far */
            {
                if( *s == '\0')  /* if nulls match also, strings are equal */
                {
                    return(0);
                }
            t++;
            s++;
            }
        }
    }
}


