#include	<stdio.h>
#include	<signal.h>


static process_sigint()
{
	/*	Buffers aren't flushed when a ^C is encountered by.
	 *	dos. Since exit() does do a buffer flush so re arange
	 *	to call exit on intercepting a ^C
	 */

	exit( 1 );
}


ctlc()
{
	/*	Make control-C behave intellegently
	*/

	signal(SIGINT, process_sigint );
}
