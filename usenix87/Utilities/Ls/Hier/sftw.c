/*
 * Sorted file tree walk (library routine).
 *
 * Identical (in theory) to ftw(3), except:
 *
 * - Calls user's fn() with the files sorted alphabetically (per strcmp(3))
 *   in two groups:  All non-directories first, followed by directories (with
 *   the descendents of each directory after the directory).  Non-stat'able
 *   files and non-readable directories are included in the second group.
 *
 * - Doesn't keep one file open for each level of recursion, so it doesn't
 *   need a depth argument (which actually affects file opens/closes, NOT
 *   maximum search depth).
 *
 * - Uses a lot more malloc space.
 *
 * - Supports an additional argument which tells it to include all files
 *   and directories, including those whose names start with "." (except that
 *   the given filename is always included, regardless of the flag, like
 *   ls(1)).  The caller could implement this, but not very efficiently.
 *
 * Like ftw(), it ignores "." and ".." files, even with the all flag.
 *
 * For convenience, form of call is:
 *
 *	#include <ftw.h>
 *
 *	int sftw (path, fn, allfiles)
 *		char	*path;
 *		int	(*fn)();
 *		int	allfiles;
 *
 * Form of fn() is:
 *
 *	int fn (name, statp, type)
 *		char	*name;
 *		struct	stat *statp;
 *		int	type;
 *
 * See ftw(3) for more information.
 *
 * Compile with -DTEST to get a runnable test version that walks from "."
 * and tells types, permissions, and filenames passed to fn().
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <ftw.h>
#ifdef	BSD
#include <sys/dir.h>
#else
#include <ndir.h>
#endif	/* BSD */

static char *malloc(), *strcpy();
static void free();


/*********************************************************************
 * MISCELLANEOUS GLOBAL VALUES:
 */

#define	PROC				/* null; easy to find procs */
#define	FALSE	0
#define	TRUE	1
#define	CHNULL	('\0')
#define	CPNULL	((char *) NULL)
#define	REG	register

/* built-up filename for passing to the user program; hope it's big enough */
static	char filename [1000];

static	unsigned short euid, egid;	/* only get them once */



/************************************************************************
 * FILE DATA STRUCTURE:
 *
 * A contiguous array of pointers is used for sorting, built after knowing
 * how many directory entries there are to sort.  Each entry points to a
 * struct filedata which holds information for one directory entry.
 */

typedef	struct filedata *fdpt;
typedef	struct filedata **fdppt;

struct filedata {
	char	*name;		/* in malloc'd space	*/
	int	type;		/* see ftw.h		*/
	struct	stat statbuf;	/* from stat(2)		*/
};


/************************************************************************
 * FILE AND STRING DATA BLOCKS:
 *
 * Since a directory may grow arbitrarily as it's read, there's no way to
 * know in advance how big it is.  And it's necessary to return all malloc'd
 * memory.  To make this possible, and to save malloc space and time, directory
 * entry data and filenames are stored in buffers allocated a chunk a time.
 */

#define	DBENTRIES	20	/* file entries per datablock */
#define	STRINGENTRIES 1008	/* chars per string buffer    */

typedef	struct datablock *dbpt;
typedef	struct datablock **dbppt;

struct datablock {
	dbpt	next;				/* next block if any */
	struct	filedata fd [DBENTRIES];	/* the data itself   */
};

#define	DBNULL  ((dbpt) NULL)
#define	DBSIZE  (sizeof (struct datablock))


typedef struct stringblock *sbpt;
typedef struct stringblock **sbppt;

struct stringblock {
	sbpt	next;				/* next block if any */
	char	buf [STRINGENTRIES];		/* the data itself   */
};

#define	SBNULL  ((sbpt) NULL)
#define	SBSIZE  (sizeof (struct stringblock))


/************************************************************************
 * S F T W
 *
 * Handle the filename given by the user.  Since sftw() must stat() each
 * file before sorting, the first (top level) file must be handled specially,
 * not as part of re-entrant code.  (Think about it...)
 */

