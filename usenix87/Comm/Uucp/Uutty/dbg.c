/* John Chambers' debugging package for C programs.  This
** package is intended, among other things, to be added to
** other peoples' software after the fact to aid in getting
** it to run.  An attempt has been made to avoid global 
** names that others will use, though there are likely
** to be collisions with other debugging packages.  The
** most common source of conflicts is due to the fact that
** this package includes <stdio.h> and <sys/types.h>; you
** should hunt down references to either of these and put
** the line:
**	#include <dbg.h>
** in place of both of them.
**
** The primary functions defined here are based on conditional 
** calls of printf(), with the program's name inserted at the 
** start, a newline appended, and fflush() called to send it 
** on its way.  There is also a hexdump() routine to produce
** a hex dump of a chunk of memory, and an ascdump() routine
** to display things in ASCII form.  All these are intended 
** to be called via the macros defined in "dbg.h".
**
** The global variable "debug" should be set to a value in [0,9],
** to control the amount of debugging output.
**
** Note that "dbgout" is a FILE*, either stdout or stderr as you
** wish, with stderr the default.
*/
#include "dbg.h"

static char dbgheader[] = "@(#)RELEASE: 1.0  Sep 20 1986 ~jc/dbg";
/* 
** If you normally use terminals with a particular line
** length, you might adjust these.  Note that there is
** an initial 7-byte fieldin all lines, plus a newline,
** so MAXCOLS should be >= MAXCHS+8.
*/
#define MAXCOLS 80		/* Max length of print line */
#define MAXCHRS 64		/* Max chars in one hexdump chunk */

extern int errno;			/* Unix error code */
	char  *crlf   = "\n";		/* Line terminator string, might be "\n\r" */
	FILE  *dbgout = stderr;		/* Can be assigned to switch to stdout */
	int    dbgsleep = 0;		/* Sleep time (secs) after messages */
	int    debug  = 1;		/* Set to desired debug level (0-9) */
	char *dbgtimep  = 0;		/* Set to ASCII date/time if desired in output */
static	int    hex_chrs = MAXCHRS;	/* Number of chars to dump in one line */
static	int    hex_cols = MAXCOLS;	/* Length of print line */
static	int    hex_word = MAXCHRS;	/* Number of bytes to group together */
	char  *progname = "?";		/* Set to argv[0] */
