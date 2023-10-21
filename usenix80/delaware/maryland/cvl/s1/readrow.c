#
#define infile 0
/*
 * Read a complete row of picture
 * so that if a pipe returns a partial amount
 * the rest will be read before continuing.
 * Calls errprnt to die after read errors.
 * Returns count of bytes requested but not read.
 * Thus, a successful call returns 0.
 */
readrow(buff, len)
char *buff;
int len;
{
	register int cnt;
	register char *bp;

	bp = buff;
	while(cnt = read(infile, bp, len)) {
	flush(1);
		if(cnt == -1) errprnt("read error");
		if((len =- cnt) == 0) return(0); /* complete row read */
		bp = &bp[cnt];
	}
	return(len);      /* End of file before row completed */
}
