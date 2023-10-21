char equal_____[] "~|^`equal.c:	2.2";
/*
	Compares 2 strings.  Returns 1 if equal, 0 if not.
*/

equal(a,b)
char *a, *b;
{
	register char *x, *y;

	x = a;
	y = b;
	while (*x == *y++) if (*x++ == 0) return(1);
	return(0);
}

