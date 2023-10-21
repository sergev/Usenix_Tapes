#include	"tp.h"

tpclose(tf) TPFILE *tf;	{

	VOID(close(tf->_tp_fildes));
	if (tf->_tp_rw == 'w')	{
		tf->_tp_rw = 'r';
		VOID(close(open(_tpname(tf), 0)));
	}
	tf->_tp_fildes = 0;
}
