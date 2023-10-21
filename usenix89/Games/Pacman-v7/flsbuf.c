/*LINTLIBRARY*/
#include <stdio.h>

/*
**  It is necessary to count the number of characters that are printed.
**  It was decided that the easiest way to do this would be to increment
**  a counter variable in _flsbuf().
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/
extern	char	*malloc();
extern	long	chcnt;		/* counter */

int
_flsbuf(c, iop)
	unsigned c;
	register FILE *iop;
{
	register char *base;
	register n = 1, rn;
	extern char _sobuf[];

	if (iop->_flag & _IORW) {
		iop->_flag |= _IOWRT;
		iop->_flag &= ~_IOEOF;
	}

tryagain:
	if (iop->_flag & _IONBF) {
		iop->_cnt = 0;
		if (1 != write(fileno(iop), (char *) &c, 1))
		{
			iop->_flag |= _IOERR;
			return(EOF);
		}
		chcnt++;	/* counter incremented */
		return(c);
	} 
	if ((base = iop->_base) == NULL) {
		if (iop == stdout) {
			if (isatty(fileno(stdout))) {
				iop->_flag |= _IONBF;
				goto tryagain;
			}
			iop->_base = _sobuf;
			iop->_ptr = _sobuf;
			goto tryagain;
		}
		if ((iop->_base = base = malloc(BUFSIZ)) == NULL) {
			iop->_flag |= _IONBF;
			goto tryagain;
		}
		iop->_flag |= _IOMYBUF;
		rn = n = 0;
	} else if((rn = n = iop->_ptr - base) > 0) {
		iop->_ptr = base;
		n = write(fileno(iop), base, n);
		chcnt += (long) n;	/* counter incremented */
	}
	iop->_cnt = BUFSIZ - 1;
	*base++ = c;
	iop->_ptr = base;
	if (rn != n) {
		iop->_flag |= _IOERR;
		return(EOF);
	}
	return(c);
}

#ifndef	LINT
int
fflush(iop)
	register FILE *iop;
{
	register char *base;
	register n;

	if ((iop->_flag & (_IONBF|_IOWRT)) == _IOWRT
	 && (base = iop->_base) != NULL && (n = iop->_ptr - base) > 0) {
		iop->_ptr = base;
		iop->_cnt = BUFSIZ;
		if (write(fileno(iop), base, n) != n) {
			iop->_flag |= _IOERR;
			return(EOF);
		}
	}
	return(0);
}

/*
 * Flush buffers on exit
 */

#ifndef	BSD43
_cleanup()
{
	register FILE *iop;
	extern FILE *_lastbuf;

	for(iop = _iob; iop < _lastbuf; iop++)
		fclose(iop);
}
#endif

int
fclose(iop)
	register FILE *iop;
{
	register r;

	r = EOF;
	if (iop->_flag & (_IOREAD|_IOWRT|_IORW)
	    && (iop->_flag & _IOSTRG) == 0) {
		r = fflush(iop);
		if (close(fileno(iop)) < 0)
			r = EOF;
		if (iop->_flag & _IOMYBUF)
			free(iop->_base);
		if (iop->_flag & (_IOMYBUF|_IONBF))
			iop->_base = NULL;
	}
	iop->_flag &=
		~(_IOREAD|_IOWRT|_IONBF|_IOMYBUF|_IOERR|_IOEOF|_IOSTRG|_IORW);
	iop->_cnt = 0;
	return(r);
}
#endif
