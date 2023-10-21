getshno(cp, np, str)
char	*cp, *np, *str;
{
	register	snum;

	snum = onearg(cp, np);
	if( getship(snum, str) == -1 ) snum = -1;
	return(snum);
}
