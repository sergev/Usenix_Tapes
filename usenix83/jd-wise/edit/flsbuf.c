#include	<stdio.h>
#include	<errno.h>

char	*malloc();

int
_flsbuf(c, iop)
	unsigned c;
	register FILE *iop;
{
	register char *base;
	register n, rn;
	char c1;
	extern char _sobuf[];

#ifndef vax
	if (iop->_flag & _IORW) {
		iop->_flag |= _IOWRT;
		iop->_flag &= ~_IOEOF;
	}
#endif

tryagain:
	if (iop->_flag & _IONBF) {
		c1 = c;
		rn = 1;
		n = ewrite(fileno(iop), &c1, rn);
		iop->_cnt = 0;
	} else {
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
			n = ewrite(fileno(iop), base, n);
		}
		iop->_cnt = BUFSIZ - 1;
		*base++ = c;
		iop->_ptr = base;
	}
	if (rn != n) {
		iop->_flag |= _IOERR;
		return(EOF);
	}
	return(c);
}

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
		if (ewrite(fileno(iop), base, n) != n) {
			iop->_flag |= _IOERR;
			return(EOF);
		}
	}
	return(0);
}

/*
 * Flush buffers on exit
 */

_cleanup()
{
	register FILE *iop;
	extern FILE *_lastbuf;

	for(iop = _iob; iop < _lastbuf; iop++)
		fclose(iop);
}

int
fclose(iop)
	register FILE *iop;
{
	register r;

	r = EOF;
#ifdef vax
	if (iop->_flag & (_IOREAD|_IOWRT)
#else
	if (iop->_flag & (_IOREAD|_IOWRT|_IORW)
#endif
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
#ifdef vax
		~(_IOREAD|_IOWRT|_IONBF|_IOMYBUF|_IOERR|_IOEOF|_IOSTRG);
#else
		~(_IOREAD|_IOWRT|_IONBF|_IOMYBUF|_IOERR|_IOEOF|_IOSTRG|_IORW);
#endif
	iop->_cnt = 0;
	return(r);
}

ewrite(fildes, buffer, nbytes)
	char *buffer;
	{
	register n,j;
	extern errno;

	/** we really need some form of protection against other
	 ** interrupts here, but since they may only be ignored,
	 ** and not held, there's not much hope.
	 ** we could just ignore them, since the alarm will
	 ** eventually service them, but that could really mess
	 ** up the spell function */
	j=alarm(0);
rewrite:
	n=write(fildes,buffer,nbytes);
	if(n<=0){
#ifdef debugging
debug(83,"n=%d errno=%d",n,errno);
#endif
		if(errno==EINTR){
			write(2,"\007EINTR in ewrite",16);
/*
			goto rewrite;
*/
			}
		}
	alarm(j?j:30);
	return(n);
	}
