/* Low-level I/O handling for 68000
 * Uses 2nd PCIA for putchar, etc.
 */

#include "nu.h"
#include "dbg.h"

#define	TIMEOUT	500000

short	*PCIDATA = (short *) PCI2_DATA,	/* PCI register adrs		*/
	*PCISR	= (short *) PCI2_SR,
	*PCIMR	= (short *) PCI2_MR,
	*PCICR	= (short *) PCI2_CR;

char	IntIO = 0,		/* 1 iff using interrupt-based I/O	*/
	TTIFlg = 0,		/* 1 iff TTIChr is a valid input char	*/
	TTIChr = 0;		/* Current input character.		*/

extern long IVector[];

/* TYI interrupt handler						*/

TYIint(ss)
 {	TTIChr = (*PCIDATA & 0377);
	if ((TTIChr & 0177) == INTCHR) Genesis();
	TTIFlg = 1; }

/* TYO interrupt handler						*/

TYOint()
 {	return;		}

/* dummy exit routine:							*/

Split()
 { buzz: goto buzz; }

char getchar()
 {	if (IntIO)
	 { while (!TTIFlg);
	   TTIFlg = 0;
	   return TTIChr; }

	while (!(*PCISR & 02));
	return (*PCIDATA & 0377); }

int kbhit()		/* returns 1 iff keyboard character waiting	*/
 {	if (IntIO) return TTIFlg;
	return (*PCISR & 02); }

putchar(ch)
 char ch;
 {	while (!(*PCISR & 01));
	*PCIDATA = ch;
	if (ch == '\n') putchar('\r'); }

putst(str)
 char *str;
 {	while (*str) putchar(*str++); }

/* Initialize I/O.  If arg != 0, turns on interrupts & initializes
 *  interrupt-based I/O; else initializes non-interrupt I/O.
 */

ioinit(inton)
 {	short ignore = *PCICR;	/* Read command reg to cause reset	*/

	register long x;
	register short i;
	register short *p =
		(short *) IOEVRAM;

	spl7();			/* Raise processor priority		*/
	*((short *) IOCTRL) = 0;/* Turn off I/O interrupts		*/
	*PCIMR = 0X4E;		/* 1 stop bit, 8 data bits, no par, x16 clk */
	*PCIMR = 0X3E;		/* 9600 baud				*/
	*PCICR = 0X37;		/* reset errors, enable receiver */
	ignore = *PCIDATA;	/* just to reset things. */

	x = (INT0 << 4)|1;
	for (i=0; i<EVNUM; i++)
	 { *p++ = x; p++;
	   x += (1 << 4); }

	if (inton)
/*	 { IVector[INTTTI] = (long) TYIint;
	   IVector[INTTTO] = (long) TYOint;
	   TTIFlg = 0;
	   IntIO = 1;
	   *((short *) IOCTRL) = IOENAB;
	   spl0();
	   ignore = *PCIDATA; }
 */ inton=inton;
	else IntIO = 0;
 }
