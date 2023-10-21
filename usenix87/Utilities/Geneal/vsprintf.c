/* @(#)sprintf.c	4.1 (Berkeley) 12/21/80 */
/* vsprintf from sprintf */
/* Created from unix sprintf by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 09:00:15 by jimmc (Jim McBeath) */
/* last edit 11-Sept-85 22:00:00 by tlr (Terry L. Ridder) */
#include	<stdio.h>

/* vsprintf is like sprintf, but instead of passing a list of arguments,
 * the address of the list is passed.  This is typically used to implement
 * a function which accepts a format string and list of arguments of
 * its own.
 */

/* VARARGS2 */
char *vsprintf(str, fmt, argv)
char *str, *fmt;
{
#ifdef USG
	FILE _strbuf;

	_strbuf._flag = _IOWRT + _IOLBF;
	_strbuf._ptr = (unsigned char *)str;
#else
	struct _iobuf _strbuf;

	_strbuf._flag = _IOWRT + _IOSTRG;
	_strbuf._ptr = (unsigned char *)str;
#endif
	_strbuf._cnt = 32767;
	_doprnt(fmt, argv, &_strbuf);
	putc('\0', &_strbuf);
	return(str);
}
