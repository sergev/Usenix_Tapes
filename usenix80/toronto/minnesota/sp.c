#define MAXPAGE 144

/* sp -- packs many short input lines into a few long ones
 *
 * Usage
 *	sp [ tdx ] [ cN ] [ [p] N ]
 * Useful for compressing output of programs such as ls,
 *	which produce many short lines of output.
 *
 * Filtering such output through sp will save paper.
 * Lines are printed left-to-right, not columnwise as with pr.
 *
 * Automatic column justification is attempted, based on the
 * line lengths encountered in the first 512 characters of data.
 * (This is overridden if a column width is specified).
 * Command letters are explained in the comments of the switch(*ap++).
 *
 */

char buf[MAXPAGE];
int tab, lntab, ccol, cwid, xflag;
char *lnbp buf;
char *bp buf;

main(argc,argv)
char **argv;
{
	register char *ap;
	register c;
	int pwid, nextab, lines;

	cwid = pwid = 0;
	if(ttyn(0) != 'x')  exit( write(2,
  "sp -- saves paper on output of ls, nm, &c. Use as a filter, e.g., ls | sp\n",74) );
	while (--argc) {
		ap = *++argv;
		if(*ap == '-') ap++;
	  lookc:
		switch(*ap++) {
		case 'c':	/* 'c': set column width */
			if((cwid=atoi(ap))>0) break;
			cwid = 14;
			goto lookc;
		case 'w':
		case 'p':	/* 'p' or 'w' : set page width to number */
			pwid = atoi(ap);
			break;
		case 'd':	/* decwriter page width */
			pwid = 132;
			goto lookc;
		case 't':	/* tty page width */
			pwid = 72;
			goto lookc;
		case 'x':	/* 'x': when a double newline found, break line */
			xflag++;
			goto lookc;
		default:
			pwid =+ atoi(ap-1);
		}
	}
	if(pwid <= 0) pwid = 80;
	else if (pwid>MAXPAGE) pwid = MAXPAGE;
	lines = 0;
	if(pwid > MAXPAGE) pwid = MAXPAGE;
	getchar(1);	/* initialize */
	while(c = *bp++ = getchar(0)) {
		switch(c) {

		case '\n':
		case '\r':
			if(xflag && bp == lnbp+1) {
				print(1);
				lines = 0;
				break;
			}
			if (bp[-2]==':' && bp >= buf+2) {
				print(0);
				if(lines && lnbp>buf) write(1,"\n",1);
				lines = 0;
			}
			if(cwid) {
				ccol =+ cwid;
				if(ccol > tab+1) nextab = ccol;
				else if(tab-ccol >= cwid-3)
					nextab = ((tab-ccol)/cwid+1)*cwid;
				else nextab = tab+1;
			} else nextab = tab + 8 - tab%8;
			if(nextab < pwid) {
				bp--;
				if(cwid)
					for(c=nextab-tab; --c >= 0;) *bp++ = ' ';
				else *bp++ = '\t';
				tab = lntab = nextab;
				lnbp = bp;
			} else {
				print(1);
				lines++;
			}
			break;
		case '\t':
			tab =+ 8 - tab%8;
			break;
		default:
			tab++;
		}
		if(tab >= pwid) {
			if(lnbp > buf) print(0);
			else {
				*bp++ = '\n';
				print(1);
			}
			lines++;
		}
	}
	*bp++ = '\n';
	if(tab) write(1, buf, bp-buf);
}

print(all)
{
	register c, k;

	if(all) {
		putln(bp-1);
		lntab = tab = ccol = 0;
		lnbp = bp = buf;
		return;
	}
	putln(lnbp-1);
	k = bp-lnbp;
	for(c=0; c <= k; c++) buf[c] = lnbp[c];
	tab =- lntab;
	lntab = ccol = 0;
	lnbp = buf;
	bp = buf+k;
}

getchar(first)
{
	static char *cp;
	static remain;
	static char cbuf[512];
	register k, len;
	int sum, cnt;

	if(remain == 0) {
		if((remain = read(0,cbuf,512)) <= 0) return(0);
		cp = cbuf;
	}
	if(first) {
		if(cwid > 0) {
			if(cwid == 1) cwid = 0;
		} else {
			  /* compute column width */
			len = 0;
			for(k = remain; --k >= 0;) switch(*cp++) {
				case '\n':
				case '\r':
					if(cwid<len)  cwid = len;
					if(len>4) {
						sum =+ len;
						cnt++;
					}
					len = 0;
					continue;
				case '\t':
					len =+ 8 - len % 8;
					continue;
				default:
					len++;  continue;
			}
			if(len < cwid)  len = cwid;
			if(len/2 > sum/cnt)	/* if max >> mean */
				len = (sum + sum/4) / cnt; /* fudge */
			cwid = len + 1;
			cp = cbuf;
		}
		return;
	}
	do
		remain--;
	while((k = *cp++) == 0);
	return(k);
}

putln(eobuf)
char *eobuf;	/* points at end of buf line to be printed */
{
	register char *p;
	register c;

	p = eobuf;
	while(*p <= ' ' && p > buf) p--;
	p++;
	c = *p;
	*p++ = '\n';
	write(1, buf, p-buf);
	*--p = c;
}
