#include "dbg.h"
/* 
** Assorted string-manipulation subroutines.
*/
/* Match a null-terminated string against the initial portion
** of another string.  If the match succeeds, return its length;
** else return 0.
*/
int st_init(x,y)
	char *x;
	char *y;
{	char *s;
	char *t;

	s = x;
	t = y;
	while (*s)
		if (*s++ != *t++) goto fail;
	D9("st_init(\"%s\",\"%s\")=%d",x,y,s-x);
	return(s - x);
fail:
	D9("st_init(\"%s\",\"%s\")=%d",x,y,0);
	return 0;
}
/* Return the int value of the trailing digits of a null-terminated string.
** Note that non-numeric chars reset the value, so only digits that follow
** all non-numerics are used.  Also, only the last '-' is effective.  Thus
** "15-x-37" gives the value 37.
*/	
int st_ival(s)
	char *s;
{	int   c, val, sign;

	sign = val = 0;
	while (c = *s++)
		if ('0'<=c && c<='9')
			val = (val * 10) + (c - '0');
		else if (c == '-')
			sign = c;
		else 
			sign = val = 0;
	return(sign ? -val : val);
}
/* Convert a long to an ASCII string.
** Return pointer to byte just after the value.
*/
char * st_ltoa(l,a)
	long  l;
	char *a;	
{
	if (l < 0) {*a++ = '-'; l = -l;}
	if (l > 9) a = st_ltoa((l/10),a);
	*a++ = '0' + (l % 10);
	return a;
}
/* Return the long value of the trailing digits of a null-terminated string.
** Note that non-numeric chars reset the value, so only digits that follow
** all non-numerics are used.  Also, only the last '-' is effective.  Thus
** "15-x-37" gives the value 37.
*/	
long st_lval(s)
	char *s;
{	int   c, sign;
	long  val;

	val = sign = 0;
	D4("st_lval: sign=%d val=%ld",sign,val);
	while (c = *s++) {
		if ('0'<=c && c<='9')
			val = (val * 10) + (c - '0');
		else if (c == '-')
			sign = c;
		else 
			val = sign = 0;
		D4("st_lval: sign=%d val=%ld c=%02x='%c'",sign,val,c,dsp(c));
	}
	return(sign ? -val : val);
}	
