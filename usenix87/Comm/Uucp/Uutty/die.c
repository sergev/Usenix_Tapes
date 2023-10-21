#include "uutty.h"
#include <signal.h>

die(retval)
{	int i;

	D4("die(%d)",retval);
	if (locked) unlock();
	l_tries = l_reads = 1;	/* Give up quickly */
	Resync;			/* Gobble up all input */
	if (m_exit) Awrite(m_exit);	/* Special exitial message? */
	if (termfl && isatty(dev)) {	/* Restore terminal status */
		D7("die: %d:\tcflag=%06o",dev,trminit.c_cflag);
		D7("die: %d:\tiflag=%06o",dev,trminit.c_iflag);
		D7("die: %d:\tlflag=%06o",dev,trminit.c_lflag);
		D7("die: %d:\toflag=%06o",dev,trminit.c_oflag);
		D5("die:before ioctl(%d,%d,%06lX)",dev,TCSETA,&trminit);
		i = ioctl(dev,TCSETA,&trminit);
		D5("die: after ioctl(%d,%d,%06lX)=%d",dev,TCSETA,&trminit,i);
		D3("File %d restored to normal.",dev);
	}
	exit(retval);
}
