/************************************************************************
 *									*
 *		INDEX.C    06/10/82     S. Klyce			*
 *									*
 *	Return the index of a string, t, in string, s.  Routine can be	*
 *  told to ignore quoted segments of a string (qt=1).			*
 *									*
 *	USE:  ivar=index(string,pattern,ignore_quotes);			*
 *									*
 ************************************************************************/
index(s,t,qt)
char *s, *t;
int qt;
{
	register i,j,k;
	int q;
	for (q=i=0;*(s+i) != '\0';i++) {
		if (*(s+i) == '\"') q++;
		if (q==2) q=0;
		if (!q || !qt) {
			for (j=i,k=0;*(t+k) != '\0' && *(s+j) == *(t+k);j++,k++);
			if (*(t+k) == '\0') return(i);
		}
	}
	if (q) return(-2); /* Use this in calling routine to check for mismatched quotation marks */
	return(-1); /* No match */
}
