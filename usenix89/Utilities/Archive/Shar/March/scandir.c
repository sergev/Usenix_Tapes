/*
 *	scandir - a SysV library routine that simulates the
 *		  BSD 4.3 version.
 *
 *	returns:
 *		-1 for failures
 *		# of files found if successful
 *
 *	Produces a list of files which are contained in the
 *	directory "dirname".
 *
 *	The list is returned through "namelist" which is an
 *	array of pointers to (struct direct).  This list of
 *	pointers are malloc'ed and should be freed when
 *	done with.
 *
 *	The passed function "select" is passed a (struct direct)
 *	pointer for each directory entry.  If "select" returns
 *	non-zero the entry is included in "namelist", otherwise
 *	it is not.  If "select" is NULL, all entries are returned.
 *
 *	The function "compar" is passed a pair of pointers to
 *	(struct direct) and is used to sort "namelist" via qsort.
 *	If "compar" is NULL, the names are unsorted.
 *
 *	This source is public domain and the author claims no
 *	rights to it.  It may be copied, spindled, or mutilated.
 *
 *	R.J. Esposito - Bell of Pennsylvania
 *
 */

#include <stdio.h>
#ifdef	SYSV
#include <sys/types.h>
#include <ndir.h>

int
scandir(dirname, namelist, select, compar)
char		*dirname;
struct direct	*(*namelist[]);
int		(*select)();
int		(*compar)();
{
	DIR		*dfp;
	struct direct	*dp;
	register int	ii, nf;
	char		*malloc();

	if ((dfp = opendir(dirname)) == NULL)	/* can't open directory */
		return(-1);

	nf = 0;
	while ((dp = readdir(dfp)) != NULL)	/* read thru direcetory */
		if (select == NULL || (*select)(dp))
			nf++;

	if (!nf)				/* nothing found */
		return(0);

	/* malloc memory for the namelist array */

	*namelist = (struct direct **)malloc((nf+1)*sizeof(struct direct *));
	if (*namelist == NULL) {
		fprintf(stderr, "scandir: out of memory\n");
		return(-1);
	}

	for (ii = 0; ii < nf; ii++) {
		(*namelist)[ii] = (struct direct *)malloc(sizeof(struct direct));
		if ((*namelist)[ii] == NULL) {
			fprintf(stderr, "scandir: out of memory\n");
			return(-1);
		}
	}

	/* now re-read the directory loading up the namelist array */

	(*namelist)[ii] = 0;
	seekdir(dfp, 0L);
	ii = 0;

	while ((dp = readdir(dfp)) != NULL) {
		if (select == NULL || (*select)(dp)) {
			(*namelist)[ii]->d_ino = dp->d_ino;
			(*namelist)[ii]->d_reclen = dp->d_reclen;
			(*namelist)[ii]->d_namlen = dp->d_namlen;
			strcpy((*namelist)[ii]->d_name, dp->d_name);
			ii++;
		}
	}

	closedir(dfp);
	if (compar != NULL)		/* sort the list if required */
		qsort((char **)*namelist, nf, sizeof(struct direct *), compar);

	return(nf);
}
#else
static dummy(){}
#endif	SYSV
