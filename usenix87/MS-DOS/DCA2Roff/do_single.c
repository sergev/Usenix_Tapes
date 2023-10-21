#include "dca2troff.h"

/* single byte control character */
do_single(c)
int c;
{
    switch (c)
	{
	case 0x05:			/* "HT"		tab */
		in_fcr = 1;
		outstr("\t");
		return;
	case 0x33:			/* "IRT"	index return */
	case 0x06:			/* "RCR"	required carrier ret */
		outstr("\n.br\n");
		if(in_it != 0)
			outstr(".in 0\n");
		in_it == 0;			/* reset in_it */
		return;
	case 0x09:			/* "SPS"	superscript */
		outstr("\\u");			/* up half a line */
		return;
	case 0x0c:			/* "PE"		page end */
		outstr("\n");
		return;
	case 0x0d:			/* "ZICR"	zero index carrier
							return */
		return;				/* go to pos 0 on current
						line */
	case 0x15:			/* "CRE"	carrier return */
		if ( in_fcr == 1)
			outstr("\n.br\n");		/* forced cr */
		else
			outstr("\n");			/* just an eol */
		in_fcr = 0;				/* reset in_fcr */
		return;
	case 0x16:			/* "BS"		backspace */
		outstr("\b");
		return;
	case 0x1a:			/* "UBS"	unit backspace */
		outstr("\b");
		return;
	case 0x23:			/* "WUS"	word underscore */
						/* back to last word break */
		printf("\\fI");			/* start the underscore */
		outachar('\177');		/* flush last word */
		printf("\\fR");			/* back to roman */
		return;
	case 0x25:			/* "INX"	index */
		return;				/* line feed? */
	case 0x36:			/* "NBS"	numeric backspace */
		outstr("\\h'-\\w'1'u'");
		return;
	case 0x38:			/* "SBS"	subscript */
		outstr("\\d");			/* down half a line */
		return;
	case 0x39:			/* "IT"		indent tab */
		in_fcr = 1;
		in_it = 1;
		outstr("\n'in+.5i\n");		/* move indent plus a stop */
		return;
	case 0x3a:			/* "RPE"	required page end */
		outstr("\n.bp\n");
		return;
	case 0x3f:			/* "SUB"	substitute */
		outstr("_");			/* inserted in place of err */
		return;
	case 0x40:			/* "SP"		space */
	case 0x41:
		outstr(" ");		/* "RSP"	required space */
		return;
	case 0x60:			/* "HYP"	required hyphen */
		outstr("-");
		return;
	case 0xca:			/* "SHY"	syllable hyphen */
		return;
	case 0xe1:			/* "NSP"	numeric space */
		outstr("\\h'\\w'1'u'");
		return;
	default:
		fprintf(stderr, "unknown single char code (x%02x)\n", c);
		exit(1);;
	}
}
