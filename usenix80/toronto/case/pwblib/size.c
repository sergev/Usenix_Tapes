char size_____[] "~|^`size.c:	2.1";
/*
	Returns size (counting null byte) of arg string.
*/

size(s)
char s[];
{
	register int i;

	i = 0;
	while(s[i++]);
	return(i);
}

