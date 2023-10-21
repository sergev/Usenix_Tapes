/* elbat:  a program to de-compile nroff drive tables so they can
**         be fiddled with.
**
** usage:  elbat tabXXX > tabXXX.c
**
** Matt Crawford, University of Chicago, 10 May 1984
** ihnp4!oddjob!matt	       crawford@anl-mcs.arpa
**
** Modified by: Bruce Townsend, Bell Northern-Research, March 6 1985
**       - Changed for USG UNIX systems.
**
*/

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include "term.h"

void	loadtab(), vprint();
char	*codelabel[] = {
	"space",	"!",	"\"",	"#",	"$",	"%",	"&",
	"' close",	"(",	")",	"*",	"+",	",",	"- hyphen",
	".",	"/",	"0",	"1",	"2",	"3",	"4",	"5",
	"6",	"7",	"8",	"9",	":",	";",	"<",	"=",
	">",	"?",	"@",	"A",	"B",	"C",	"D",	"E",
	"F",	"G",	"H",	"I",	"J",	"K",	"L",	"M",
	"N",	"O",	"P",	"Q",	"R",	"S",	"T",	"U",
	"V",	"W",	"X",	"Y",	"Z",	"[",	"\\",	"]",
	"^",	"_ dash",	"` open",	"a",	"b",	"c",
	"d",	"e",	"f",	"g",	"h",	"i",	"j",	"k",
	"l",	"m",	"n",	"o",	"p",	"q",	"r",	"s",
	"t",	"u",	"v",	"w",	"x",	"y",	"z",	"{",
	"|",	"}",	"~",	"narrow sp",	"hyphen",	"bullet",
	"square",	"3/4 em",	"rule",	"1/4",	"1/2",	"3/4",
	"minus",	"fi",	"fl",	"ff",	"ffi",	"ffl",	"degree",
	"dagger",	"section",	"foot mark",	"acute accent",
	"grave accent",	"underrule",	"slash (longer)",
	"half narrow space",	"unpaddable space",	"alpha",
	"beta",	"gamma",	"delta",	"epsilon",	"zeta",
	"eta",	"theta",	"iota",	"kappa",	"lambda",
	"mu",	"nu",	"xi",	"omicron",	"pi",	"rho",	"sigma",
	"tau",	"upsilon",	"phi",	"chi",	"psi",	"omega",
	"Gamma",	"Delta",	"Theta",	"Lambda",
	"Xi",	"Pi",	"Sigma",	"Tau",	"Upsilon",	"Phi",
	"Psi",	"Omega",	"square root",	"terminal sigma",
	"root en",	">=",	"<=",	"identically equal",
	"equation minus",	"approx =",	"approximates",
	"not equal",	"right arrow",	"left arrow",	"up arrow",
	"down arrow",	"eqn equals",	"multiply",	"divide",
	"plus-minus",	"cup (union)",	"cap (intersection)",	"subset of",
	"superset of",	"improper subset",	" improper superset",
	"infinity",	"pt deriv",	"gradient",	"not",	"integral",
	"proportional to",	"empty set",	"member of",
	"equation plus",	"registration mk",	"copyright mk",
	"box rule",	"cent sign",	"dbl dagger",	"right hand",
	"left hand",	"math * ",	"bell system sign",
	"or (was star)",	"circle",	"left top of big curly",
	"left bottom of big curly",	"right top of big curly",
	"right bottom of big curly",	"left center of big curly",
	"right center of big curly",	"bold vertical rule",
	"left bottom of big bracket",	"right bottom of big bracket",
	"left top of big bracket",	"right top of big bracket",
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	"???",	"???",	"???",	"???",	"???",	/* No idea what these are */
};

struct t t;
struct t_stor t_stor;

