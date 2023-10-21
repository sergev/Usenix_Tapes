char patoi_____[] "~|^`patoi.c:	2.1";
/*
	Function to convert ascii string to integer.  Converts
	positive numbers only.  Returns -1 if non-numeric
	character encountered.
*/

patoi(b)
char *b;
{
	register int i;
	register char *a;

	a = b;
	i = 0;
	while(*a >= '0' && *a <= '9') i = 10 * i + *a++ - '0';

	if(*a) return(-1);
	return(i);
}
