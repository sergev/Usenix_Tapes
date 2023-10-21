#include <stdio.h>

/* sindex looks for an occurance  of the string s2 in the string
** s1. If s2 does not ocurr, sindex returns NULL. Otherwise, sindex
** returns pointer in s1 to where s2 ocurrs.
*/
char *sindex(big,small)
register  char *big;
register  char *small;
{ int biglen;
  int smlen;
  register char *lastchar;
  register char *bigptr, *smptr, *curptr;
  
  biglen = strlen(big);
  smlen = strlen(small);
  if (biglen < smlen || smlen == 0) return(NULL);
  lastchar = big + biglen - smlen;

  for (bigptr=big ; bigptr <= lastchar ; bigptr++) {
	curptr = bigptr;
	smptr = small;
	while ((*smptr) && (*smptr == *curptr++)) smptr++;
	if (*smptr == '\0') return(bigptr);
  }
  return(NULL);
}/*sindex*/