PROC int sftw (path, UserFunc, allfiles)
	char	*path;
	int	(*UserFunc)();
	int	allfiles;
{
	struct	stat statbuf;		/* from first file	*/
	int	type;			/* of first file	*/
	int	retval;			/* return by UserFunc()	*/
	unsigned short geteuid(), getegid();

	euid = geteuid();	/* initialize values */
	egid = getegid();

/*
 * HANDLE INITIAL FILE:
 */

	type = GetType (path, & statbuf);

	if (retval = UserFunc (path, & statbuf, type))	/* it's unhappy */
	    return (retval);

	if (type != FTW_D)			/* we're done */
	    return (0);

/*
 * WORK ON A READABLE DIRECTORY:
 */

	strcpy (filename, path);		/* now we can append to it */
	strcat (filename, "/");			/* prepare for additions   */

	return (DoDirectory (UserFunc, allfiles));

} /* sftw */


/************************************************************************
 * D O   D I R E C T O R Y
 *
 * Given UserFunc(), all files flag, and global filename (directory path) where
 * to start and on which to build complete pathnames, read the directory, sort
 * filenames, and call UserFunc() for each file in the directory.  This routine
 * calls itself to recurse, after each directory name is passed to UserFunc().
 * Because it reads and saves a directory's contents in order to sort them, it
 * does not keep any files open while recursing, just lots of memory.
 *
 * Free all memory from this level before returning, even in case of error.
 * Return -1 in case of error, or the value from UserFunc() if non-zero.
 */

PROC static int DoDirectory (UserFunc, allfiles)
	int	(*UserFunc)();
	int	allfiles;		/* include ".*" files?	  */
{
	dbpt	dbhead = DBNULL;	/* first datablock ptr	  */
	sbpt	sbhead = SBNULL;	/* first stringblock ptr  */

	fdppt	fdphead;		/* filedata list to sort  */
REG	fdppt	fdpcurr;		/* current list pointer	  */
REG	fdpt	fdcurr;			/* current entry pointer  */
REG	int	files;			/* number in directory	  */

	int	retval;			/* copy of return value	  */

/* pointer into filename where to append basenames */
REG	char	*basename = filename + strlen (filename);

	int	FDCmp();
	void	qsort();

#define	RETURN(value) { FreeBlocks (dbhead, sbhead); return (value); }

/*
 * READ DIRECTORY:
 */

	if ((files = ReadDirectory (& dbhead, & sbhead, allfiles)) < 0)
	    RETURN (-1);

/*
 * BUILD AND SORT POINTERS TO FILES:
 *
 * Get a big chunk of contiguous memory for the pointers, then set them up.
 * Afterwards, filedata entries will be accessed via the pointers.
 */

	if ((fdphead = (fdppt) malloc (files * sizeof (fdpt))) == (fdppt) NULL)
	    RETURN (-1);

#undef	RETURN
#define	RETURN(value) { FreeBlocks (dbhead, sbhead); \
			free ((char *) fdphead); return (value); }

	SetFDList (fdphead, fdphead + files, dbhead);
	qsort ((char *) fdphead, (unsigned) files, sizeof (fdpt), FDCmp);

/*
 * TRAVERSE FILES USING SORTED POINTERS:
 *
 * Append each file's basename to the current path in global filename,
 * overlaying whatever basename was there before, and pass it to UserFunc().
 */

	fdpcurr = fdphead;

	while (files-- > 0)
	{
	    strcpy (basename, (fdcurr = (*fdpcurr++)) -> name);

	    if (retval = UserFunc (filename, & (fdcurr -> statbuf),
				   fdcurr -> type))  /* it's unhappy */
	    {
		RETURN (retval);
	    }

/*
 * RECURSE FOR A DIRECTORY:
 */

	    if ((fdcurr -> type) == FTW_D)
	    {
		strcat (basename, "/");		/* for next level */

		if (retval = DoDirectory (UserFunc, allfiles))
		    RETURN (retval);
	    }
	}

	RETURN (0);

} /* DoDirectory */


