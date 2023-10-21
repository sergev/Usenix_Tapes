/*
 * Open file, calling xerror on failure.
 */

#include <stdio.h>
#include "config.h"

FILE *
ckfopen(name, mode)
register char *name, *mode;
{
	register FILE *fp;
	extern int errno;

	if ((fp = fopen(name, mode)) == NULL) {
#ifdef IHCC
		char	*fname, *rindex();
		/*
		 * IHCC users only see the "filename" that was in trouble,
		 * not the whole path.  (for security!)
		 */
		if ((fname = rindex(name, '/') != NULL)
			fname++;
		else
			fname = name;
		xerror("Cannot open %s (%s), errno=%d", fname, mode, errno);
#else
		xerror("Cannot open %s (%s), errno=%d", name, mode, errno);
#endif
	}
#ifdef notdef /* this doesn't make sense for readnews */
	/* kludge for setuid not being honored for root */
	if ((uid == 0) && (duid != 0) && ((mode == "a") || (mode == "w")))
		chown(name, duid, dgid);
#endif
	return(fp);
}
