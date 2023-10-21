#include "uutty.h"
/* 
** Write a character string to the port, stopping at the first null,
** and processing a variety of escape sequences.  This routine always
** writes one byte at a time.  Doing a system call per byte is a bit
** wasteful of processor time, but it helps assure that we won't flood
** the poor little helpless modem.
*/
#define Gotbyte goto gotbyte 

awrite(msg)
	char*msg;
{	int  i, n;
	char c, d, *p;

	D5("awrite(%08lX)",msg);
	if (debug) {
		dbgtimep = getime();
		if (debug >= 2) P("%s Send:   %s",dbgtimep,msg);
		if (debug >= 4) Hexdnm(msg,1,"Send:");
	}
	n = strlen(msg);
	D8("port_wr:slow=%d",slow);
	p = msg;
	while (c = *p++) {
		switch (c) {
		case '^':		/* CTRL-X notation */
			c = *p++ & 0x3F;
			Gotbyte;
		case '%':		/* %AB is a hex value */
			c = 0;
			n = 2;		/* Accept at most 2 digits */
			while (n-- > 0) {
				switch (d = *p) {
				case '9': case '8': case '7': case '6': case '5': 
				case '4': case '3': case '2': case '1': case '0':
					c = (c << 4) | (c - '0');
					Gotbyte;
				case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
					c = (c << 4) | (c - 'A' + 10);
					Gotbyte;
				case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
					c = (c << 4) | (c - 'a' + 10);
					Gotbyte;
				default:
					goto gotbyte;
				}
				++p;
			}
			Gotbyte;
		case '\\':		/* Escape notation */
			switch (c = *p++) {
			case '7': case '6': case '5': case '4':
			case '3': case '2': case '1': case '0':
				c -= '0';
				if ((d = *p) && ('0'<=d && d<='7')) {
					c = (c << 3) | (d - '0');
					++p;
					if ((d = *p) && ('0'<=d && d<='7')) {
						c = (c << 3) | (d - '0');
						++p;
					}
				}
				Gotbyte;
			case 'B': case 'b': c = '\b'; Gotbyte;
			case 'D': case 'd': sleep(1); continue;
			case 'N': case 'n': c = '\n'; Gotbyte;
			case 'R': case 'r': c = '\r'; Gotbyte;
			case 'T': case 't': c = '\t'; Gotbyte;
			case 'X': case 'x': sendbrk(dev); continue;
			default : c = d; Gotbyte;
			}
		}
gotbyte:	Slowly;
		if (debug >= 3) {
			dbgtimep = getime();
			if (debug >= 3) Ascdnm(&c,1,"Write:");
			if (debug >= 4) Hexdnm(&c,1,"Write:");
		}
		D4("awrite: c=%02X='%c'",c,dsp(c));
		D9("port_wr:before write(%d,%06lX,%d)",dev,&c,1);
		i = write(dev,&c,1);
		D9("port_wr: after write(%d,%06lX,%d)=%d",dev,&c,1,i);
		if (i <= 0) {
			if (debug) P("%s: write failed, quitting.",getime());
			die(2);
		}
	}
}
