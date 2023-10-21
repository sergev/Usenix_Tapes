/*
 * call ioctl to set unit 1 (f77) to 2400 baud
 *
 */

#include <sgtty.h>

ioset_()
{
	struct sgttyb st;
	if( isatty(3) ) {
		gtty( 3, &st);
		st.sg_ispeed = B2400;
		st.sg_ospeed = B2400;
		stty( 3, &st);
	}
	return( 0 );
}
