/*	Scdosfil will scan a UNIX filename and convert it to a
	DEC standard filename (e.g. 6 char file, 3 char ext packed 
	into rad50).
*/

scdosfil(ufil, dfil)
char *ufil;
int dfil[3];
{
	char register cc, cclass;
	char tnam[9];
	int i, nxtchr, ext;
	
	for (i=0; i<9; i++)
		tnam[i] = ' ';
	nxtchr = 0;
	ext = 0;
	
	while (cc = *ufil++) {
		if (cc >= 'a' && cc <= 'z')
			cc =- 040;
		if ((cc>='A' && cc<='Z') || (cc>='0' && cc<='9'))
			cclass = 'a';
		else
			cclass = cc;
		switch (cclass) {
			case '/':
				for (i=0; i<9; i++)
					tnam[i]=' ';
				nxtchr = 0;
				ext = 0;
				break;
			case 'a':
				if ((nxtchr != 6 || ext) && nxtchr != 9)
					tnam[nxtchr++] = cc;
				break;
			case '.':
				nxtchr = 6;
				ext = 1;
			}
		}
	dfil[0] = rad50(tnam);
	dfil[1] = rad50(&tnam[3]);
	dfil[2] = rad50(&tnam[6]);
}

rad50(str)
char *str;
{
	register int a,b,c;

	a = radmap(*str++);
	b = radmap(*str++);
	c = radmap(*str);
	return((((a * 050) + b) * 050) +c);
}

radmap(letter)
char letter;
{
	register int ii;

	ii = letter & 0177;
	if (( 'A' <= ii) && (ii <= 'Z')) return(ii-0100);
	if (ii == ' ') return(0);
	if (ii == '$') return(033);
	if (ii == '.') return(034);
	if ((ii >= '0') && (ii <= '9')) return(ii-022);
	return(035);
}
