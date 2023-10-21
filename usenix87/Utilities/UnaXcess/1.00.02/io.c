/*
 *	@(#)io.c	1.2 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)io.c	1.1 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"
#include <varargs.h>

#define linelen	user.u_llen
#define pagelen	user.u_lines

#ifdef SYS3
#  include <sys/ioctl.h>
#  include <termio.h>
#  define TERMPARAMS		struct termio
#  define GETPARAMS(fd, buf)	ioctl(fd, TCGETA, buf)
#  define SETPARAMS(fd, buf)	ioctl(fd, TCSETAW, buf)
#  define TERM_CHARMODE(buf)	((buf)->c_lflag &= ~(ICANON|ECHO), (buf)->c_cc[VMIN] = 1, (buf)->c_cc[VTIME] = 0)	/* OOPS!  2/3/87 ++bsa */
#else
#  include <sgtty.h>
#  ifdef V7
#    define SETPARAMS(fd, buf)	gtty(fd, buf)
#    define SETPARAMS(fd, buf)	stty(fd, buf)
#    define TERMPARAMS		struct sgttyb
#    define TERM_CHARMODE(buf)	((buf)->sg_flags &= ~ECHO, (buf)->sg_flags |= RAW)	/* OOPS!  2/3/87 ++bsa */
#  else
#    include <sys/ioctl.h>
#    define GETPARAMS(fd, buf)	(ioctl(fd, TIOCGETP, &((*(buf)).__tp)), ioctl(fd, TIOCGETC, &((*(buf)).__tc)))
#    define SETPARAMS(fd, buf)	(ioctl(fd, TIOCSETN, &((*(buf)).__tp)), ioctl(fd, TIOCSETC, &((*(buf)).__tc)))
#    define TERMPARAMS		struct { struct sgttyb __tp; struct tchars __tc; }
#    define TERM_CHARMODE(buf)	((buf)->__tp.sg_flags &= ~ECHO, (buf)->__tp.sg_flags |= CBREAK, (buf)->__tc.t_intrc = '\003')	/* OOPS!  2/387 ++bsa */
#  endif V7
#endif SYS3

static int __pager = -1, __wrap, __bwrap, __col, __didwrp, __suppsp, __echo;
static TERMPARAMS __oldterm, __newterm;
static char __buf[133];
static char *__bufp;
static char __so_buf[BUFSIZ];

io_on(flag) {
	setbuf(stdout, __so_buf);
	__pager = 1;
	__wrap = 1;
	__echo = 1;
	__bwrap = 1;
	__didwrp = 0;
	__col = 0;
	__suppsp = 0;
	__bufp = __buf;
	if (flag) {
		user.u_lines = 16;
		user.u_llen = 80;
	}
	if (GETPARAMS(fileno(stdin), &__oldterm) < 0)
		return;
	__newterm = __oldterm;
	TERM_CHARMODE(&__newterm);
	SETPARAMS(fileno(stdout), &__newterm);
}

io_off() {
	if (__pager == -1)
		return;
	__flushwd();
	SETPARAMS(fileno(stdout), &__oldterm);
}

writec(ch)
register char ch; {
	register int cnt;
	register char *cp;

	ch &= 0x7f;
	if (ch == '\t') {
		do {
			writec(' ');
		} while (__col % 8 != 0);
		return;
	}
	if (ch < ' ' && ch != '\r' && ch != '\n' && ch != '\b') {
		writec('^');
		writec(uncntrl(ch));
		return;
	}
	if (ch == '\177') {
		writec('^');
		writec('?');
		return;
	}
	if (!__wrap) {
		__outch(ch);
		return;
	}
	__didwrp = 0;
	if (!__bwrap) {
		if (__col == linelen - 1 && ch != '\b' && ch != '\r' && ch != '\n') {
			for (cnt = 0; &__buf[cnt] < __bufp; cnt++)
				__didwrp++;
			for (cp = __bufp - 1; cp >= __buf; cp--) {
				__outch('\b');
				__outch(' ');
				__outch('\b');
				if (*cp == '\177' || *cp < ' ') {
					__outch('\b');
					__outch(' ');
					__outch('\b');
				}
			}
			__outch('\n');
			__suppsp = 1;
			for (cp++; cp < __bufp; cp++)
				__outch(*cp);
		}
		__outch(ch);
		fflush(stdout);
	}
	if (ch == ' ' || ch == '\n') {
		if (__bwrap)
			__flushwd();
		__bufp = __buf;
		if (__bwrap) {
			if (ch == ' ') {
				if (!__suppsp)
					__outch(' ');
			}
			if (ch == '\n')
				__outch('\n');
		}
		return;
	}
	__suppsp = 0;
	*__bufp = '\0';
	if (__bwrap && strlen(__buf) == linelen - 1) {
		__outch('\n');
		__suppsp = 1;
		for (__bufp = __buf; *__bufp != '\0'; __bufp++)
			if (*__bufp != ' ' || !__suppsp) {
				__outch(*__bufp);
				__suppsp = 0;
			}
		__outch('\n');
		__bufp = __buf;
		__suppsp = 1;
		return;
	}
	*__bufp++ = ch;
}

