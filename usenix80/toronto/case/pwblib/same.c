char same_____[] "~|^`same.c:	2.1";
/*
	Compares 1st n characters of strings a and b.  Returns 1 is equal,
	0 if not.  Both a and b must be at least of length n.
*/

same(a,b,n)
char a[], b[];
int n;
{
	register int i;

	for(i=0; i<n; i++) {
		if(a[i] != b[i]) return(0);
		if(!a[i]) break;
	}
	return(1);
}
