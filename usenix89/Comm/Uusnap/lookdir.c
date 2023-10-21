/*
 * lookdir.c
 * author - dennis bednar 5 3 84
 * reads directory entries one at a time for you
 * By recompiling with proper ifdef flag, it works with either fixed length
 * (S3) or variable length names up to 255 chars as in Berkeley.
 *
 *
 * to use:
 *
 * setdir (char *name);		// to open directory "name" for reading
 *				// returns -1 iff error
 * char *getdir ();		// gets next directory name from this directory
 *				// returns (char *) 0 if done
 *
 * you do NOT need to call enddir () to close the directory, this
 * is automatically done when the eof on the directory is encountered.
 *
 * TODO
 * handle multiple open directories, reading from any.
 * allow user to call enddir() when he feels like it.
 */


/* pass DDIRFIXLEN ==1 or DDIRVARLEN == 1 from config.h which is
 * included by machine.h
 */

#include <stdio.h>
#include "machine.h"
#ifdef DIRFIXLEN
#include <sys/types.h>	/* added for osg5 only */
#include <sys/dir.h>	/* old directory format */
#else
#include <sys/types.h>	/* define u_long */
#include <sys/dir.h>	/* new directory format */
#endif


#ifdef DIRFIXLEN
#ifdef DIRVARLEN
	both defined error
#endif
#endif

#ifndef DIRFIXLEN
#ifndef DIRVARLEN
	neither defined error
#endif
#endif


#ifdef DIRVARLEN
DIR *opendir ();	/* open a directory, Berkley variable len format */
struct direct *readdir ();	/* read a filename from a directory */
extern void closedir ();	/* finally, close the directory opened by opendir() */
#endif

#define	MAXPATH		512

/* globals */
static	char	errbuf [MAXPATH];	/* build error message here */
#ifdef DIRVARLEN
	static DIR *headptr = (DIR *)0;	/* pointer to directory structure returned by opendir */
	static struct direct *dirptr;	/* pointer to directory entry structure returned by readdir */
#else
	static int fd = -1;	/* file descriptor associated with directory */
	static struct direct dirbuf;	/* holds directory entry read in from directory */
#endif

/* global routines */
char *getdir ();


/*
 * open the directory "name"
 * returns
 *	0 = good
 *	-1 = can't open directory
 */

setdir (name)
char *name;

{
	/* open the full path name of this dir */

#ifdef DIRFIXLEN
	/* if already open, close it first */
	if (fd != -1)
		enddir ();
	if ((fd = open (name, 0)) == -1)

#else
	/* if already open, close it first */
	if (headptr != (DIR *) 0)
		enddir ();
	if ((headptr = opendir (name)) == (DIR *) NULL)
#endif

		{
		return (-1);
		}
	else
		return 0;

	/* headptr is a pointer to a memory block containing info about this dir */

}


/*
 * return the name of the next directory entry
 * (char *) 0 means end of names
 */

char *getdir ()

{
	static char filename [MAXPATH];	/* read in file name from directory here */


#ifdef DIRFIXLEN

	/* check that directory is indeed open */
	if (fd == -1)
		{
		fprintf (stderr, "dir: getdir: forget to call setdir () first\n");
		exit (1);
		}

	while (read(fd, (char *)&dirbuf, sizeof(dirbuf))>0) {	/* not err or eof */
		if (dirbuf.d_ino == 0)	/* i-node is zero in this slot */
			continue;	/* skip this slot */
		if (strcmp (dirbuf.d_name, ".") == 0	/* then myself */
		||  strcmp (dirbuf.d_name, "..") == 0)	/* or my parent */
			continue;	/* skip this dir entry */
		strncpy (filename, dirbuf.d_name, 14);	/* tail end of name */
#ifdef DEBUG
		printf ("debug: %s\n", filename);
		fflush (stdout);
#endif
		return (filename);	/* next call read next entry */
		}

#else
	/* check that directory is indeed open */
	if (headptr == (DIR *) 0)
		{
		fprintf (stderr, "dir: getdir: forget to call setdir () first\n");
		exit (1);
		}


	while ((dirptr = readdir (headptr)) != (struct direct *)NULL) {	/* not err or eof */
		/* readdir will automatically skip over inodes which are zero */
		if (strcmp (dirptr->d_name, ".") == 0	/* then myself */
		||  strcmp (dirptr->d_name, "..") == 0)	/* or my parent */
			continue;	/* skip this dir entry */
		strcpy (filename, dirptr->d_name);	/* tail end of name */
		return (filename);	/* next call read next entry */
		}
#endif




enddir ();		/* have come to eof */
return (char *) 0;


}


/*
 * called internally when eof is detected
 * sets file descriptor to indicate that file is closed
 */

