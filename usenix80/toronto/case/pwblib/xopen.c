# include "macros.h"
char xopen_____[] "~|^`xopen.c:	2.2";
/*
	Interface to open(II) which differentiates among the various
	open errors.
*/

xopen(name,mode)
char name[];
int mode;
{
	register int fd;
	extern int errno;

	if((fd=open(name,mode)) >= 0) return(fd);
	if(errno == 2) fatal(sprintf(Error,"`%s%s",name,"' nonexistent (208)"));
	if(errno == 13) {
		if(mode == 0) fatal(sprintf(Error,"`%s%s",name,"' unreadable (209)"));
		if(mode == 1) fatal(sprintf(Error,"`%s%s",name,"' unwritable (210)"));
		fatal(sprintf(Error,"`%s%s",name,"' unreadable or unwritable (211)"));
	}
	fatal(stringf("error %d opening `%s' (212)",errno,name));
}