__outch(ch)
register char ch; {
	switch (ch) {
	case '\n':
		putchar('\r');
		putchar('\n');
		__col = 0;
		if (pagelen > 0 && __pager > 0 && ++__pager == pagelen) {
			fputs("--More--", stdout);
			fflush(stdout);
			ch = getchar();
			__pager = 1;
			fputs("\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b", stdout);
		}
		fflush(stdout);
		__suppsp = 0;
		break;
	case '\r':
		putchar('\r');
		__col = 0;
		__suppsp = 0;
		break;
	case '\b':
		if (__col == 0)
			break;
		putchar('\b');
		__col--;
		__suppsp = 0;
		break;
	default:
		if (__col == linelen - 1) {
			__outch('\n');
			__suppsp = 1;
		}
		if (ch < ' ' || ch > '~')
			putchar('.');
		else if (ch != ' ' || !__suppsp) {
			putchar(ch);
			__suppsp = 0;
		}
		__col++;
		break;
	}
}

writes(str)
register char *str; {
	for (; *str != '\0'; str++)
		writec(*str);
	writec('\n');
}

/*VARARGS*/
writef(va_alist)
va_dcl {
	register va_list args;
	register char *fmt;
	register char esch;
	register short esclen;
	short lzflag, ljflag, width, prec, longf, precf;
	
	va_start(args);
	for (fmt = va_arg(args, char *); *fmt != '\0'; fmt++) {
		if (*fmt == '\\')
			switch (*++fmt) {
			case '\0':
				va_end(args);
				return;
			case 'n':
				writec('\n');
				break;
			case 't':
				writec('\t');
				break;
			case 'r':
				writec('\r');
				break;
			case 'b':
				writec('\b');
				break;
			case 'f':
				writec('\f');
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '7':
				esch = '\0';
				for (esclen = 0; esclen < 3; esclen++) {
					esch = esch << 3;
					esch += *fmt - '0';
					if (*++fmt != '0' &&
					      *fmt != '1' &&
					      *fmt != '2' &&
					      *fmt != '3' &&
					      *fmt != '4' &&
					      *fmt != '5' &&
					      *fmt != '6' &&
					      *fmt != '7')
					      	break;
				}
				writec(esch);
				break;
			default:
				writec(*fmt);
				break;
			}
		else if (*fmt != '%')
			writec(*fmt);
		else {
			lzflag = 0;
			ljflag = 0;
			width = 0;
			prec = 0;
			longf = 0;
			precf = 0;

morefmt:
			switch (*++fmt) {
			case '\0':
				writec('%');
				va_end(args);
				return;
			case 'c':
				__fmtc(va_arg(args, int), ljflag, width);
				break;
			case 'd':
				if (longf)
					__fmti(va_arg(args, long), lzflag, ljflag, width, 10);
				else
					__fmti((long) va_arg(args, int), lzflag, ljflag, width, 10);
				break;
			case 'x':
				if (longf)
					__fmti(va_arg(args, long), lzflag, ljflag, width, 16);
				else
					__fmti((long) va_arg(args, int), lzflag, ljflag, width, 16);
				break;
			case 'o':
				if (longf)
					__fmti(va_arg(args, long), lzflag, ljflag, width, 8);
				else
					__fmti((long) va_arg(args, int), lzflag, ljflag, width, 8);
				break;
			case 's':
				__fmts(va_arg(args, char *), ljflag, width, prec);
				break;
			case 'f':
				__fmtf(va_arg(args, double), ljflag, width, prec);
				break;
			case 'e':
				__fmte(va_arg(args, double), ljflag, width, prec);
				break;
			case 'g':
				__fmtg(va_arg(args, double), ljflag, width, prec);
				break;
			case 'l':
				if (longf)
					break;
				longf = 1;
				goto morefmt;
			case '-':
				if (precf || width > 0 || lzflag || ljflag)
					break;
				ljflag = 1;
				goto morefmt;
			case '*':
				if (!precf)
					if (width != 0) {
						writec('*');
						break;
					}
					else
						width = va_arg(args, int);
				else if (prec != 0) {
					writec('*');
					break;
				}
				else
					prec = va_arg(args, int);
				goto morefmt;
			case '0':
				if (!precf && width == 0)
					lzflag = 1;
				else if (precf)
					prec *= 10;
				else
					width *= 10;
				goto morefmt;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (precf) {
					prec *= 10;
					prec += *fmt - '0';
				}
				else {
					width *= 10;
					width += *fmt - '0';
				}
				goto morefmt;
			case '.':
				if (precf)
					break;
				precf = 1;
				goto morefmt;
			default:
				break;
			}
		}
	}
	va_end(args);
}

