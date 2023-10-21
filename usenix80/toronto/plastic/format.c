/*
 * routine to format long int's according to the following format:
 *
 *	' ' -- suppress leading zero's
 *	X -- copy digit as is
 *	, -- insert as placed
 *	. -- insert as placed
 *	- -- if not present, suppresses printing of minus
 *	$ -- suppress leading zero's and replace with $
 *	* -- suppress leading zero's and replace with *
 *
 *	errors cause return of zero, normal causes length of resulting string to
 * 	return.
 */

format(inta, cnt, buf, fstrng)
char *buf,*fstrng;
int *inta, cnt;
{
	register char *p,*r;
	register int i;
	char formbuf[60],zerbuf[60],*s,*tmp;
	char *locv(),*ends(),curmd;

	tmp = locv(inta, cnt);
	for (p = &formbuf[0]; p < &formbuf[60]; *p++ = '\0');
	for (p = &zerbuf[0]; p < &zerbuf[60]; *p++ = '0');
	p = &zerbuf[30];
	r = tmp;
	if (*r == '-')
		r++;
	while(*p++ = *r++);
	p = ends(&zerbuf[0]);
	r = fstrng;
	r = ends(r);
	s = &formbuf[58];
	curmd = 0;
	while(r >= fstrng)	{
		switch(*r)	{
			default:
				r--;
				break;

			case '_':
				*s-- = '_';
				r--;
				break;

			case 'X':
				*s-- = *p--;
				r--;
				break;

			case '.':
			case ',':
			case '/':
				if (curmd)	{
					*s-- = curmd;
					r--;
				} else
					if (p >= &zerbuf[30] || *r == '.')
						*s-- = *r--;
					else
						*s-- = *(r-- - 1);
				break;

			case '-':
				if (inta[cnt - 1] < 0)
					*s-- = '-';
				else
					*s-- = ' ';
				r--;
				break;

			case ' ':
			case '*':
			case '$':
				if (p >= &zerbuf[30])	{
					*s-- = *p--;
					r--;
					break;
				}
				curmd = *s-- = *r--;
				break;

		}
	}
	i = &formbuf[58] - s;
	p = buf;
	s++;
	while(*p++ = *s++);
	return(i);
}
