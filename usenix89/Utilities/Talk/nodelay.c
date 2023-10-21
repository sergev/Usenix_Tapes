/*
**	You must have fcntl or the equivalent to make this work. This
**	is the heart of the program.
*/
#include <sys/fcntl.h>			/* used for setting ONDELAY */

nodelay(win,flag)
char *win;    	/* it is ignored any way */
int flag;
	
{
int arg;

	arg=fcntl(0,F_GETFL,arg);
	
	if(flag)
		arg |= O_NDELAY;
	else
		arg &= ~O_NDELAY;

	fcntl(0,F_SETFL,arg);
}