__fmtc(ch, ljflag, width)
register int width; {
	if (width > 255)
		width = 255;
	if (width < 2) {
		writec(ch);
		return;
	}
	width--;
	if (!ljflag)
		while (width-- > 0)
			writec(' ');
	writec(ch);
	if (ljflag)
		while (width-- > 0)
			writec(' ');
}

__fmts(str, ljflag, width, prec)
register char *str;
register int width, prec; {
	register int len;

	if (str == (char *) 0)
		str = "(null)";
	if (prec == 0)
		prec = strlen(str);
	for (len = 0; str[len] != '\0' && len < prec; len++)
		;
	if (width < len)
		width = 0;
	else
		width -= len;
	if (!ljflag)
		while (width-- > 0)
			writec(' ');
	while (len-- > 0)
		writec(*str++);
	if (ljflag)
		while (width-- > 0)
			writec(' ');
}

__fmti(num, lzflag, ljflag, width, base)
long num;
register int width; {
	char buf[19];
	char *bufp, *dp;
	int sign;
	static char digit[] = "0123456789ABCDEF";
	
	sign = 0;
	if (num < 0L) {
		num = -num;
		sign = 1;
	}
	if (width > 18)
		width = 18;
	bufp = &buf[width? width: 18];
	*bufp-- = '\0';
	while (bufp >= buf) {
		*bufp-- = digit[num % base];
		num /= base;
	}
	for (bufp = buf; *bufp == '0'; bufp++)
		;
	if (*bufp == '\0')
		bufp--;
	width -= strlen(bufp) + sign;
	if (width < 0)
		width = 0;
	if (lzflag)
		ljflag = 0;
	if (!ljflag)
		while (width-- > 0)
			writec(lzflag? '0': ' ');
	if (sign)
		writec('-');
	while (*bufp != '\0')
		writec(*bufp++);
	if (ljflag)
		while (width-- > 0)
			writec(' ');
}

__fmte(num, ljflag, width, prec)
double num;
register int width; {
	char buf[20];
	int isneg, expon;
	register char *bufp;
	
	if (width > 18)
		width = 18;
	strcpy(buf, ecvt(num, (prec? prec: 18) - 6 - (num < 0.0), &expon, &isneg));
	if (prec == 0)
		for (bufp = &buf[12 - (num < 0.0? 1: 0)]; bufp != buf && *bufp != '\0'; bufp--)
			*bufp = '\0';
	if (width < strlen(buf) + 6 + (num < 0.0? 1: 0))
		width = strlen(buf) + 6 + (num < 0.0? 1: 0);
	width -= strlen(buf) + 6 + (num < 0.0);
	if (!ljflag)
		while (width-- > 0)
			writec(' ');
	if (isneg)
		writec('-');
	writec('.');
	for (bufp = buf; *bufp != '\0'; bufp++)
		writec(*bufp);
	writec('E');
	writec(expon < 0? '-': '+');
	__fmti((long) expon, 1, 0, 2, 10);
	if (ljflag)
		while (width-- > 0)
			writec(' ');
}

