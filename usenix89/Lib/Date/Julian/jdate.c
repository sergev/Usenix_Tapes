/* Julian date converter. Takes a julian date (the number of days since
** some distant epoch or other), and returns an int pointer to static space.
** ip[0] = month;
** ip[1] = day of month;
** ip[2] = year (actual year, like 1977, not 77 unless it was  77 a.d.);
** ip[3] = day of week (0->Sunday to 6->Saturday)
** These are Gregorian.
** Copied from Algorithm 199 in Collected algorithms of the CACM
** Author: Robert G. Tantzen, Translator: Nat Howard
*/
int *
jdate(j)
long j;
{
	static int ret[4];

	long d, m, y;

	ret[3] = (j + 1L)%7L;
	j -= 1721119L;
	y = (4L * j - 1L)/146097L;
	j = 4L * j - 1L - 146097L * y;
	d = j/4L;
	j = (4L * d + 3L)/1461L;
	d = 4L * d + 3L - 1461L * j;
	d = (d + 4L)/4L;
	m = (5L * d - 3L)/153L;
	d = 5L * d - 3 - 153L * m;
	d = (d + 5L) / 5L;
	y = 100L * y + j;
	if(m < 10) 
		m += 3;
	else {
		m -= 9;
		++y;
	}
	ret[0] =  m;
	ret[1] = d;
	ret[2] = y;
	return(ret);
}