/************************************************************************
 * R E A D   D I R E C T O R Y
 *
 * Given pointers to datablock and stringblock chain head pointers, all files
 * flag, and global filename (name of a readable directory) on which to build
 * complete pathnames, open, read, and close a directory, saving name, stat,
 * and type information on each file in a chain of datablocks and stringblocks,
 * and setting the chain head pointers.  Return the number of filenames read
 * and saved (>= 0), or -1 in case of error, but always close the directory.
 */

PROC static int ReadDirectory (dbheadp, sbheadp, allfiles)
	dbppt	dbheadp;		/* start of chain	*/
	sbppt	sbheadp;		/* start of chain	*/
	int	allfiles;		/* include ".*" files?	*/
{
REG	DIR	*dirp;			/* for reading directory  */
REG	struct	direct *entp;		/* directory entry	  */
REG	char	*name;			/* fast copy of filename  */

REG	dbpt	dbcurr;			/* current datablock ptr  */
	dbppt	dbnextp = dbheadp;	/* next datablock ptr ptr */
REG	int	dbentry = DBENTRIES;	/* current entry in block */

REG	fdpt	fdcurr;			/* current filedata entry */

REG	sbpt	sbcurr;			/* current stringblock ptr  */
	sbppt	sbnextp = sbheadp;	/* next stringblock ptr ptr */
REG	char	*end   = "";		/* last + 1 of block	    */
REG	char	*start = end;		/* next free place	    */

/* pointer into filename where to append basenames */
REG	char	*basename = filename + strlen (filename);
REG	int	files	  = 0;		/* files read and saved	  */

#undef	RETURN
#define	RETURN(value) { closedir (dirp); return (value); }

/*
 * OPEN AND READ DIRECTORY:
 */

	if ((dirp = opendir (filename)) == (DIR *) NULL)
	    return (-1);		/* hope errno is set */

	/* now be sure to use the RETURN() macro */

	while ((entp = readdir (dirp)) != (struct direct *) NULL)
	{

/*
 * OPTIONALLY SKIP ".*" FILES:
 *
 * Always skip "." and ".." files, like ftw().
 */

	    if ((* (name = entp -> d_name) == '.')	/* fast check */
	     && ((! allfiles)
	      ||  (name [1] == CHNULL)
	      || ((name [1] == '.') && (name [2] == CHNULL))))
	    {
		continue;
	    }

	    files++;

/*
 * GET A NEW DATABLOCK; APPEND TO CHAIN:
 */

	    if (dbentry >= DBENTRIES)		/* block is full */
	    {
		if ((dbcurr = *dbnextp = (dbpt) malloc (DBSIZE)) == DBNULL)
		    RETURN (-1);

		* (dbnextp = & (dbcurr -> next)) = DBNULL;
		dbentry = 0;
	    }

/*
 * GET A NEW STRINGBLOCK; APPEND TO CHAIN:
 *
 * Yes, we may abandon some unused space in the previous block...  Hope that
 * STRINGENTRIES is much larger than the average directory entry name size.
 */

	    if ((entp -> d_namlen) + 1 > (end - start))
	    {
		if ((sbcurr = *sbnextp = (sbpt) malloc (SBSIZE)) == SBNULL)
		    RETURN (-1);

		* (sbnextp = & (sbcurr -> next)) = SBNULL;
		end = (start = (sbcurr -> buf)) + STRINGENTRIES;
	    }

/*
 * SAVE INFORMATION ON ONE FILE:
 *
 * Append each file's basename to the current path in global filename,
 * overlaying whatever basename was there before, and pass it to GetType().
 */

	    fdcurr = (dbcurr -> fd) + (dbentry++);	/* quick pointer */

	    strcpy (((fdcurr -> name) = start), name);
	    start += (entp -> d_namlen) + 1;

	    strcpy (basename, name);
	    (fdcurr -> type) = GetType (filename, & (fdcurr -> statbuf));

	} /* while */

	RETURN (files);

} /* ReadDirectory */


/************************************************************************
 * G E T   T Y P E
 *
 * Given a filename and a pointer to a stat structure, stat() the file into the
 * structure and return an appropriate ftw() type.  Since directories are not
 * opened for reading until much later, after sorting, determine readability
 * now using euid, egid, and st_mode.  Can't use access(2) because it checks
 * real ids, not effective (sigh).
 */

