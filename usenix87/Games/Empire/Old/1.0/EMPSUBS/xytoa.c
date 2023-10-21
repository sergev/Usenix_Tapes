char *
xytoa(x, y, cno)
int x, y, cno;
{
	static char buf[64], nxtbuf = 0;
	extern int capxof[], capyof[];

	nxtbuf = (nxtbuf + 1) & 03;
	sprintf(&buf[nxtbuf << 4], "%d,%d", xwrap(x - capxof[cno]),
		ywrap(y - capyof[cno]));
	return (&buf[nxtbuf << 4]);
}
