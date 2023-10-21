#include "uutty.h"
/*
** Wait on a lockfile.
*/
lockwait()
{	unsigned n;

	D5("lockwait()");
	if (lockfile[0] == 0) lockname();
	D4("lockwait:lockfile=\"%s\"",lockfile);
	n = 0;
	if (stat(lockfile,&status) >= 0) {
		D4("%s Lockfile \"%s\" exists.",getime(),lockfile);
		n = lockup();
		D4("%s Lockfile \"%s\" gone.",getime(),lockfile);
	}
	if (n) {				/* Port may be screwed up */
		opendev();			/* Close and reopen it */
		restdev();			/* Get it into proper state */
	}
	D4("lockwait:Returned after %d waits.",n);
	return n;
}
