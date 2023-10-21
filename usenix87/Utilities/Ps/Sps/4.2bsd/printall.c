# include       <stdio.h>
# include       "sps.h"

/* PRINTALL - Recursively print the process tree. */
printall ( p, md )

register struct process         *p ;
register int                    md ;

{
	while ( p )
	{       /* Print this process */
		printproc( p, md ) ;    
		(void)fflush( stdout ) ;
		/* Print child processes */
		printall( p->pr_child, md+1 ) ;
		/* Print brother processes */
		p = p->pr_sibling ;     
	}
}
