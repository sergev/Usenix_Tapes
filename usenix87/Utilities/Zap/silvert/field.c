/* field utility, from Bourne pp.228-9 */
#include <stdio.h>
static char SCCSID[] = "@(#)field.c	Ver. 1.9, 85/04/23 18:58:12";
#define MAXF	256
#define MAXL	4096
#define IFS	':'	/* Field separator is colon on input, */
#define OFS	':'	/* and on output. */

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
	static char FIELD[] = "@(#)field.c	1.9 85/04/23: : in, \t out"; /* SCCS identifier */
	int f;

	while (argc>1) {
		if(sscanf(argv[1], "%d", &fv[nf++]) != 1) {
			printf("usage: field [ n ] ...\n");
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
				if(fc != nf) putchar(OFS);
			}
			if(c == EOF) break;
			putchar('\n');
			cp = L;
			ap = fp;
			*ap++ = cp;
		}
		else if(c == IFS) {
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
