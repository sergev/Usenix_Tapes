#include "macros.h"

char remove_____[] "~|^`remove.c	1.1";
/*
	remove (unlink): write permission in directories is required
	(either implicity - owner == effuid, or explicitly)
	Calls xunlink().
*/

remove(file)
char *file;
{
	register int m;
	struct inode buf;
	char *d;

	d = dname(file);
	if(stat(d,&buf) == -1)
		fatal(sprintf(Error,"directory `%s%s",d,"' nonexistent (213)"));
	m = buf.i_mode;
	chmod(d,m|(IWRITE|IEXEC));
	xunlink(file);
	chmod(d,m);
}

