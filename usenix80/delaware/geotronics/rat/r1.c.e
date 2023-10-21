202,206c
	yyval = genlab(); genlab();
	brkstk[++brkptr] = yyval;
	outnum(yyval);
.
197c
docode(p1) char *p1; {
.
124,126c
		if (lnb == '\n'  ||  lnb == ',')	/* continue */
.
29a
asscode(p1) int p1; {
	int c;

	outcode ("\tif(.not.");
	balpar (scrat);
	outcode (scrat);
	outcode (") call Bug (");
	while ((c = getc ())==' ' || c == '\t')
		;		/* skip whitespace */
	if (c=='-' || (c>='0' && c<='9'))  {
		ptc (c);
		while ((c = getc ())>='0' && c<='9')
			ptc (c);
	} else
		ptc ('0');	/* default */
	peek = c;
	ptc (')');
	outcode (0);		/* end line */
}

.
21,24c
	if (unt)  {
		outcode("\tif(.not.");
		balpar(scrat);
		outcode(scrat);
		outcode(")");
	}
.
19c
untils(unt,p1) int unt, p1; {
.
8c
int	forstk[20];
.
6c
int	brkstk[20];
.
0a
#
/*
 * r1.c - Ratfor preprocessor part 1
 *
 *	modified 28-Apr-1980 by D A Gwyn:
 *	1) until not required after repeat;
 *	2) do without label removed;
 *	3) stacks extended;
 *	4) continuation after + - * ( / & | = removed;
 *	5) assert statement implemented.
 */
.
w
q
