/*******************************************************/
/***   STRINP.C     Feb 18, 1982    S. KLYCE         ***/
/*******************************************************/

/* #include <stdio.h> */
#define MINASCII	'\037'   /**  These two lines           **/
#define MAXASCII	'\176'   /** terminal dependent.        **/

strinp(s,svar,lim)
char *s, *svar;
int lim;
{
	int c,i;
	int error = 1;
	while (error) {
		printf("%s",s);
		error=i=0;
		while (--lim > 0 && (c=getchar()) != EOF && c != '\n')
		{
			if ((c < MINASCII || c > MAXASCII) && c != '\n') error=1;
			svar[i++] = c;
		}
		if (c == '\n')
			svar[i++] = c;
		svar[i] = '\0';
	}
}
