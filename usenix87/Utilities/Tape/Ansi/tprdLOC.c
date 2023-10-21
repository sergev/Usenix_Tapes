#include	"tp.h"
/*
 * _tprdloc reads one block from a character device.
 * It must take care of local restrictions.
 *
 * This is the PDP11/45 version (even block size).
 */

int
_tprdloc(tf, buf, size) TPFILE *tf; char *buf;	{
	char ch = buf[size];
	int sz = read(tf->_tp_fildes, buf, size % 2 ? size + 1 : size);
	buf[size] = ch;
	return sz > size ? size : sz;
}
