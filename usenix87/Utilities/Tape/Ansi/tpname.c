#include	"tp.h"

char * /* transient */
_tpname(tf) TPFILE *tf;	{
	static char name[20];
	int unit = tf->_tp_unit;
	char *nmdns = tf->_tp_nmdns;
	char *nm = name;

	if (unit == TP_IMAG || unit == TP_CDEV)
		return nmdns;

	if (tf->_tp_rw == 'w')
		while (*nmdns++) {}
	do
		*nm++ = *nmdns == '?' ? unit + '0' : *nmdns;
	while (*nmdns++);

	return name;
}
