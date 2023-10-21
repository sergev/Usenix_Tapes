
#include	"tp.h"
/*
 * _tpwloc writes one block of non-zero length to a character device.
 * It must take care of local restrictions.
 *
 * This is the PDP11/45 version (even block size).
 */

int
_tpwloc(tf, buf, size) TPFILE *tf; char *buf;	{
	int sz = write(tf->_tp_fildes, buf, size % 2 ? size + 1 : size);
	return sz > size ? size : sz;
}