static enddir ()

{
#ifdef DIRFIXLEN
	close (fd);
	fd = -1;
#else
	closedir (headptr);
	headptr = (DIR *) 0;
#endif
}


#ifdef STAND
/*
 * standalone test code
 *	calling format:

	readdir		// lists all files in the current directory
	readdir dir	// lists files in "dir"
 */

main (argc, argv)
int argc;
char **argv;

{
	int i;		/* loop index into nargv [] */
	int nargc;	/* new arg count returned by getfnames */
	char **nargv;	/* new argv [] array returned by getfnames */


	if (argc == 1)
		getfnames (".", &nargc, &nargv);
	else if (argc == 2)
		getfnames (argv[1], &nargc, &nargv);
	else
		{
		fprintf (stderr, "Wrong number of arguments: usage: %s [directory]\n", argv [0]);
		exit (1);
		}

	if (nargc == -1)
		{
		fprintf (stderr, "%s: getfnames returned error\n", argv[0]);
		exit (1);
		}


	/* sort file names */
	sortargs (nargc-1, &nargv [1]);

	/* pretty print in columns */
	pretty (nargc, nargv, 80);

}
#endif


/*
 * print working directory
 * returns the name in curdir
 */
#if 0
pwd (curdir, dirlen)
char *curdir;
int dirlen;

{
	FILE *Fd;		/* file descriptor for popen */
	int fd;			/* file descriptor for open */
	char *p;		/* index within the dir */
	static char tempfile [] = "/tmp/pwdXXXXXX";	/* temporary file */
	static char pwdcmd [80] = "/bin/pwd >";		/* indirect to a file */
	static char rmcmd [80] = "rm ";		/* indirect to a file */



	/* new way is 10 times cleaner than before !! */
	Fd = popen ("/bin/pwd", "r");	/* open a pipe to pwd cmd */
	fgets (curdir, dirlen, Fd);	/* read in the answer */
	pclose (Fd);

	curdir[strlen(curdir) - 1] = '\0';	/* change the newline to null */

}

#endif


/*
 * returns the file names of all files in "dir" in an argv array
 * just like the shell does, with count in argc.
 *	*pargc = argc returned
 *		(-1 = error and *pargv is garbage,
 *		(1 = no files, and argv[0] = ""),
 *		(2 = 1 file, pointed to by argv[1])
 *	*pargv = pointer to array of pointers to file names
 * ie.
 * argv is returned in *pargv.
 * argv[0] contains the "" string
 * argv[1], ..., argv [argc-1] are pointers to the file names returned
 */

getfnames (dirname, pargc, pargv)
char *dirname;
int *pargc;
char ***pargv;

{
	char *rp;	/* return pointer to file name returned from getdir */
	char **argv;	/* argv [] pointer array dynamically growing */
	char *fp;	/* filename pointer malloc'ed, stuffed into argv[i] */
	int argc;	/* number of elements in the array so far */
	int increase;	/* number of bytes to expand argv[] array by */
	int numentries;	/* number of slots allocated in argv [] array */
	int cursize;	/* total number of bytes in argv[] currently */
	char *malloc (), *realloc ();	/* malloc (3)		*/


	if (setdir (dirname) == -1)
		{
		sprintf (errbuf, "setdir: can't open %s", dirname);
		perror (errbuf);
		exit (1);
		}

/* number of slots in argv array each time expanded by */
#define NUMWANTED 25

	/* compute size in bytes for expanding argv[] by NUMWANTED slots */
	increase = cursize = NUMWANTED * sizeof (*argv);

	/* allocate pointer array the first time */
	if ((argv = (char **) malloc (cursize)) == (char **) 0)
		{
		*pargc = -1;
		return;
		}
	else
		{
		numentries = NUMWANTED;		/* number of slots allocated */
		argv [0] = "";
		argc = 1;	/* number of slots filled so far */
		}


	while ((rp = getdir ()) != (char *) 0)
		{

		/* first expand the array if we need to */

		if (++argc > numentries)
			{
			/* have to reallocate a bigger array */
			argv = (char **) realloc ((char *)argv, cursize = cursize + increase);
			if (argv == (char **) 0)
				{
				*pargc = -1;	/* lost the old array!!! */
				return;
				}
			numentries += NUMWANTED;
			}


		/* get room to copy file name from static area returned
		 * from getdir (), if can't, loose filename.
		 */
		fp = malloc ((strlen (rp) + 1));	/* NULL at end */
		if (fp == (char *) 0)
			{
			--argc;
			break;
			}

		/* now this name will fit in the array, so put it there */
		strcpy (fp, rp);
		argv [argc-1] = fp;

		}

/* return with answers */
*pargc = argc;
*pargv = argv;


}
