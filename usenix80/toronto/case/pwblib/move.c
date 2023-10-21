char move_____[] "~|^`move.c:	2.3";
/*
	Copies n characters from string a to string b.
	No check on overflow.
*/

struct { int *ip; };

move(a,b,n)
char *a,*b;
int n;
{
	register char *x, *y;
	register int m;

	if (m=n) {
		x = a;
		y = b;
		if (((x^y)&1)==0) {
			if (x&1) {
				*y++ = *x++;
				n--;
			}
			for (m=n/2+1; --m; )
				*(y.ip)++ = *(x.ip)++;
			m = n&1;
		}
		for (++m; --m; ) *y++ = *x++;
	}
}

