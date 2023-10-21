char copy_____[] "~|^`copy.c:	2.2";
/*
	Copy first string to second; no overflow checking.
	Returns pointer to null in new string.
*/

copy(a,b)
register char *a, *b;
{
	while(*b++ = *a++);
	return(--b);
}
