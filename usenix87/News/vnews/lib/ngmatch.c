#include "defs.h"

/*
 * News group matching.
 *
 * nglist is a list of newsgroups.
 * sublist is a list of subscriptions.
 * sublist may have "meta newsgroups" in it.
 * All fields are NGDELIM separated,
 *
 * Currently implemented glitches:
 * sublist uses 'all' like shell uses '*', and '.' like shell '/'.
 * If subscription X matches Y, it also matches Y.anything.
 */
ngmatch(nglist, sublist)
register char *nglist, *sublist;
{
	register char *n, *s;
	register int rc;

	rc = 0;
	for (n = nglist; *n != '\0' && rc == 0;) {
		for (s = sublist; *s != '\0';) {
			if (*s != NEGCHAR)
				rc |= ptrncmp(s, n);
			else
				rc &= ~ptrncmp(s+1, n);
			while (*s && *s++ != NGDELIM);
		}
		while (*n && *n++ != NGDELIM);
	}
	return(rc);
}

/*
 * Compare two newsgroups for equality.
 * The first one may be a "meta" newsgroup.
 */
ptrncmp(ng1, ng2)
register char *ng1, *ng2;
{
	while (*ng1 && *ng1 != NGDELIM) {
		if (ng1[0]=='a' && ng1[1]=='l' && ng1[2]=='l') {
			ng1 += 3;
			while (*ng2 && *ng2 != NGDELIM && *ng2 != '.')
				if (ptrncmp(ng1, ng2++))
					return(1);
			return (ptrncmp(ng1, ng2));
		} else if (*ng1++ != *ng2++)
			return(0);
	}
	return (*ng2 == '\0' || *ng2 == '.' || *ng2 == NGDELIM);
}
