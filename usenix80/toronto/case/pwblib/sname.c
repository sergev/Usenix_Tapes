char sname_____[] "~|^`sname.c:	2.1";
/*
	Returns pointer to "simple" name of path name; that is,
	pointer to first character after last "/".  If no slashes,
	returns pointer to first char of arg.
*/

sname(s)
char *s;
{
	register char *p;

	for(p=s; *p; p++) if(*p == '/') s = p + 1;
	return(s);
}

