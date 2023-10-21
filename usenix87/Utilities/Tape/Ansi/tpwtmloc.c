#include	"tp.h"
/*
 * _tpwtmloc writes a tape mark to a character device.
 * It must take care of the local restrictions.
 */

_tpwtmloc(tf) TPFILE *tf;	{

	VOID(close(tf->_tp_fildes));
	tf->_tp_fildes = open(_tpname(tf), 1);
}
