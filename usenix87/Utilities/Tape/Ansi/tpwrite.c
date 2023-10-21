#include	"tp.h"

tpwrite(tf, buf, size) TPFILE *tf; char *buf;	{

	if (tf->_tp_unit == TP_IMAG)	{
		int i, n = size;
		char pref[_TP_PREF];
		for (i = _TP_PREF; i--; )	{
			pref[i] = n % 10 + '0';
			n = n / 10;
		}
		if (write(tf->_tp_fildes, pref, _TP_PREF) != _TP_PREF)
			goto Lerr;
		if (size > 0)
			if (write(tf->_tp_fildes, buf, size) != size)
				goto Lerr;
	}
	else	{
		if (size == 0)
			_tpwtmloc(tf);
		else
		if (_tpwloc(tf, buf, size) != size)
			goto Lerr;
	}

	if (size == 0)	{
		tf->_tp_mkc++;
		tf->_tp_blkc = 0;
	}
	else	{
		tf->_tp_blkc++;
	}

	return;

Lerr:
	_tprwerr("write error", tf);
	exit(1);
}
