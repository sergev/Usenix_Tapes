/*
**	1stlatex.w	1st_word to latex conversion program
**	file: 1stquote.c	handles hard to get characters
**	version 1.1
**
**	Jerome M. Lang
**	Public Domain
**	7 March 1985.
**
**	quote() - handles characters that cannot be simply passed through
*/
/*	1986-04-28 : add support for boldmath characters (lowercase bold
**			greek characters (shouldn't need to do that, YUK).
*/

#include "1stlatex.h"

static	boolean	debold;
extern	boolean isbold();

quote( c )	/* harder to get character or ones with special names */
int	c;
{
	debold = FALSE;

	switch( c )
	{
	  case '$':	outstr("\\$");break;
	  case '&':	outstr("\\&");break;
	  case '%':	outstr("\\%");break;
	  case '#':	outstr("\\#");break;
	  case '_':	outstr("\\_");break;
	  case '{':	outstr("\\{");break;
	  case '}':	outstr("\\}");break;

	  case '>':	outstr("$>$");break;
	  case '<':	outstr("$<$");break;
	  case '|':	outstr("$|$");break;

	  case '~':	outstr("{\\verb+~+}");break;
	  case '^':	outstr("{\\verb+^+}");break;
	  case '\\':	outstr("{\\verb+\\+}");break;

	  case 0x01:	outstr("$\\uparrow$");break;
          case 0x02:	outstr("$\\downarrow$");break;
	  case 0x03:	outstr("$\\rightarrow$");break;
	  case 0x04:	outstr("$\\leftarrow$");break;
	  case 0x7f:	outstr("$\\Delta$");break;

	  case 0x80:	outstr("\\c{C}");break;
	  case 0x81:	outstr("\\\"{u}");break;
	  case 0x82:	outstr("\\'{e}");break;
	  case 0x83:	outstr("\\^{a}");break;
	  case 0x84:	outstr("\\\"{a}");break;
	  case 0x85:	outstr("\\`{a}");break;
	  case 0x86:	outstr("\\aa ");break;
	  case 0x87:	outstr("\\c{c}");break;

	  case 0x88:	outstr("\\^{e}");break;
	  case 0x89:	outstr("\\\"{e}");break;
	  case 0x8a:	outstr("\\`{e}");break;
	  case 0x8b:	outstr("\\\"{\\i}");break;
	  case 0x8c:	outstr("\\^{\\i}");break;
	  case 0x8d:	outstr("\\`{\\i}");break;
	  case 0x8e:	outstr("\\\"{A}");break;
	  case 0x8f:	outstr("\\AA ");break;

	  case 0x90:	outstr("\\'{E}");break;
	  case 0x91:	outstr("\\ae ");break;
	  case 0x92:	outstr("\\AE ");break;
	  case 0x93:	outstr("\\^{o}");break;
	  case 0x94:	outstr("\\\"{o}");break;
	  case 0x95:	outstr("\\`{o}");break;
	  case 0x96:	outstr("\\^{u}");break;
	  case 0x97:	outstr("\\`{u}");break;

	  case 0x98:	outstr("\\\"{y}");break;
	  case 0x99:	outstr("\\\"{O}");break;
	  case 0x9a:	outstr("\\\"{U}");break;
/*	no cent sign */
	  case 0x9c:	outstr("\\pounds ");break;
/*	no yen */
	  case 0x9e:	mbold();outstr("$\\beta$");embold();break;
	  case 0x9f:	outstr("$f$");break;

	  case 0xa0:	outstr("\\'{a}");break;
	  case 0xa1:	outstr("\\'{\\i}");break;
	  case 0xa2:	outstr("\\'{o}");break;
	  case 0xa3:	outstr("\\'{u}");break;
	  case 0xa4:	outstr("\\~{n}");break;
	  case 0xa5:	outstr("\\~{N}");break;
	  case 0xa6:	outstr("\\b{a}");break;
	  case 0xa7:	outstr("\\b{o}");break;

	  case 0xa8:	outstr("?' ");break;
	  case 0xab:	outstr("$1/2$");break;
	  case 0xac:	outstr("$1/4$");break;
	  case 0xad:	outstr("!' ");break;
	  case 0xae:	outstr("$\\ll$");break;
	  case 0xaf:	outstr("$\\gg$");break;

	  case 0xb0:	outstr("\\~{a}");break;
	  case 0xb1:	outstr("\\~{o}");break;
	  case 0xb2:	outstr("\\O ");break;
	  case 0xb3:	outstr("\\o ");break;
	  case 0xb4:	outstr("\\oe ");break;
	  case 0xb5:	outstr("\\OE ");break;
	  case 0xb6:	outstr("\\`{A}");break;
	  case 0xb7:	outstr("\\~{A}");break;

	  case 0xb8:	outstr("\\~{O}");break;
	  case 0xb9:	outstr("\\\"{\\ }");break;
	  case 0xba:	outstr("\\'{\\ }");break;
	  case 0xbb:	outstr("\\dag ");break;
	  case 0xbc:	outstr("\\P ");break;
	  case 0xbd:	outstr("\\copyright ");break;
/*	no registered or trademark */

	  case 0xc2:	mbold();outstr("$\\aleph$");embold();break;
	  case 0xcb:	outstr("\\verb+^+");break;   /* my convention */
	  case 0xd2:	outstr("$\\partial$");break; /* my own convention */
	  case 0xd6:	mbold();outstr("$\\omega$");embold();break;  /* ditto */
	  case 0xd7:	outstr("$\\Lambda$");break;  /* yet again */
	  case 0xdd:	outstr("\\S ");break;
	  case 0xde:	outstr("$\\wedge$");break;
	  case 0xdf:	outstr("$\\infty$");break;

	  case 0xe0:	mbold();outstr("$\\alpha$");embold();break;
	  case 0xe1:	mbold();outstr("$\\beta$");embold();break;
	  case 0xe2:	outstr("$\\Gamma$");break;
	  case 0xe3:	mbold();outstr("$\\pi$");embold();break;
	  case 0xe4:	outstr("$\\Sigma$");break;
	  case 0xe5:	mbold();outstr("$\\sigma$");embold();break;
	  case 0xe6:	mbold();outstr("$\\mu$");embold();break;
	  case 0xe7:	mbold();outstr("$\\tau$");embold();break;

	  case 0xe8:	outstr("$\\Phi$");break;
	  case 0xe9:	outstr("$\\Theta$");break;
	  case 0xea:	outstr("$\\Omega$");break;
	  case 0xeb:	mbold();outstr("$\\delta$");embold();break;
	  case 0xec:	outstr("$\\oint$");break;
	  case 0xed:	mbold();outstr("$\\phi$");embold();break;
	  case 0xee:	outstr("$\\in$");break;
	  case 0xef:	outstr("$\\cap$");break;

	  case 0xf0:	outstr("$\\equiv$");break;
	  case 0xf1:	outstr("$\\pm$");break;
	  case 0xf2:	outstr("$\\geq$");break;
	  case 0xf3:	outstr("$\\leq$");break;
	  case 0xf4:	outstr("$\\int$");break;	/* top of int */
	  case 0xf5:	break;				/* bot of int */
	  case 0xf6:	outstr("$\\div$");break;
	  case 0xf7:	outstr("$\\approx$");break;

	  case 0xf8:	outstr("$\\circ$");break;
	  case 0xf9:	outstr("$\\bullet$");break;
	  case 0xfa:	outstr("$\\cdot$");break;
	  case 0xfb:	outstr("$\\surd$");break;

	  default:	Cconout(c);fatal("in quote - bad character");
	}
	return;
}


/* mbold()	ugly kludge to get bold lowercase greek -- YUK!
**		needs global flag "debold"	1986-04-28
*/
mbold()
{
	if(isbold()){
		outstr("\\mbox{\\boldmath ");
		debold = TRUE;
	}
}

/* embold()	close flag for mbold -- YUK
**		resets flag "debold"		1986-04-28
*/
embold()
{
	if(debold){
		output('}');
	}
	debold = FALSE;
}
