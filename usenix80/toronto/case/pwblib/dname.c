# include "macros.h"
# define DIRSIZ 50
char dname_____[] "~|^`dname.c	2.3";
/*
	Returns directory name containing a file.  Returns "." if current
	directory; handles root correctly.
*/

dname(p)
char p[];
{
	static char dir[DIRSIZ];
	struct inode sb;
	register char *c;
	register int s;

	if((s=size(p)) > DIRSIZ) fatal(sprintf(Error,"`%s%s",p,"' too long (205)"));
	copy(p,dir);
	for(c=dir+s-2; c>dir; c--)
		if(*c == '/') {
			*c = '\0';
			if (stat(dir,&sb)== -1) continue;
			if ((sb.i_mode&IFMT)==IFDIR)
				return(dir);
		}
	if(*c == '/') *(c+1) = '\0';
	else copy(".",dir);
	return(dir);
}

