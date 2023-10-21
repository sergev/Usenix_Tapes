#include "macros.h"

char rename_____[] "~|^`rename.c	1.3";
/*
	rename (unlink/link): write permission in directories is required
	(either implicity - owner == effuid, or explicitly)
	Calls xlink() and xunlink().
*/

rename(oldname,newname)
char *oldname, *newname;
{
	register int mo;
	int mn;
	struct inode buf;
	char *dold, *dnew;

	dold = dname(oldname);
	dnew = dname(newname);
	if(stat(dold,&buf) == -1)
		fatal(sprintf(Error,"directory `%s%s",dold,"' nonexistent (213)"));
	mo = buf.i_mode;
	if(stat(dnew,&buf) == -1)
		fatal(sprintf(Error,"directory `%s%s",dnew,"' nonexistent (213)"));
	mn = buf.i_mode;
	chmod(dold,mo|(IWRITE|IEXEC));
	chmod(dnew,mn|(IWRITE|IEXEC));
/*
	Just in case the newname is already being used, remove it if
	it is there.
*/
	unlink(newname);

	xlink(oldname,newname);
	xunlink(oldname);
	chmod(dold,mo);
	chmod(dnew,mn);
}
