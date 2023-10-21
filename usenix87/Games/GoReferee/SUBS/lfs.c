/*
**	strip line feeds of the argument
*/

char    *
lfs(str)
char   *str;
{
	register char *fp, *tp;
	static char buf[128];

	for (fp = str, tp = buf; *tp = *fp; )
	    if (*fp++ != '\n')
		tp++;
	return(buf);
}
