#include	"tp.h"

TPFILE *
tpopen(unit, nmdns, rwx) char *nmdns, *rwx;	{
	char *name = NULL, *err;
	static TPFILE tflist[_TP_MOPEN];
	TPFILE *tf;

	for (tf = &tflist[0]; tf->_tp_fildes != 0; tf++)
		if (tf == &tflist[_TP_MOPEN-1])	{
			err = "Too many tapes";
			goto Lerr;
		}

	tf->_tp_unit = unit; tf->_tp_nmdns = nmdns;
	tf->_tp_rw = 'r'; tf->_tp_x = ' ';
	while (*rwx)	switch (*rwx++)	{
	case 'r':
		tf->_tp_rw = 'r';
		break;
	case 'w':
		tf->_tp_rw = 'w';
		break;
	case 'x':
		tf->_tp_x = 'x';
		break;
	default:
		err = "Bad option in tpopen";
		goto Lerr;
	}
	tf->_tp_blkc = tf->_tp_mkc = tf->_tp_mc = tf->_tp_eof = 0;

	name = _tpname(tf);

	if (tf->_tp_rw == 'w')	{
		if ((tf->_tp_fildes = creat(name, 0666)) < 0)	{
			err = "cannot create";
			goto Lerr;
		}
	}
       	else	{
	       	if ((tf->_tp_fildes = open(name, 0)) < 0)	{
	       		err = "cannot open";
			goto Lerr;
		}
	}

	return tf;

Lerr:
	if (name != NULL)
		fprintf(tperr, "%s: ", name);
	fprintf(tperr, "%s\n", err);
	exit(1);
	return NULL;
}

FILE *tperr = stderr;

_tprwerr(msg, tf) char *msg; TPFILE *tf;	{
	fprintf(tperr,
		"After %d tape mark%s, after %D block%s: %s on %s\n",
		english(tf->_tp_mkc), english(tf->_tp_blkc), msg, _tpname(tf));
}
