
#include	"tp.h"
/*
 * _tpwloc writes one block of non-zero length to a character device.
 * It must take care of local restrictions.
 *
 * This is the VAX version (anything goes).
 */

int
_tpwloc(tf, buf, size) TPFILE *tf; char *buf;	{
	return write(tf->_tp_fildes, buf, size);
}
