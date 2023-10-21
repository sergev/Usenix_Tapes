/*
 This routine accepts a three word integer array and returns the DEC
   filename contained in it.
*/

char *r50toasc(fil)
int fil[3];
{
	static char fnbuf[11];
	register char *p1, c;
	int i,j;

	p1 = fnbuf;
	for (i=0; i<3; i++) {
		if (i==2 && fil[2] != 0)
			*p1++ = '.';
		for (j=1; j<=3; j++)
			if (c=rmap(fil[i], j))
				*p1++ = c;
	}
	if (p1 == fnbuf)
		return(0);
	*p1++ = '\0';
	return(fnbuf);
}

rmap(i, p)
int i, p;
{
	register int c, j, k;

	for (j=3; j>p; j--)
		i = ldiv(0, i, 050);
	c = lrem(0, i, 050);
	if (c == 0)
		return(0);
	if (c > 0 && c <= 032)
		return('a'+c-1);
	if (c > 035 && c <= 047)
		return('0'+c-036);
	if (c==033) return('$');
	if (c==034) return('.');
	if (c==035) return('_');
}
