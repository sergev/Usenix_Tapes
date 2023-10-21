/* unod.c -- reverse an 'od -c' */
#include <stdio.h>
main()
{
	char mnbuf[75];
	char chbuf[2][65];
	char outbuf[512];
	register char *obpnd, *obp;
	int bytcnt[2];
	register int incbyt;
	register int idubl = 0;
	register int ndubld = 1;
	register int i, j;
	int k;
	unsigned int m;
	register char *p;
	char c;
	mnbuf[74] = '\0';	/* a stop just in case */
	chbuf[0][64] = '\0';
	chbuf[1][64] = '\0';
	bytcnt[0] = 0;
	bytcnt[1] = 0;
	obp = outbuf;
	obpnd = outbuf + 511;
#define BADINPUT(string)	(void)fprintf(stderr,"Bad input '%s'\n",string)
#define FLUSHOUT	{ \
			m = obp-outbuf; \
			if (write(1,outbuf,m) != m) { \
			    (void)write(2,"Error writing to stdout!\n",25); \
			    return 4; \
			} \
			obp=outbuf; \
			}
	while (1) {
		if (fgets(mnbuf,73,stdin) == NULL) {
			FLUSHOUT;
			return 0;
		}
		if (mnbuf[0] == '*') continue;
		if ((i =sscanf(mnbuf,"%o7",&bytcnt[idubl])) != 1) {
			BADINPUT(mnbuf);
			return 1;
		}
		(void)strncpy(chbuf[idubl],mnbuf+7,64);
		incbyt = bytcnt[idubl] - bytcnt[1 - idubl];
		idubl = 1 - idubl;
		if (ndubld) {
			ndubld = 0;
			continue;
		}
		/* Now we have two filled double buffers. */
		for (i = 0; i < incbyt; i++) {
			j = (i % 16) * 4;
			p = chbuf[idubl] + j;
			if (strncmp("   ",p,3) == 0) {
				/* the character itself */
				*obp++ = *(p+3);
				if (obp > obpnd) FLUSHOUT;
			} else if (strncmp("  \\",p,3) == 0) {
				switch (*(p+3)) {
				case '0':
					c = '\0';
					break;
				case 'b':
					c = '\b';
					break;
				case 't':
					c = '\t';
					break;
				case 'n':
					c = '\n';
					break;
				case 'f':
					c = '\f';
					break;
				case 'r':
					c = '\r';
					break;
				default:
					BADINPUT(chbuf[idubl]);
					return 2;
				}
				
				*obp++ = c;
				if (obp > obpnd) FLUSHOUT;
			} else {
				if (sscanf(p+1,"%o3",&k) != 1) {
					BADINPUT(chbuf[idubl]);
					return 3;
				}
				*obp++ = k;
				if (obp > obpnd) FLUSHOUT;
			}
		}
	}
}
