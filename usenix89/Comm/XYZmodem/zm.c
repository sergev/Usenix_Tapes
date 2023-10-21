/*
 *   Z M . C
 *    ZMODEM protocol primitives
 *    8-16-86  Chuck Forsberg Omen Technology Inc
 *
 * Entry point Functions:
 *	zsbhdr(type, hdr) send binary header
 *	zshhdr(type, hdr) send hex header
 *	zgethdr(hdr, eflag) receive header - binary or hex
 *	zsdata(buf, len, frameend) send data
 *	zrdata(buf, len) receive data
 *	stohdr(pos) store position data in Txhdr
 *	long rclhdr(hdr) recover position offset from header
 */

#ifndef CANFDX
#include "zmodem.h"
int Rxtimeout = 100;		/* Tenths of seconds to wait for something */
#endif

static char *frametypes[] = {
	"CARRIER LOST",		/* -3 */
	"TIMEOUT",		/* -2 */
	"ERROR",		/* -1 */
#define FTOFFSET 3
	"ZRQINIT",
	"ZRINIT",
	"ZSINIT",
	"ZACK",
	"ZFILE",
	"ZSKIP",
	"ZNAK",
	"ZABORT",
	"ZFIN",
	"ZRPOS",
	"ZDATA",
	"ZEOF",
	"ZFERR",
	"ZCRC",
	"ZCHALLENGE",
	"ZCOMPL",
	"ZCAN",
	"ZFREECNT",
	"ZCOMMAND",
	"ZSTDERR",
	"xxxxx"
#define FRTYPES 22	/* Total number of frame types in this array */
			/*  not including psuedo negative entries */
};

/* Send ZMODEM binary header hdr of type type */
zsbhdr(type, hdr)
register char *hdr;
{
	register n;
	register unsigned short oldcrc;

	vfile("zsbhdr: %s %lx", frametypes[type+FTOFFSET], rclhdr(hdr));
	xsendline(ZPAD); xsendline(ZDLE); xsendline(ZBIN);

	zsendline(type); oldcrc = updcrc(type, 0);

	for (n=4; --n >= 0;) {
		zsendline(*hdr);
		oldcrc = updcrc((0377& *hdr++), oldcrc);
	}
	oldcrc = updcrc(0,updcrc(0,oldcrc));
	zsendline(oldcrc>>8);
	zsendline(oldcrc);
	if (type != ZDATA)
		flushmo();
}

/* Send ZMODEM HEX header hdr of type type */
zshhdr(type, hdr)
register char *hdr;
{
	register n;
	register unsigned short oldcrc;

	vfile("zshhdr: %s %lx", frametypes[type+FTOFFSET], rclhdr(hdr));
	sendline(ZPAD); sendline(ZPAD); sendline(ZDLE); sendline(ZHEX);
	zputhex(type);

	oldcrc = updcrc(type, 0);
	for (n=4; --n >= 0;) {
		zputhex(*hdr); oldcrc = updcrc((0377& *hdr++), oldcrc);
	}
	oldcrc = updcrc(0,updcrc(0,oldcrc));
	zputhex(oldcrc>>8); zputhex(oldcrc);

	/* Make it printable on remote machine */
	sendline(015); sendline(012);
	/*
	 * Uncork the remote in case a fake XOFF has stopped data flow
	 */
	if (type != ZFIN)
		sendline(021);
	flushmo();
}

/*
 * Send binary array buf of length length, with ending ZDLE sequence frameend
 */
zsdata(buf, length, frameend)
register char *buf;
{
	register unsigned short oldcrc;

	vfile("zsdata: length=%d end=%x", length, frameend);
	oldcrc = 0;
	for (;--length >= 0;) {
		zsendline(*buf);
		oldcrc = updcrc((0377& *buf++), oldcrc);
	}
	xsendline(ZDLE); xsendline(frameend);
	oldcrc = updcrc(frameend, oldcrc);

	oldcrc = updcrc(0,updcrc(0,oldcrc));
	zsendline(oldcrc>>8);
	zsendline(oldcrc);
	if (frameend == ZCRCW) {
		xsendline(XON);  flushmo();
	}
}

/*
 * Receive array buf of max length with ending ZDLE sequence
 *  and CRC.  Returns the ending character or error code.
 */
