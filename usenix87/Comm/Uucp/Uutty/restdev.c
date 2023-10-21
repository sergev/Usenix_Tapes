#include "uutty.h"
/*
** Set the device to our desired (raw) state.  This may only 
** work if we are the super-user.  This routine, such as it is, 
** should work on just about any Unix system.  See makeraw.c
** for the real system-dependent stuff.
*/
restdev()
{	int i;

	D6("restdev()");
	makeraw(dev);			/* We want to do raw I/O */
	errno = 0;
	D2("Change \"%s\" to user %d, group %d, permissions 666.",device,euid,egid);
	i = chown(device,euid,egid);	/* Try to get ownership */
	D4("restdev: chown(\"%s\",%d,%d)=%d",device,euid,egid,i);
	errno = 0;
	i = chmod(device,0666);		/* Make it publicly accessible */
	D4("restdev: chmod(\"%s\",0%o)=%d\t[errno=%d]",device,0666,i,errno);
	return i;
}