PROC static int GetType (name, statp)
	char	*name;
	struct	stat *statp;
{
#define	UREAD (S_IREAD)	     /* read permission bits for user, group, other */
#define	GREAD (S_IREAD >> 3)
#define	OREAD (S_IREAD >> 6)

	if (stat (name, statp) < 0)
	    return (FTW_NS);			/* not stat'able */

	if (((statp -> st_mode) & S_IFMT) != S_IFDIR)
	    return (FTW_F);			/* not directory */

	/* pick appropriate permission bit, then see if it's set */

	return (
	    ( ((statp -> st_uid) == euid) ? ((statp -> st_mode) & UREAD) :
	      ((statp -> st_gid) == egid) ? ((statp -> st_mode) & GREAD) :
					    ((statp -> st_mode) & OREAD) ) ?
	    FTW_D : FTW_DNR);

} /* GetType */


/************************************************************************
 * S E T   F D   L I S T
 *
 * Given pointers to the current (head) and end + 1 of an array of
 * uninitialized struct filedata pointers, and a current (head) struct
 * datablock pointer, fill in the pointers in the array to point to the
 * filedata entries in the datablock chain.
 */

PROC static SetFDList (fdpcurr, fdpend, dbcurr)
REG	fdppt	fdpcurr;	/* start at head */
REG	fdppt	fdpend;		/* last + 1	 */
REG	dbpt	dbcurr;		/* start at head */
{
REG	int	dbentry;	/* current index */

	while (TRUE)				/* until return */
	{
	    for (dbentry = 0; dbentry < DBENTRIES; dbentry++)	/* one block */
	    {
		if (fdpcurr >= fdpend)		/* no more */
		    return;

		*fdpcurr++ = (dbcurr -> fd) + dbentry;
	    }

	    dbcurr = dbcurr -> next;
	}

	/* never get here */

} /* SetFDList */


/************************************************************************
 * F D   C M P
 *
 * Given two pointers to pointers to filedata entries, compare the entries
 * and return -1, 0, or 1 for how they relate.  "Normal" files (FTW_F)
 * are always lower than other types, then names are compared with strcmp().
 */

PROC static int FDCmp (fdpp1, fdpp2)
	fdppt	fdpp1, fdpp2;
{
REG	int	type1 = (*fdpp1) -> type;
REG	int	type2 = (*fdpp2) -> type;

	return (((type1 == FTW_F) && (type2 != FTW_F)) ? -1 :
		((type1 != FTW_F) && (type2 == FTW_F)) ?  1 :
		strcmp ((*fdpp1) -> name, (*fdpp2) -> name));

} /* FDCmp */


/************************************************************************
 * F R E E   B L O C K S
 *
 * Given pointers to heads of datablock and stringblock chains, free the
 * malloc'd memory in the chains.
 */

PROC static FreeBlocks (dbhead, sbhead)
REG	dbpt	dbhead;
REG	sbpt	sbhead;
{
REG	dbpt	dbtemp;
REG	sbpt	sbtemp;

	while (dbhead != DBNULL)
	{
	    dbtemp = dbhead -> next;
	    free ((char *) dbhead);
	    dbhead = dbtemp;
	}

	while (sbhead != SBNULL)
	{
	    sbtemp = sbhead -> next;
	    free ((char *) sbhead);
	    sbhead = sbtemp;
	}

} /* FreeBlocks */


#ifdef TEST

/************************************************************************
 * TEST HARNESS:
 */

PROC int fn (name, statp, type)
	char	*name;
	struct	stat *statp;
	int	type;
{
	printf ("%-3s %06o \"%s\"\n",
		(type == FTW_F)   ? "F"   : (type == FTW_D)  ? "D"  :
		(type == FTW_DNR) ? "DNR" : (type == FTW_NS) ? "NS" : "?",
		statp -> st_mode, name);

	return (0);
}

PROC main()
{
	printf ("sftw returns %d\n", sftw (".", fn));
}

#endif TEST
