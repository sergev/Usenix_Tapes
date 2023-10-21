/* Tell the device driver to do normal I/O on the device.  
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
static	struct termio trmstat;
#endif
extern int errno;
extern int baudmask;	/* CBAUD mask if baud rate specified */

makesane(fn)
	int fn;		/* File number */
{	int i;

	D5("makesane(%d)",fn);
	errno = 0;
#ifdef SYS5
	i = ioctl(fn,TCGETA,&trmstat);
	if (debug >= 4) {
	  P("makesane: %d:\tcflag=%06o [old]",fn,trmstat.c_cflag);
	  P("makesane: %d:\tiflag=%06o [old]",fn,trmstat.c_iflag);
	  P("makesane: %d:\tlflag=%06o [old]",fn,trmstat.c_lflag);
	  P("makesane: %d:\toflag=%06o [old]",fn,trmstat.c_oflag);
	}
	D5("makesane: ioctl(fn=%d,TCGETA=%d,&trmstat=%08X)=%d",fn,TCGETA,&trmstat,i);
/* 
** The following was given by "stty -a" on one SYS/V machine:
**	speed 9600 baud; line = 2; intr = ^c; quit = ^\; erase = ^H; kill = ^x; eof = ^d; eol = ^@;susp = ^z;dsus = ^y
**	-parenb -parodd cs8 -cstopb -hupcl cread -clocal -tostop 
**	-ignbrk brkint ignpar -parmrk -inpck istrip -inlcr -igncr icrnl -iuclc 
**	ixon -ixany -ixoff 
**	isig icanon -xcase echo echoe echok -echonl -noflsh 
**	opost -olcuc onlcr -ocrnl -onocr -onlret -ofill -ofdel tab3 
speed 9600 baud; line = 2; intr = ^c; quit = ^\; erase = DEL; kill = ^x; eof = ^d; eol = ^@;susp = ^z;dsus = ^y
-parenb -parodd cs8 -cstopb -hupcl cread -clocal -tostop 
-ignbrk brkint ignpar -parmrk -inpck istrip -inlcr -igncr icrnl -iuclc 
ixon -ixany -ixoff 
isig icanon -xcase echo echoe echok -echonl -noflsh 
opost -olcuc onlcr -ocrnl -onocr -onlret -ofill -ofdel tab3 
*/
	trmstat.c_cflag &= CBAUD;	/* Save the speed */
	if (baudmask) {
	  D4("makesane: baudmask=0%o",baudmask);
	  trmstat.c_cflag = baudmask;	/* Set a different speed */
	}
	trmstat.c_cflag |= HUPCL  | CREAD  | CS8;
	trmstat.c_iflag  = BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
	trmstat.c_lflag  = ISIG   | ICANON | ECHO   | ECHOE | ECHOK;
	trmstat.c_oflag  = OPOST  | ONLCR  | TAB3;
	trmstat.c_cc[0] = 0x03;	/* INTR = ^C */
	trmstat.c_cc[1] = 0x1C;	/* QUIT = ^\ */
	trmstat.c_cc[2] = 0x10;	/* ERASE= ^H [BS] */
	trmstat.c_cc[3] = 0x18;	/* KILL = ^X */
	trmstat.c_cc[4] = 0x04;	/* EOF  = ^D */
	trmstat.c_cc[5] = 0;	/* EOL  char */
	trmstat.c_cc[6] = 0;	/* EOL2 char */
/*	trmstat.c_cc[7] = __;	** Reserved */
	trmstat.c_cc[8] = 0x1A;	/* SUSP = ^Z */
	trmstat.c_cc[9] = 0x19;	/* SUSP = ^Y */
	if (debug >= 5)  {
	  P("makesane: %d:\tcflag=%06o [new]",fn,trmstat.c_cflag);
	  P("makesane: %d:\tiflag=%06o [new]",fn,trmstat.c_iflag);
	  P("makesane: %d:\tlflag=%06o [new]",fn,trmstat.c_lflag);
	  P("makesane: %d:\toflag=%06o [new]",fn,trmstat.c_oflag);
	  Hexdnm(&trmstat.c_cc[0],NCC,"c_cc:");
	}
	i = ioctl(fn,TCSETA,&trmstat);
#endif
#ifdef SYS3		/* Note: Tested only on PC/IX */
	i = ioctl(fn,TCGETA,&trmstat);
	trmstat.c_cflag |= HUPCL  | PARENB | CREAD  | CS7;
	trmstat.c_iflag  = BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
	trmstat.c_lflag  = ISIG   | ICANON | ECHO   | ECHOE | ECHOK;
	trmstat.c_oflag  = OPOST  | ONLCR  | TAB3;
	D5("makesane: \tcflag=%06o",trmstat.c_cflag);
	D5("makesane: \tiflag=%06o",trmstat.c_iflag);
	D5("makesane: \tlflag=%06o",trmstat.c_lflag);
	D5("makesane: \toflag=%06o",trmstat.c_oflag);
	i = ioctl(fn,TCSETA,&trmstat);
#endif
#ifdef BERK		/* Note: Tested only on 4.2 */
	i = ioctl(fn,TIOCGETP,&sgttyb);
	sgttyb.sg_flags  =  SANE | TANDEM;
	ioctl(fn,TIOCSETP,&sgttyb);
	if (errno) {
		E("makesane: Can't do sane i/o on \"%s\"",fnnam);
		exit(-1);
	}
#endif
	D5("makesane: %d is now sane.",fn);
}
