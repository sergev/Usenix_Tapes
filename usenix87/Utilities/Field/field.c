/* field utility, from Bourne pp.228-9 */
#include <stdio.h>
static char SCCSID[] = "<@(#)field.c 1.11, 85/07/23>";
#define MAXF	256
#define MAXL	4096
#define IFS	'\t'	/* Field separator is tab on input, */
#define OFS	'\t'	/* and on output. */

int fv[MAXF];
int nf;
int mf;
char *fp[MAXF];
char L[MAXL];

main(argc,argv)
	int argc;
	char *argv[];
{
	register char *cp;
	register char **ap;
	register int c;
	int f;
	char opt, fs, ifs=IFS, ofs=OFS;

	while (argc>1) {
		if(sscanf(argv[1], "-%c%c", &opt, &fs) == 2) {
			switch(opt) {
			case 't':	/* change both field separators */
				ofs = fs;
			case 'T':	/* change only ifs */
				ifs = fs;
				break;
			default:
				printf("usage: %s [-tc] [ n ] ...\n", argv[0]);
				return(2);
			}
		}
		else if(sscanf(argv[1], "%d", &fv[nf++]) != 1) {
			printf("usage: %s [-tc] [ n ] ...\n", argv[0]);
			return(2);
		}
		argc--; argv++;
	}

	/* read and copy input */
	nf--;
	cp = L;
	ap = fp;
	*ap++ = cp;
	while(1){
		c = getc(stdin);
		if(c=='\n' || c== EOF) {
			int fc;
			if(cp==L && c==EOF) break;
			*cp++ = 0;
			mf = ap-fp;

			/* print this line */
			for(fc = 0; fc <= nf; fc++){
				putf(fv[fc]-1);
				if(fc != nf) putchar(ofs);
			}
			if(c == EOF) break;
			putchar('\n');
			cp = L;
			ap = fp;
			*ap++ = cp;
		}
		else if(c == ifs) {
			*cp++ = 0;
			*ap++ = cp;
		}
		else *cp++ = c;
	}
	return(0);
}

/* output field n from current line */
putf(n)
{
	register char *cp = fp[n];
	register char c;

	if(n<0 || n>=mf) return;
	while (c = *cp++) putchar(c);
}
