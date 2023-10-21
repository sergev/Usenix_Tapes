#include "uutty.h"
/*
** Remove the lockfile.
*/
unlock()
{
	D2("%s: Delete lockfile %d=\"%s\".",getime(),lockfn,lockfile);
	unlink(lockfile);
	D4("Close lockfn=%d.",lockfn);
	close(lockfn);
	lockfn = -1;
	locked = 0;
}
