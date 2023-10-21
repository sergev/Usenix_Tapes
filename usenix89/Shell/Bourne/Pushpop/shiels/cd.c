#include	<stdio.h>
#include	<dos.h>

cd( name )
register char	*name;
{
	/*	Change the current directory to the indicated name.
	 *	Log in a new disk if necessary. This routine is
	 *	a bit more sophisticated than DOS itself, in that
	 *	it checks if a disk exists before trying to
	 *	log it on. This checking is done using the
	 *	"Get diskette status" service of BIOS interrupt 0x13.
	 *	Get to the current directory on another disk by saying
	 *	"cd x:" Get to another directory on another disk by
	 *	saying "cd x:/dir/subdir/etc".
	 */

	if( name[1] == ':')
	{
		bdos( 0x0E, toupper(*name) - 'A', 0);
		name += 2;
	}

	if( *name  &&  chdir( name ) < 0 )
		return( -1 );
	return( 0 );
}

