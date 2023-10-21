# include "stdio.h"

fdfopen(fd, mode)
int fd, mode;
{
register struct _iobuf *iop;
extern struct _iobuf _iob[], *_lastbuf;

	for(iop = _iob; iop->_flag & (_IOREAD|_IOWRT); iop++)
		if(iop >= _lastbuf)
			return(NULL);

	iop->_flag =& ~(_IOREAD|_IOWRT);
	iop->_file = fd;
	iop->_flag |= (mode ? _IOWRT : _IOREAD);
	return(iop);
}
