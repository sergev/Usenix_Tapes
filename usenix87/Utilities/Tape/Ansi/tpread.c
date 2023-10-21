#include	"tp.h"

int
tpread(tf, buf, size) TPFILE *tf; char *buf;	{
	int res;

	if (tf->_tp_eof)
		return EOF;

	if (tf->_tp_unit != TP_IMAG && tf->_tp_x == ' ' && tf->_tp_mc >= 4)
		res = EOF;
	else
		res = _tp_rd(tf, buf, size);

	if (res > 0)	{
		tf->_tp_blkc++;
		tf->_tp_mc = 0;
	}
	else
	if (res == 0)	{
		tf->_tp_mkc++;
		tf->_tp_mc++;
		tf->_tp_blkc = 0;
	}
	else
	if (res == EOF)	{
		tf->_tp_mc = 0;
	}

	return size < res ? size : res;
}

int
_tp_rd(tf, buf, size) TPFILE *tf; char *buf;	{
	int sz;
	char ch;

	if (size <= 0)	{
		buf = &ch;
		size = 1;
	}

	if (tf->_tp_unit == TP_IMAG)	{
		char pref[_TP_PREF];
		int n;
		char ch;

		n = read(tf->_tp_fildes, pref, _TP_PREF);
		if (n == 0)
			return EOF;
		if (n != _TP_PREF)
			goto Lformerr;
		sz = 0;
		for (n = 0; n < _TP_PREF; n++)	{
			int dig = pref[n] - '0';
			if (dig < 0 || dig > 9)
				goto Lformerr;
			sz = sz*10 + dig;
		}
		n = sz < size ? sz : size;
		if (n > 0)
			if (read(tf->_tp_fildes, buf, n) != n)
				goto Lformerr;
		while (sz-- > size)
			if (read(tf->_tp_fildes, &ch, 1) != 1)
				goto Lformerr;
		return n;

	Lformerr:
		_tprwerr("tape image error", tf);
		exit(1);
	}
	else	{
		int erc = 0;

		while ((sz = _tprdloc(tf, buf, size)) < 0)	{
			_tprwerr("garbage", tf);
			erc++;
			if (	(tf->_tp_x == ' ' &&
					(tf->_tp_mc > 0 || erc >= 2))
				||
				(tf->_tp_x == 'x' &&
					erc >= 100)
			)	return EOF;
		}
		return sz;
	}
	return EOF;	/* stupid `lint' */
}