main(argc, argv)
char	**argv;
{
	register int	c;
	register char	**endptr = &t.zzz;
	register FILE	*twfp;
	char		labelbuf[64];

	if (argc != 2) {
		fprintf (stderr, "Usage: elbat tabXXX > tabXXX.c\n");
		exit(1);
		}

	loadtab(argv[1]);
	if ( (twfp = fopen("term.h", "r")) == NULL ) {
		perror("term.h");
		exit(1);
	}
	
	while ( (c = getc(twfp)) != EOF && c != /*{*/ '}')
		putc(c, stdout);
	fclose(twfp);
	printf(/*{*/ "} t = {\n" /*}*/);	/* Stupid emacs! */
	printf("/*bset    */\t\t0%o,\n", t.bset);
	printf("/*breset  */\t\t0%o,\n", t.breset);
#define intshow(memb) \
		printf("/*%-8s*/\t\t%d,\n", "memb", t.memb)
	intshow(Hor);
	intshow(Vert);
	intshow(Newline);
	intshow(Char);
	intshow(Em);
	intshow(Halfline);
	intshow(Adj);
#define show(memb) \
		printf("/*%-8s*/\t\t\"", "memb");\
		vprint(t.memb);\
		printf("\",\n")
	show(twinit);
	show(twrest);
	show(twnl);
	show(hlr);
	show(hlf);
	show(flr);
	show(bdon);
	show(bdoff);
	show(iton);
	show(itoff);
	show(ploton);
	show(plotoff);
	show(up);
	show(down);
	show(right);
	show(left);
	while ( **--endptr == '\0' )
		;
	for ( c = 0; c < 256-32		/* Not all 256-32 chars are in use */
		&& &t.codetab[c] <= endptr; c++ ) {
		sprintf(labelbuf, "/* %s */", codelabel[c]);
		printf("%-20s\t\"", labelbuf);
		if ( t.codetab[c][0] )
			vprint(t.codetab[c]);
		else if ( t.codetab[c][1] ) {
			printf("\\000");
			vprint(t.codetab[c]+1);
			}
		else
			printf("\\000\\0");
		printf("\",\n");
	}
	printf(/*{*/ "};\n");
	exit(0);
}

void
vprint(str)
register char	*str;
{
	while ( str && *str ) {
		char	c[5];

		if ( isascii(*str) && isprint(*str)
				&& *str != '\\' && *str != '"' ) {
			c[0] = *str;
			c[1] = '\0';
		} else
			switch ( *str ) {
			case '\\':
			case '"':
				c[0] = '\\';
				c[1] = *str;
				c[2] = '\0';
			        break;
			case '\b':
			        strcpy(c, "\\b");
			        break;
			case '\t':
			        strcpy(c, "\\t");
			        break;
			case '\n':
			        strcpy(c, "\\n");
			        break;
			case '\r':
			        strcpy(c, "\\r");
			        break;
			default:
				sprintf(c, "\\%03.3o", (int)*str & 0377);
			        break;
			}
		fputs(c, stdout);
		str++;
	}
}

void
loadtab( tname )
char	*tname;
{
	register int	tfd;
	int		c_size, *ip;
	register char	**pp, *mptr;
	extern char	*malloc();

	if( (tfd=open(tname, O_RDONLY)) < 0 ) {
		perror( tname );
		exit(1);
	}
	read(tfd, &c_size, sizeof(int));
	read(tfd, &t_stor, sizeof(t_stor));
	mptr = malloc (c_size);
	read(tfd, mptr, c_size);
	t.bset = t_stor.bset;
	t.breset = t_stor.breset;
	t.Hor = t_stor.Hor;
	t.Vert = t_stor.Vert;
	t.Newline = t_stor.Newline;
	t.Char = t_stor.Char;
	t.Em = t_stor.Em;
	t.Halfline = t_stor.Halfline;
	t.Adj = t_stor.Adj;
	ip = &t_stor.twinit;
	for ( pp = &t.twinit; pp < &t.zzz;) *pp++ = mptr + *ip++;
}
