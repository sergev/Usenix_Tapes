/* Tell the device driver to do raw I/O on the device.  
 * Unfortunately, this is done in different ways on different
 * brands of Unix.  Make sure that your system is included
 * somewhere in the following list.
 */
#include <dbg.h>
#ifdef BERK
#	include <sgtty.h>
	int ldisc = NETLDISC;
	struct sgttyb sgttyb;
#endif
#ifdef PCIX
#	include <termio.h>
	struct termio termio;
#endif
#ifdef SYS5
#	include <termio.h>
	struct termio termio;
#endif
extern int baudmask;	/* CBAUD mask, if baud rate specified */
extern int errno;
int  flowcontrol = 1;
uint timeout = 100;	/* Timeout in 0.1 second quanta */

makeraw(fn)
	int fn;		/* File number */
{	int i;

	D5("makeraw(%d)",fn);
	errno = 0;
#ifdef SYS5
	i = ioctl(fn,TCGETA,&termio);
	D4("makeraw: %d:\tcflag=%06o [old]",fn,termio.c_cflag);
	D4("makeraw: %d:\tiflag=%06o [old]",fn,termio.c_iflag);
	D4("makeraw: %d:\tlflag=%06o [old]",fn,termio.c_lflag);
	D4("makeraw: %d:\toflag=%06o [old]",fn,termio.c_oflag);
	D5("makeraw: ioctl(fn=%d,TCGETA=%d,&termio=%08X)=%d",fn,TCGETA,&termio,i);
	if (baudmask)			/* Wipe out all of cflag but speed */
	     termio.c_cflag  = baudmask;
	else termio.c_cflag &= CBAUD;
	termio.c_cflag |= CS8 | CREAD | HUPCL;
	termio.c_iflag  = flowcontrol? (IXON | IXANY | IXOFF): 0;
	termio.c_lflag  = 0;
	termio.c_oflag  = 0;
	termio.c_cc[4]  = 0;		/* Number of bytes to buffer up */
	termio.c_cc[5]  = timeout;	/* Timeout in 0.1 sec units */
	D4("makeraw: %d:\tcflag=%06o [new]",fn,termio.c_cflag);
	D4("makeraw: %d:\tiflag=%06o [new]",fn,termio.c_iflag);
	D4("makeraw: %d:\tlflag=%06o [new]",fn,termio.c_lflag);
	D4("makeraw: %d:\toflag=%06o [new]",fn,termio.c_oflag);
	i = ioctl(fn,TCSETA,&termio);
#endif
#ifdef SYS3
	i = ioctl(fn,TCGETA,&termio);
	termio.c_iflag |=  IGNCR | IXON | IXANY | IXOFF;
	termio.c_lflag  =  0;
	D5("makeraw: \tcflag=%06o",termio.c_cflag);
	D5("makeraw: \tiflag=%06o",termio.c_iflag);
	D5("makeraw: \tlflag=%06o",termio.c_lflag);
	D5("makeraw: \toflag=%06o",termio.c_oflag);
	i = ioctl(fn,TCSETA,&termio);
#endif
#ifdef BERK		/* Berkeley Unix has its own conventions */
	i = ioctl(fn,TIOCGETP,&sgttyb);
	sgttyb.sg_flags  =  RAW | TANDEM;
	ioctl(fn,TIOCSETP,&sgttyb);
	if (errno) {
		E("makeraw: Can't do raw i/o on \"%s\"",fnnam);
		exit(-1);
	}
#endif
	D5("makeraw: %d is now raw",fn);
}
