#include <ctype.h>

#ifdef _tolower
#define tolower(c) _tolower(c)
#endif


lcase(s)
register char *s;
{
	register char *ptr;

	for (ptr = s; *ptr; ptr++)
		if (isupper(*ptr))
			*ptr = tolower(*ptr);
}
