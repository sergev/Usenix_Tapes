#include "uutty.h"
/* 
** The lockfile exists; hang until it disappears.
*/
lockup()
{	int n;

	n = 0;
	do {
		if ((n==0 && debug>=2) || debug>=3)
			P("%s Lockfile \"%s\" exists.",getime(),lockfile);
		if (lsleep > 0)
			sleep(lsleep);
		++n;
		}
			while (stat(lockfile,&status) >= 0);
	D3("locked: Lockfile \"%s\" freed.",lockfile);
	if (debug>=2)
		P("%s Lockfile \"%s\" gone.",getime(),lockfile);
	return n;
}
