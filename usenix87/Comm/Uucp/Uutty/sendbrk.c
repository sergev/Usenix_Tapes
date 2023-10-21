#include "uutty.h"
/* 
** If fd is a terminal-type device, and ioctl(-,TCSBRK,0) works,
** this will send a break signal down the line, which may (or may
** not) get the attention of whatever is at the other end.
*/
sendbrk(fd)
{	int i;

	D4("BREAK");
	errno = 0;
	D5("sendbrk:before ioctl(fd=%d,TCSBRK=%d,0)",fd,TCSBRK);
	i = ioctl(fd,TCSBRK,0);
	D5("sendbrk: after ioctl(fd=%d,TCSBRK=%d,0)=%d	[errno=%d]",fd,TCSBRK,i,errno);
	if (i<0 || errno)
		D2("ioctl(%d,TCSBRK=%d,0)=%d\t[errno=%d]",fd,TCSBRK,i,errno);
	return i;
}
