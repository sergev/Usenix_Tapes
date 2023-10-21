#include <stdio.h>
#include "bastrans.h"

/************************************************************************
 *									*
 *		REPLACE.C     03/11/82      S. Klyce			*
 *									*
 *	This is a function that will replace occurrences of an		*
 *  "old" phrase with a "new" phrase in a "s"tring.  Replacement	*
 *  continues until all "old" are substituted (n=0) or until		*
 *  n occurrences have been replaced.  NOTE that passing a HUGE 	*
 *  n will have the same effect as passing 0.				*
 *									*
 *	Passing a 1 to "quot" will cause replace to ignore expressions	*
 *  in quotations.  This is useful for parsing program command lines.	*
 *									*
 *  USE:  replace(string,old_phrase,new_phrase,how_many,quote_ignore);	*
 *									*
 ************************************************************************/

replace(s,old,new,n,quot)
char  *old, *new;
char s[CLEN];
int n,quot;
{
	register i,j,k;
	char t[CLEN];
	int ind,cnt=0;
	while ((ind=index(s,old,quot)) >= 0 && (!n || (cnt++ < n))) {

		for (i=j=k=0;i<ind;i++,j++) *(t+i) = *(s+j);
		for (j += strlen(old);k < strlen(new); i++,k++) *(t+i) = *(new+k);
		while ((*(t+i++) = *(s+j++)) != '\0');
		for (i=0;((*(s+i) = *(t+i)) != '\0');i++);
	}
}
