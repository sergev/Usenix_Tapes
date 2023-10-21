/*
 * Routines dealing with getting input from the keyboard (i.e. from the user).
 */

#include "less.h"

/*
 * The boolean "reading" is set true or false according to whether
 * we are currently reading from the keyboard.
 * This information is used by the signal handling stuff in signal.c.
 * {{ There are probably some race conditions here
 *    involving the variable "reading". }}
 */
public int reading;

static int tty;

/*
 * Open keyboard for input.
 * (Just use file descriptor 2.)
 */
	public void
open_getc()
{
	tty = 2;
}

/*
 * Get a character from the keyboard.
 */
	public int
getc()
{
#if MSDOS
#if MSC
	int c;
	struct regs {
		int ax, bx, cx, dx, si, di, ds, es;
	} cregs, rregs;
	int intno = 0x016;
#else
	char c;
#endif
#endif
	int result;

	reading = 1;
#if MSDOS
#if MSC
	cregs.ax = 0x0000;			/* set registers */
	int86(0x16, &cregs, &cregs);		/* call BIOS - INT 16h */
	c = (cregs.ax & 0x00ff);
	reading = 0;
	return(c & 0177);
#else
	do
	{
		flush();
		result = read(tty, &c, 1);
	} while (result != 1);
	reading = 0;
	return (c & 0177);
#endif
#endif
}

