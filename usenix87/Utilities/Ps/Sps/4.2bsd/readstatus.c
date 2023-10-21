# include       "sps.h"
# include       <h/text.h>

/* READSTATUS - Reads the kernel memory for current processes and texts */
readstatus ( process, text )

register struct process         *process ;
struct text                     *text ;

{
	register struct proc    *p ;
	register struct proc    *p0 ;
	register struct process *pr ;
	extern struct info      Info ;
	extern int              Flkmem ;
	char                    *getcore() ;

	/* Read current text information */
	memseek( Flkmem, (long)Info.i_text0 ) ;
	if ( read( Flkmem, (char*)text, Info.i_ntext * sizeof( struct text ) )
	!= Info.i_ntext * sizeof( struct text ) )
		prexit( "sps - Can't read system text table\n" ) ;
	/* Read current process information */
	p0 = (struct proc*)getcore( sizeof( struct proc )*Info.i_nproc ) ;
	memseek( Flkmem, (long)Info.i_proc0 ) ;
	if ( read( Flkmem, (char*)p0, Info.i_nproc * sizeof( struct proc ) )
	!= Info.i_nproc * sizeof( struct proc ) )
		prexit( "sps - Can't read system process table\n" ) ;
	/* Copy process information into our own array */
	for ( p = p0, pr = process ; pr < &process[ Info.i_nproc ] ; p++, pr++ )
		pr->pr_p = *p ;
	free( (char*)p0 ) ;
}
