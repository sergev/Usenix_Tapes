/* -- Minimal file path
 *
 * $File: mpath.c  1.3  1985-03-29  14:12:00 $
 *
 *  Copyright (c) 1983  Kim Walden
 *	ENEA DATA, Sweden and 
 *	SYSLAB, Department of Computer Science,
 *	University of Stockholm, Sweden.
 *
 *	This software may be used and copied freely,
 *	provided that proper credit is given to the originator
 *	by including the above Copyright lines in each copy.
 *
 *	Usage:	mpath(filename, currdir)
 *		
 *		A pointer to the minmal path of 'filename'
 *		relative to directory 'currdir' is returned.
 *
 *		If 'filename' is not given in absolute form,
 *		it is taken to be relative to 'currdir'.
 *
 *		'currdir'  must be given in absolute form.
 *		
 *		E.g.:
 *
 *		1. mpath("/usr/src/doc", "/usr/src/cmd")  -->  "../doc"
 *
 *		2. mpath("../../src/doc/csh/..", "/usr/src/cmd")  -->  "../doc"
 *
 * $Log:	mpath.c,v $
 * 
 * Revision 1.3  1985-03-29  14:12:00  kim
 * New source format
 * 
 * Revision 1.2  1985-02-18  14:24:24  kim
 * New error handling
 */

# define LINSIZ 256
# define MAXLEVEL 100
# define CNULL '\0'

# include <stdio.h>

char	*	mpath();
int		split(),
		reduce();

char *
mpath(filename, currdir)
	char	*	filename,
		*	currdir;
{
	static	char	buf[LINSIZ];
	char	*	nm[MAXLEVEL],
		*	cd[MAXLEVEL];
	int		nmlen,
			cdlen,
			i,
			k;

	if (strlen(filename)+strlen(currdir) > LINSIZ-2)
	{
		error("too long path: %s/%s", currdir, filename);
		return NULL;
	}

	if (currdir[0] != '/')
	{
		error("directory not absolute: %s", currdir);
		return NULL;
	}

    /* Split the path strings of current directory and filename
    ** into arrays of characters pointers
    */
	cdlen = split(currdir, cd, '/');

	if (filename[0] != '/')
		sprintf(buf, "%s/%s", currdir, filename);
	else
		strcpy(buf, filename);

	nmlen = split(buf, nm, '/');

    /* Remove "//", "/.", and "x/.." entries */

	cdlen = reduce(cd, cdlen);
	nmlen = reduce(nm, nmlen);

    /* Both strings are now absolute and normalized,
    ** find out how far they match from root down
    */
	for (i = 0; i < cdlen && i < nmlen; i++)
		if (strcmp(cd[i], nm[i]) != 0)
			break;

    /* Concatenate the required number of ".." references
    ** needed to reach the point of divergence.
    */
	buf[0] = CNULL;

	for (k = 0; k < cdlen-i; k++)
	{
		if (buf[0] != CNULL)
			strcat(buf, "/");
		strcat(buf, "..");
	}

    /* Append the rest of the filename path */

	for (k = 0; k < nmlen-i; k++)
	{
		if (buf[0] != CNULL)
			strcat(buf, "/");
		strcat(buf, nm[i+k]);
	}

    /* Empty means "." reference */

	if (strlen(buf) == 0)
		strcpy(buf, ".");

	return buf;
}

static int
split(s, array, sep)
	char	*	s,
		*	array[];
{
	char	*	index(),
		*	p;
	int		n = 0;

    /* Split string on given separator into array.
    ** Works much like the split function of awk.
    */
	if ((p = (char *) calloc(1, strlen(s)+1)) == NULL)
	{
		error("can't allocate: %s", s);
		return NULL;
	}

	strcpy(p, s);

	array[n++] = p;
	while ((p = index(p, sep)) != NULL)
	{
		*p++ = CNULL;
		array[n++] = p;
	}

	return n;
}


static int
reduce(array, len)
	char	*	array[];
{
	int		af,
			bf;

    /* Remove ".", "..", and empty references from array,
    ** by shifting the cells.
    */
	for (af = bf = 0; bf < len; bf++)
	{
		if (strlen(array[bf]) == 0 ||
		    strcmp(array[bf], ".") == 0)
			continue;

		if (strcmp(array[bf], "..") == 0)
		{
			if (af == 0)
				continue;

			af -= 1;
		}
		else
			array[af++] = array[bf];
	}

	return af;
}