__fmtf(num, ljflag, width, prec)
double num;
register int width; {
	char buf[40];
	int isneg, expon;
	register char *bufp;
	
	if (width > 18)
		width = 18;
	strcpy(buf, ecvt(num, prec, &expon, &isneg));
	if (width < strlen(buf) + 1 + (num < 0.0))
		width = strlen(buf) + 1 + (num < 0.0);
	width -= strlen(buf) + 1 + (num < 0.0);
	if (!ljflag)
		while (width-- > 0)
			writec(' ');
	if (isneg)
		writec('-');
	for (bufp = buf; *bufp != '\0'; bufp++) {
		if (expon-- == 0)
			writec('.');
		writec(*bufp);
	}
	if (expon == 0)
		writec('.');
	if (ljflag)
		while (width-- > 0)
			writec(' ');
}

__fmtg(num, ljflag, width, prec)
double num;
register int width; {
	char buf[40];
	register char *bufp;
	
	if (width > 18)
		width = 18;
	strcpy(buf, gcvt(num, prec, buf));
	if (width < strlen(buf))
		width = strlen(buf);
	width -= strlen(buf);
	if (!ljflag)
		while (width-- > 0)
			writec(' ');
	for (bufp = buf; *bufp != '\0'; bufp++)
		writec(*bufp);
	if (ljflag)
		while (width-- > 0)
			writec(' ');
}

char *reads(buf)
char *buf; {
	short bp;
	int savecol, rpos;
	char ch;
	
	savecol = __col;
	__pager = 0;
	__wrap = 0;
	__flushwd();
	bp = 0;
	fflush(stdout);
	while ((ch = getchar() & 0x7f) != '\n' && ch != '\r') {
		switch (ch) {
		case '\177':
		case '\b':
			if (bp == 0)
				putchar('\7');
			else {
				bp--;
				if (__echo) {
					if (__col > 0) {
						writec('\b');
						writec(' ');
						writec('\b');
					}
					else {
						if (bp - linelen < -1) {
							while (__col < savecol)
								writec(' ');
							rpos = 0;
						}
						else
							rpos = bp - linelen + 1;
						while (rpos < bp)
							writec(buf[rpos++]);
					}
				}
			}
			break;
		case '\030':
		case '\025':
			if (__echo) {
				writec('X');
				writec('X');
				writec('X');
				writec('\n');
				while (__col < savecol)
					writec(' ');
			}
			bp = 0;
			break;
		default:
			if (ch < ' ' || bp == 255)
				putchar('\7');
			else {
				if (__echo)
					__outch(ch);
				buf[bp++] = ch;
			}
		}
		fflush(stdout);
	}
	writec('\n');
	fflush(stdout);
	__wrap = 1;
	__pager = 1;
	buf[bp] = '\0';
	return buf;
}

interact() {
	__flushwd();
	__bwrap = 0;
	__pager = 0;
}

buffer() {
	__bwrap = 1;
	__bufp = __buf;
	__pager = 1;
}

wrapped() {
	return __didwrp;
}

__flushwd() {
	*__bufp = '\0';
	if (strlen(__buf) >= linelen - __col) {
		__outch('\n');
		__suppsp = 1;
	}
	for (__bufp = __buf; *__bufp != '\0'; __bufp++)
		if (*__bufp != ' ' || !__suppsp) {
			__outch(*__bufp);
			__suppsp = 0;
		}
	__bufp = __buf;
	fflush(stdout);
}

doecho() {
	__echo = 1;
}

xecho() {
	__echo = 0;
}

cat(file)
char *file; {
	FILE *f;
	int ch;

	if ((f = fopen(file, "r")) == NULL) {
		log("Error %d opening %s", errno, file);
		writes("Cannot open file.");
		return;
	}
	while ((ch = getc(f)) != EOF)
		writec(ch);
	fclose(f);
}

readc() {
	char ch;
	
	__flushwd();
	__pager = 0;
	while (((ch = getchar() & 0x7f) < ' ' && ch != '\r' && ch != '\n') || ch == '\177') {
		putchar('\7');
		fflush(stdout);
	}
	if (ch > ' ') {
		ch = ToUpper(ch);
		writec(ch);
	}
	else
		ch = ' ';
	writec('\n');
	fflush(stdout);
	__pager = 1;
	return ch;
}
