#include "uutty.h"
/*
** Read a response from the debugger into a buffer.  Note the three
** stop conditions: finding an EOL char; filling the buffer; timeout.
*/
pread(stopch,bp,bs)
	int  stopch;
	char*bp;
	int  bs;
{	int  c, n;
	uint reads;

	D4("pread(stopch=%02X,bp=%06lX,bs=%d)",stopch,bp,bs);
	n = reads = 0;
	while (n < bs) {
		c = nextbyte();		/* Next char from port */
		if (c <= 0) {		/* No data returned? */
			if (debug >= 4)
				if (reads) P("pread: %d reads",reads);
			if (++reads > l_reads) {
				errno = ETIMEOUT;	/* Timeout if too many */
				D3("Input timeout...");
				Fail;			/* Timeout; return what we have */
			}
			continue;
		}
		D9("pread: c=%02X='%c' bp=%06lX bs=%d",c,dsp(c),bp,bs);
		c &= iomask;		/* Trim it to 7 bits */
		if (c == 0) continue;	/* Ignore nulls */
		*bp++ = c;			/* Note the EOL char is returned */
		++n;			/* Count the input chars */
		--bs;			/* Decr count of bytes wanted */
		if (c == stopch) Done;	/* Assorted EOL chars */
		switch (c) {
			case 0x03:		/* ^C = ETX */
				if (debug) P("%s: ^C in input, quitting [id]",getime());
				die(0);
			case '>' :
			case '\r':
			case '\n':
			case ':' :
			case '%' :
			case 0x04:		/* ^D = EOT  */
			case 0x06:		/* ^D = ACK  */
			case 0x15:		/* ^U = NAK */
				Done;
		}
		reads = 0;			/* Got data; reset timeout counter */
	}
done:
	*bp = 0;			/* Final null for debugging */
fail:
	return n;
}
