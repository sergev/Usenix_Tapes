#
#include        "./gparam"

/*
**      This routine will write out an alphanumeric string to the
**      user's  window  starting  at  the column 'fc' and writing
**      left to right.  The maximum number of characters possible
**      is 73 (with a 512 column window).
*/

int gwstr(area, string, fc, fr)
struct abuf *area;
char *string;
{
	register struct abuf *rarea;
	register *rbuf;
	register char *sptr;
	int cnt;
	int buff[81];

	rarea = area;

	cnt = 1;
	if(rarea->dsize) cnt++;

	if((fc | fr) < 0 || (fc > rarea->coln - (7 * cnt)) || (fr > rarea->rown - (9 * cnt)))
		return(-1);

	if((fc + (7 * cnt) * len(string)) > rarea->coln)
		return(-1);

	rbuf = buff;

	*rbuf++ = rarea->ldc;
	*rbuf++ = rarea->lsm;
	*rbuf++ = rarea->lwm;
	*rbuf++ = LUM | EBNEA;
	*rbuf++ = LEA | (rarea->fcol + fc);
	*rbuf++ = LEB | (7 * cnt);
	*rbuf++ = LLA | (rarea->frow + fr);

	sptr = string;
	rbuf = buff;
	cnt = 7;
	while(*sptr)
		rbuf[cnt++] = WAC | (*sptr++ & 0177);

	if(write(rarea->fdesc, buff, (cnt =* 2)) != cnt)
		return(-1);
	return(0);
}

int len(thing)
char thing[];
{
	register char *ptr;

	ptr = thing;
	while(*ptr++);
	return(ptr - thing - 1);
}