/*
** Debug output routine.  The program's name is
** prepended to the message, a terminator is added,
** and an optional sleep is done.
*/
/*VARARGS1*//*ARGSUSED*/
dmsg(fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9) char *fmt;
{	int n;
	n  = fprintf(dbgout,"%s: ",progname);
	n += fprintf(dbgout,fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	n += fprintf(dbgout,crlf);
	fflush(dbgout);
	if (dbgsleep) sleep(dbgsleep);
	return n;
}
/* Note that emsg() writes to both dbgout and stderr.
*/
/*VARARGS1*//*ARGSUSED*/
emsg(fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9) char *fmt;
{   int n;
	if (dbgout != stderr)
		n = dmsg(fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	n += fprintf(stderr,"%s: ",progname);
	n += fprintf(stderr,fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	n += fprintf(stderr,crlf);
	fflush(stderr);
	return n;
}
/* Print a message to the debug output, adding only
** a terminator, but don't add program identification.
*/
/*VARARGS1*//*ARGSUSED*/
pmsg(fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9) char *fmt;
{   int n;
	n  = fprintf(dbgout,fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	n += fprintf(dbgout,crlf);
	fflush(dbgout);
	if (dbgsleep) sleep(dbgsleep);
	return n;
}
/* Write a message to the debug output, adding nothing.
*/
/*VARARGS1*//*ARGSUSED*/
wmsg(fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9) char *fmt;
{   int n;
	n  = fprintf(dbgout,fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	fflush(dbgout);
	if (dbgsleep) sleep(dbgsleep);
	return n;
}
/* 
** Hexdump routine.  The params are a starting address,
** a byte count, an address to show on the output (which 
** is often the starting address), and an optional message
** for labeling the dump.  If the message is present, the
** address is not displayed, since they both would appear
** at the same place in the output.
**
** Note that the actual output is done by pmsg().
*/
static char  hex_cc[1+MAXCOLS];
static char  hex_hi[1+MAXCOLS];
static char  hex_lo[1+MAXCOLS];
static int   ascfl = 0;			/* True if hex dump not wanted */
/*
** Produce only the ASCII line; omit the two hex lines.
*/
ascdump(a,n,o,m)
	char *a;	/* Address of first byte to dump */
	U32   n;	/* Number of bytes */
	char *o;	/* Offset (address) to report */
	char *m;	/* Initial message, if o==NUL */
{	int   c;

	D4("ascdump(a=%06lX,n=%d,o=%06lX,m=%06lX)",a,n,o,m);
	if (dbgtimep) fprintf(dbgout,"%s ",dbgtimep);
	if (m) sprintf(hex_cc,"%s            ",m);
	  else sprintf(hex_cc,"%7d           ",o);
	fprintf(dbgout,"%7.7s",hex_cc);
	fflush(dbgout);
	while (n-- > 0) {
	  c = *a++ & 0x7F;
	  switch (c) {
	  case '\0': fprintf(dbgout,"\\0"); break;
	  case '^' : fprintf(dbgout,"\\^"); break;
	  case '\\': fprintf(dbgout,"\\\\"); break;
	  case '\b': fprintf(dbgout,"\\b"); break;
	  case '\n': fprintf(dbgout,"\\n"); break;
	  case '\r': fprintf(dbgout,"\\r"); break;
	  case '\t': fprintf(dbgout,"\\t"); break;
	  case 0x7F: fprintf(dbgout,"\\D"); break;
	  default: 
	    if (c < ' ') {
	      fprintf(dbgout,"^%c",c|0x40);
	      break;
	    }
	    fprintf(dbgout,"%c",c);
	    break;
	  }
	}
	fprintf(dbgout,crlf);
	fflush(dbgout);
}
/*
*/
hexdump(a,n,o,m)
	char *a;	/* Address of first byte to dump */
	U32   n;	/* Number of bytes */
	U32   o;	/* Offset (address) to report */
	char *m;	/* Initial message, if o==NUL; -1 for ascii only */
{
	ascfl = 0;
	return hex_dump(a,n,o,m);
}
/*
*/
hex_dump(a,n,o,m)
	char *a;	/* Address of first byte to dump */
	U32   n;	/* Number of bytes */
	U32   o;	/* Offset (address) to report */
	char *m;	/* Initial message, if o==NUL; -1 for ascii only */
{	int   col, i;
	char  c;

	hex_init(m,o);
	col = 0;
	while (n) {	
		c = *a++;
		i = 7 + col + (col / hex_word);
		n--;
		hex_lo[i] = hex_dig(c & 0xF);
		hex_hi[i] = hex_dig((c >> 4) & 0xF);
		hex_cc[i] = dsp(c);
		col++;
		o++;
		if (col >= hex_chrs) {	
			hex_out();
			hex_init(m,o);
			col = 0;
		}
	}
	if (col > 0) hex_out();
	return 0;
}
/* Generate a printable char for x.  If x is an ASCII
** printable char, it is returned.  For others, '_' is
** returned.
*/
dsp(x) 
{	x &= 0x7F;
	return (0x20<=x && x<=0x7E) ? x : '_';
}
/* Given a small integer [0-15], this routine returns
** the corresponding ASCII char for a hex digit.
*/
hex_dig(x) 
{	return  ( 0<=x && x<= 9) ? '0' + x 
	: (10<=x && x<=15) ? 'A' + x - 10 
	: '_';
}
/* Initialize the line images for a given address.
*/
hex_init(m,a)
	char *m;	/* Initial message (<=8 chars) */
	U32   a;	/* Address reported on output */
{	int   i;

	i = hex_chrs + (hex_chrs / hex_word) + 8;	/* Length of print line (including final newline) */
	if (hex_cols < i) {
		hex_cols = i;
		D5("hex_init: cols=%d chrs=%d word=%d",hex_cols,hex_chrs,hex_word);
	}
	for (i=0; i<=hex_cols; i++)
		hex_cc[i] = hex_hi[i] = hex_lo[i] = ' ';
	if (m && a==0) {
		strcpy(hex_cc,m);
	} else {
		sprintf(hex_cc,"%6ld: ",a);
		sprintf(hex_hi,"%6lX: ",a);
	}
	return 0;
}
/* The three lines of hex dump info are complete;
** do a bit of straightening out, and print them.
*/
hex_out()
{	int i;

	for (i=0; i<=hex_cols; i++) {		/* sprintf() may produce nulls */
		if (hex_cc[i] == 0) hex_cc[i] = ' ';
		if (hex_hi[i] == 0) hex_hi[i] = ' ';
		if (hex_lo[i] == 0) hex_lo[i] = ' ';
	}
	for (i=hex_cols; i>0; i--) 			/* Trim lines */
		if (hex_cc[i] == ' ') 
			hex_cc[i] = 0;
		else break;
	for (i=hex_cols; i>0; i--) 
		if (hex_hi[i] == ' ') 
			hex_hi[i] = 0;
		else break;
	for (i=hex_cols; i>0; i--) 
		if (hex_lo[i] == ' ') 
			hex_lo[i] = 0;
		else break;
	if (dbgtimep) fprintf(dbgout,"%s ",dbgtimep);
	pmsg("%s",hex_cc);			/* Symbolic dump */
	if (ascfl == 0) {
	    if (dbgtimep) fprintf(dbgout,"%s ",dbgtimep);
		pmsg("%s",hex_hi);		/* High-order hex digit */
	    if (dbgtimep) fprintf(dbgout,"%s ",dbgtimep);
		pmsg("%s",hex_lo);		/* Low-order hex digit */
	}
	if (dbgsleep) sleep(dbgsleep);
	return 0;
}
