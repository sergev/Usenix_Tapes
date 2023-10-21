static char SCCSID[] = "@(#)strindex.c	Ver. 1.2, 85/03/04 15:54:30";
strindex(s,t) /* C equivalent of fortran INDEX
	index(3) does characters, not strings
	Kernighan & Pike p. 192 */
	char *s, *t;
{	
	int i,n;
	n=strlen(t);
	for(i=0;s[i] != '\0'; i++)
		if(strncmp(s+i,t,n) == 0)
			return i;
	return -1; /* INDEX returns 0, but C strings start at 0 */
}
