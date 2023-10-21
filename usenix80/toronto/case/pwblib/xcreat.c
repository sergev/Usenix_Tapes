#include "errnos.h"
#include "macros.h"

char xcreat_____[] "~|^`xcreat.c:	2.5";
/*
	"Sensible" creat: write permission in directory is required
	(either implicity - owner == effuid, or explicitly)
	in all cases, and created file is guaranteed to have specified mode
	and be owned by effective user.
*/

xcreat(name,mode)
char name[];
int mode;
{
	register int fd, m;
	extern int errno;
	struct inode buf;
	register char *d;

	d = dname(name);
	if(stat(d,&buf) == -1) fatal(sprintf(Error,"directory `%s%s",d,"' nonexistent (213)"));
	m = buf.i_mode;
	chmod(d,m|(IWRITE|IEXEC));
	if(unlink(name) != -1 || errno == ENOENT)
		fd=creat(name,mode);
	else
		fd = -1;
	chmod(d,m);
	if (fd >= 0)
		return(fd);
	if(errno == EACCES) fatal(sprintf(Error,"directory `%s%s",d,"' unwritable (206)"));
	else if (errno == ENOSPC) fatal("no space! (351)");
	fatal(stringf("error %d creating `%s' (207)",errno,name));
}
