char *
getstri(prompt)
char	*prompt;
{
	static char	buf[512], nxtbuf = 0;

	nxtbuf = (nxtbuf + 1) & 03;
	if( prmptrd(prompt, &buf[nxtbuf<<7], 128) == 0 ) return((char *)0);
	return(&buf[nxtbuf<<7]);
}
