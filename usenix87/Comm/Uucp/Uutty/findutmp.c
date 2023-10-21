#include "uutty.h"
/* 
** Try to find our entry in the /etc/utmp file.
** If we find it, it is copied to our global utmp.
** This works on SYS/V, and probably on SYS/III,
** but likely not on anything else.
*/
struct utmp *
findutmp()
{	int rec;		/* Record number, used in debug output */

	pid = getpid();		/* We are looking for an entry with out process id */
	rec = 0;		/* No records read so far */
	up  = 0;		/* No entry unpacked so far */
#ifdef SYS5
	setutent();		/* Restart at the top of the file */
loop:
	D5("before getutent()");
	errno = 0;
	up = getutent();	/* Read in one /etc/utmp entry */
	++rec;
	D4("getutent()=%06lX rec=%d\t[errno=%d]",up,rec,errno);
	if (up == 0) {
		D3("findutmp() FAILED.");
		return 0;
	}
	if (debug >= 5) Hexdnm(up,sizeof(struct utmp),"utmp");
	if (up->ut_pid != pid) {
		D4("findutmp: rec=%d pid=%d wrong.",rec,pid);
		Loop;
	}
	D4("findutmp: rec=%d pid=%d is me.",rec,pid);
	copy(&utmp,up,sizeof(struct utmp));
	up = &utmp;
	return up;
#endif
}
