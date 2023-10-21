#include	"tp.h"
/*
 * _tprdloc reads one block from a character device.
 * It must take care of local restrictions.
 *
 * This is the VAX version (anything goes).
 */

int
_tprdloc(tf, buf, size) TPFILE *tf; char *buf;	{
	return read(tf->_tp_fildes, buf, size);
}
