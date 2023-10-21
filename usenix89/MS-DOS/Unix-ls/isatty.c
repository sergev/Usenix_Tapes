/* Return non-zero if the specified handle refers to a device */

/* Written by Bernie Roehl, July 1986 */

#define LINT_ARGS
#include	<stdio.h>
#include	<dos.h>
#include	"masdos.h"

int isatty()
{
	struct REGS regs;
	regs.h.ah = 0x44;
	regs.h.al = 0x00;
	regs.x.bx = 0x0001;
	int86( I_BDOS, &regs, &regs );
	return( (int) regs.x.dx & IO_ISDEV );
}
