/* getkey.c - Don Libes
getkey() - return the last key struck by the user or -1.
If user has struct multiple keys, ignore all but the last.
*/

#include <stdio.h>
#if BSD42 || BSD41 || EUNICE
#include <sys/ioctl.h>
#endif
#if SYSV || SYSIII
#include <sys/termio.h>
#endif

#ifdef FIONREAD
int
getkey()
{
	long chars = 0;

	if (-1 == ioctl(fileno(stdin),FIONREAD,&chars)) {
		cleanup();
		perror("ioctl in getkey");
		exit(0);
	}
	
	if (chars == 0) return(-1);

	while (chars-- > 1) getchar();

	return(getchar() & 0x7f);		/* strip parity */
}
#endif FIONREAD
