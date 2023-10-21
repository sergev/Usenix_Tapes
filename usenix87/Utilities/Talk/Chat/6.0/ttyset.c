/* ttyset.c :=: 9/25/85   By SLB */

#include "chat.h"

#ifdef TERMIO
# include <termio.h>
#else
# include <sgtty.h>
# include <sys/ioctl.h>
#endif

#ifdef TERMIO
 struct	termio	Nmode, Omode;
#else
 struct	sgttyb	Nmode, Omode;
#endif
		
ttyset(cbreak,echo)	/* set tty to/from cbreak & echo mode */
{
#ifdef TERMIO
	if (ioctl(0, TCGETA, &Nmode) == ERR) {
#else
	if (ioctl(0, TIOCGETP, &Nmode) == ERR) {
#endif TERMIO
		puts("No terminal");
		exit(1);
	}
	if (cbreak) {
#ifdef TERMIO
		Nmode.c_lflag &= ~ICANON;
		Nmode.c_cc[VMIN]=1;
		Nmode.c_cc[VTIME]=0;
#else
		Nmode.sg_flags |= CBREAK;
#endif TERMIO
	} else {
#ifdef TERMIO
		Nmode.c_lflag |= ICANON;
		Nmode.c_cc[VEOF]=CEOF;
		Nmode.c_cc[VEOL]=CNUL;
#else
		Nmode.sg_flags &= ~CBREAK;
#endif TERMIO
	}

	if (echo)
#ifdef TERMIO
		Nmode.c_lflag |= ECHO;
#else
		Nmode.sg_flags |= (ECHO	| CRMOD);
#endif TERMIO
	else
#ifdef TERMIO
		Nmode.c_lflag &= ~ECHO;
#else
		Nmode.sg_flags &= ~(ECHO | CRMOD);
#endif TERMIO

#ifdef TERMIO
	if (ioctl(0, TCSETA, &Nmode) == ERR) {
#else
	if (ioctl(0, TIOCSETP, &Nmode) == ERR) {
#endif TERMIO
		puts("Mode switch error.");
		exit(1);
	}
}

tstore()	/* Store previous terminal settings */
{
#ifdef TERMIO
	if (ioctl(0, TCGETA, &Omode) == ERR) {
#else
	if (ioctl(0, TIOCGETP, &Omode) == ERR) {
#endif TERMIO
		fputs("No terminal.",stderr);
		quit();
	}
}

trestore()	/* Restore previous terminal settings */
{

#ifdef TERMIO
	if (ioctl(0, TCSETA, &Omode) == ERR)
#else
	if (ioctl(0, TIOCSETP, &Omode) == ERR)
#endif TERMIO
		fputs("Mode switch error.",stderr);
}