zrdata(buf, length)
register char *buf;
{
	register c;
	register unsigned short oldcrc;
	register d;

	oldcrc = Rxcount = 0;
	for (;;) {
		if ((c = zdlread()) & ~0377) {
crcfoo:
			switch (c) {
			case GOTCRCE:
			case GOTCRCG:
			case GOTCRCQ:
			case GOTCRCW:
				oldcrc = updcrc((d=c)&0377, oldcrc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				oldcrc = updcrc(c, oldcrc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				oldcrc = updcrc(c, oldcrc);
				if (oldcrc & 0xFFFF) {
					zperr("Bad data CRC %x", oldcrc);
					return ERROR;
				}
				vfile("zrdata: cnt = %d ret = %x", Rxcount, d);
				return d;
			case GOTCAN:
				zperr("ZMODEM: Sender Canceled");
				return ZCAN;
			case TIMEOUT:
				zperr("ZMODEM data TIMEOUT");
				return c;
			default:
				zperr("ZMODEM bad data subpacket ret=%x", c);
				return c;
			}
		}
		if (--length < 0) {
			zperr("ZMODEM data subpacket too long");
			return ERROR;
		}
		++Rxcount;
		*buf++ = c;
		oldcrc = updcrc(c, oldcrc);
		continue;
	}
}

/*
 * Read a ZMODEM header to hdr, either binary or hex.
 *  eflag controls local display of non zmodem characters:
 *	0:  no display
 *	1:  display printing characters only
 *	2:  display all non ZMODEM characters
 *  On success, set Zmodem to 1 and return type of header.
 *   Otherwise return negative on error
 */
zgethdr(hdr, eflag)
char *hdr;
{
	register c, n, cancount;

	n = Baudrate;	/* Max characters before start of frame */
	cancount = 5;
again:
	Rxframeind = Rxtype = 0;
	switch (c = noxread7()) {
	case TIMEOUT:
		goto fifi;
	case CAN:
		if (--cancount <= 0) {
			c = ZCAN; goto fifi;
		}
	/* **** FALL THRU TO **** */
	default:
agn2:
		if ( --n == 0) {
			zperr("ZMODEM Garbage count exceeded");
			return(ERROR);
		}
		if (eflag && ((c &= 0177) & 0140))
			bttyout(c);
		else if (eflag > 1)
			bttyout(c);
		if (c != CAN)
			cancount = 5;
		goto again;
	case ZPAD:		/* This is what we want. */
		break;
	}
	cancount = 5;
splat:
	switch (c = noxread7()) {
	case ZPAD:
		goto splat;
	case TIMEOUT:
		goto fifi;
	default:
		goto agn2;
	case ZDLE:		/* This is what we want. */
		break;
	}

	switch (c = noxread7()) {
	case TIMEOUT:
		goto fifi;
	case ZBIN:
		Rxframeind = ZBIN;
		c =  zrbhdr(hdr);
		break;
	case ZHEX:
		Rxframeind = ZHEX;
		c =  zrhhdr(hdr);
		break;
	case CAN:
		if (--cancount <= 0) {
			c = ZCAN; goto fifi;
		}
		goto agn2;
	default:
		goto agn2;
	}
	Rxpos = hdr[ZP3] & 0377;
	Rxpos = (Rxpos<<8) + (hdr[ZP2] & 0377);
	Rxpos = (Rxpos<<8) + (hdr[ZP1] & 0377);
	Rxpos = (Rxpos<<8) + (hdr[ZP0] & 0377);
fifi:
	switch (c) {
	case GOTCAN:
		c = ZCAN;
	/* **** FALL THRU TO **** */
	case ZNAK:
	case ZCAN:
	case ERROR:
	case TIMEOUT:
	case RCDO:
		zperr("ZMODEM: Got %s %s", frametypes[c+FTOFFSET],
		  (c >= 0) ? "header" : "error");
	/* **** FALL THRU TO **** */
	default:
		if (c >= -3 && c <= FRTYPES)
			vfile("zgethdr: %s %lx", frametypes[c+FTOFFSET], Rxpos);
		else
			vfile("zgethdr: %d %lx", c, Rxpos);
	}
	return c;
}

/* Receive a binary style header (type and position) */
zrbhdr(hdr)
register char *hdr;
{
	register c, n;
	register unsigned short oldcrc;

	if ((c = zdlread()) & ~0377)
		return c;
	Rxtype = c;
	oldcrc = updcrc(c, 0);

	for (n=4; --n >= 0;) {
		if ((c = zdlread()) & ~0377)
			return c;
		oldcrc = updcrc(c, oldcrc);
		*hdr++ = c;
	}
	if ((c = zdlread()) & ~0377)
		return c;
	oldcrc = updcrc(c, oldcrc);
	if ((c = zdlread()) & ~0377)
		return c;
	oldcrc = updcrc(c, oldcrc);
	if (oldcrc & 0xFFFF) {
		zperr("Bad Header CRC"); return ERROR;
	}
	Zmodem = 1;
	return Rxtype;
}

/* Receive a hex style header (type and position) */
zrhhdr(hdr)
char *hdr;
{
	register c;
	register unsigned short oldcrc;
	register n;

	if ((c = zgethex()) < 0)
		return c;
	Rxtype = c;
	oldcrc = updcrc(c, 0);

	for (n=4; --n >= 0;) {
		if ((c = zgethex()) < 0)
			return c;
		oldcrc = updcrc(c, oldcrc);
		*hdr++ = c;
	}
	if ((c = zgethex()) < 0)
		return c;
	oldcrc = updcrc(c, oldcrc);
	if ((c = zgethex()) < 0)
		return c;
	oldcrc = updcrc(c, oldcrc);
	if (oldcrc & 0xFFFF) {
		zperr("Bad Header CRC"); return ERROR;
	}
	if (readline(1) == '\r')	/* Throw away possible cr/lf */
		readline(1);
	Zmodem = 1; return Rxtype;
}

/* Send a byte as two hex digits */
zputhex(c)
register c;
{
	static char	digits[]	= "0123456789abcdef";

	if (Verbose>4)
		vfile("zputhex: %x", c);
	sendline(digits[(c&0xF0)>>4]);
	sendline(digits[(c)&0xF]);
}

/*
 * Send character c with ZMODEM escape sequence encoding.
 *  Escape XON, XOFF. Escape CR following @ (Telenet net escape)
 */
zsendline(c)
register c;
{
	static lastsent;

	switch (c & 0377) {
	case ZDLE:
		xsendline(ZDLE);
		xsendline (lastsent = (c ^= 0100));
		break;
	case 015:
	case 0215:
		if ((lastsent & 0177) != '@')
			goto sendit;
	/* **** FALL THRU TO **** */
	case 020:
	case 021:
	case 023:
	case 0220:
	case 0221:
	case 0223:
#ifdef ZKER
		if (Zctlesc<0)
			goto sendit;
#endif
		xsendline(ZDLE);
		c ^= 0100;
sendit:
		xsendline(lastsent = c);
		break;
	default:
#ifdef ZKER
		if (Zctlesc>0 && ! (c & 0140)) {
			xsendline(ZDLE);
			c ^= 0100;
		}
#endif
		xsendline(lastsent = c);
	}
}

/* Decode two lower case hex digits into an 8 bit byte value */
zgethex()
{
	register c;

	c = zgeth1();
	if (Verbose>4)
		vfile("zgethex: %x", c);
	return c;
}
zgeth1()
{
	register c, n;

	if ((c = noxread7()) < 0)
		return c;
	n = c - '0';
	if (n > 9)
		n -= ('a' - ':');
	if (n & ~0xF)
		return ERROR;
	if ((c = noxread7()) < 0)
		return c;
	c -= '0';
	if (c > 9)
		c -= ('a' - ':');
	if (c & ~0xF)
		return ERROR;
	c += (n<<4);
	return c;
}

/*
 * Read a byte, checking for ZMODEM escape encoding
 *  including CAN*5 which represents a quick abort
 */
zdlread()
{
	register c;

	if ((c = readline(Rxtimeout)) != ZDLE)
		return c;
	if ((c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	switch (c) {
	case CAN:
		return GOTCAN;
	case ZCRCE:
	case ZCRCG:
	case ZCRCQ:
	case ZCRCW:
		return (c | GOTOR);
	case ZRUB0:
		return 0177;
	case ZRUB1:
		return 0377;
	default:
		if ((c & 0140) ==  0100)
			return (c ^ 0100);
		break;
	}
	zperr("Got bad ZMODEM escape sequence %x", c);
	return ERROR;
}

/*
 * Read a character from the modem line with timeout.
 *  Eat parity, XON and XOFF characters.
 */
noxread7()
{
	register c;

	for (;;) {
		if ((c = readline(Rxtimeout)) < 0)
			return c;
		switch (c &= 0177) {
		case XON:
		case XOFF:
			continue;
		default:
			return c;
		}
	}
}

/* Store long integer pos in Txhdr */
stohdr(pos)
long pos;
{
	Txhdr[ZP0] = pos;
	Txhdr[ZP1] = pos>>8;
	Txhdr[ZP2] = pos>>16;
	Txhdr[ZP3] = pos>>24;
}

/* Recover a long integer from a header */
long
rclhdr(hdr)
register char *hdr;
{
	register long l;

	l = (hdr[ZP3] & 0377);
	l = (l << 8) | (hdr[ZP2] & 0377);
	l = (l << 8) | (hdr[ZP1] & 0377);
	l = (l << 8) | (hdr[ZP0] & 0377);
	return l;
}

