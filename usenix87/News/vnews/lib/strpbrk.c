#include "config.h"
#if USGREL < 30
/*
 * strpbrk - find first occurrence of any char from s2 in s1
 *	     Written by Henry Spencer.
 */

#define	NULL	0

char *				/* found char, or NULL if none */
strpbrk(s1, s2)
char *s1;
char *s2;
{
	register char *scan1;
	register char *scan2;

	for (scan1 = s1; *scan1 != '\0'; scan1++) {
		for (scan2 = s2; *scan2 != '\0';)	/* ++ moved down. */
			if (*scan1 == *scan2++)
				return(scan1);
	}
	return(NULL);
}
#endif
