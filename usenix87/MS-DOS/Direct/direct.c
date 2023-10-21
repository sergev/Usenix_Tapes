/* direct.c - Search a directory for files matching the description, and
 *      return a sorted list of the matching files.  Supports drive name,
 *      path, and wildcards.  Expand your argv and amaze your friends.
 *
 * For use with Lattice C; last compiled with version 2.15.
 * Requires STDIO.H and DOS.H .
 *
 * IF LATTICE:
 * Allocates space for the filenames with getmem, so be careful with
 * your rlsmem's.
 *
 * ELSE	uses malloc(),free()
 *
 * (c) 1984 James J. Gillogly.  All rights reserved.
 * Copyright abandoned 28 Sep 1985.
 *
 * Modified 10/6/85 by D. M. Ihnat
 *
 * Usage: n = directory(filename, names);
 *   integer n;       Returns number of files found. 
 *   char *filename;  May include drive name, path, *, and ? 
 *   char ***names;   Address of the char ** you want pointing to the list. 
 *
 * sel_disk(letter); char letter;
 *      Sets current drive to this one (A, B, etc); returns NO on error.
 * n = directory(filename, names); char *filename, **names;
 *      Search current directory for this filename (may include ?'s), and
 *      allocate enough space for all the resulting filenames.
 *      Returns number of files found.
 * first_file(filename, buffer)
 *      Looks for the first file set up in the global file control block;
 *      returns NO if none.
 * next_file(buffer)
 *      Looks for the next file; must be preceded by a first_file().
 * set_dta(adr); char *adr;
 *      Sets the disk transfer address used by the directory functions.
 *
 * 29 Aug 84: sel_disk(letter), directory(), first_file(), next_file(),
 *            set_dta(adr)
 *
 * 17 Sep 85: changed the calling sequence for directory:
 *     n = directory(filename, names); char *filename, ***names;
 *     It now takes the address of a char ** pointer pointer.
 *   Support drive letters and paths (requires DOS 2.0 or higher).
 *
 *  6 Oct 85:	Modified by Dave Ihnat
 *    - Supports both Lattice and non-Lattice compilers (notably Aztec)
 *    - Now returns a pointer to a NULL-terminated namelist , similar to **argv
 *    - Supports a call of the form directory((char *)NULL,names) to
 *        properly release used memory.  Returns 0 if release OK, else -1.
 *    - Added extern unsigned _dir_mode_ to select type of search.  Changes
 *        to this flag are only valid when starting a search; it is a
 *        logical OR of:
 *		0x0	:	Normal files (default)
 *		0x1	:	Read-only files
 *		0x2	:	Hidden files
 *		0x4	:	System files
 *		0x8	:	Volume ID (not recommended)
 *		0x10	:	Directories
 *
 *	24 Oct 85:	Modified by Dave Ihnat
 *    - Modified to check for existence of a drive designation or a path
 *        prefix; if one exists, remember it and prepend it to every filename
 *        returned.  This makes the returned file list directly usable by
 *        in calls such as open(), close().
 */

#include <stdio.h>
#include <ctype.h>

#ifdef	LATTICE
#include <dos.h>
#define	MEMALLOC	getmem
#define	MEMFREE(a,b)	rlsmem(a,b)
#else
#include <ctype.h>
#define	MEMALLOC	malloc
#define	MEMFREE(a,b)	free(a)
#endif

/*
 * Both Aztec 3.20d and Lattice 2.15 have strchr/strrchr; this is for any
 * others who may NOT.
 */
#define	RTCHAR	strrchr

#define SEL_DISK  0x0E  /* BDOS: select current disk drive */
#define SET_DTA   0x1A  /* BDOS: set disk transfer address */
#define FIND_FRST 0x4E  /* BDOS: find first file, long form */
#define FIND_NEXT 0x4F  /* BDOS: find next file, long form */

#define ERROR     0xFF  /* BDOS: error return */
#define NOTFOUND   2    /* Error return: file not found */
#define NOMORE    18    /* Error return: no more files */

#define YES 1
#define NO 0

#define MAXNAMES 100            /* temp - will get cleverer later */

struct dirstruct                /* temporary holding area for entries */
{       struct dirstruct *next; /* pointer to next entry */
	char *fname;            /* filename of this entry */
};

struct EFCB                     /* FCB for path-type calls */
{       char reserved[21];
	char attrib_found;
	char file_time[2];
	char file_date[2];
	char low_size[2];
	char high_size[2];
	char file_name[13];
};

unsigned _dir_mode_ = 0;	/* Initialize to use only normal files */

directory(filename, names)      /* look up filename in the directory */
char *filename, ***names;
{       char buffer[80];
	struct EFCB dta;
	register struct dirstruct *p,*q;/* trundle down the directory list */
	struct dirstruct *dir_root;     /* root of directory list */
	register int n;                 /* number of entries */
	register int i, j;
	char *ptr_prefix;		/* For storage of any prefix data */
	int len_prefix;

	char *MEMALLOC();
	char *RTCHAR();

	/* If filename is a null path, then user is trying to release memory */
	if (filename == (char *)NULL)
		return(free_direct(names));

	/*
	 * If there is a drive designator or a path in the filename,
	 * save it.  Check first for a path; if there is one, then
	 * the drive designator, if any, will be saved as a side effect.
	 * Otherwise, explicitly save the descriptor.  Notice that, as either
	 * the MS-DOS directory delimiter '\', or the Unix-style '/' may
	 * be given to MS-DOS from within a program, BOTH must be tested!
	 */
	len_prefix = 0;

	if(((ptr_prefix = RTCHAR(filename,'\\')) != (char *)NULL) ||
	   ((ptr_prefix = RTCHAR(filename,'/')) != (char *)NULL)   )
	{
		/* Path; get storage, and copy it */

		char *srcptr,*dstptr,*endptr; /* Local block storage */

		len_prefix = (ptr_prefix - filename) +2; /* Allow for null */

		endptr = ptr_prefix+1;	/* Point just after the delimiter */

		ptr_prefix = MEMALLOC(len_prefix);
		for(srcptr=filename,dstptr=ptr_prefix;srcptr < endptr;dstptr++,srcptr++)
		    	*dstptr = _toupper(*srcptr);

		*dstptr = '\0';	/* Null terminate */

	}else if(filename[1] == ':')
	{
		/* Only a drive designator; save it. */
		ptr_prefix = MEMALLOC(3);	/* 2 characters + null */

		ptr_prefix[0] = _toupper(filename[0]);
		ptr_prefix[1] = filename[1];
		ptr_prefix[2] = '\0';
		len_prefix = 2;
	}

	if (!first_file(filename, buffer, &dta))
		return 0;       /* no files found */
	n = 1;                  /* one entry found  so far */
	p->next = 0;

	dir_root = (struct dirstruct *) MEMALLOC(sizeof (struct dirstruct));
	p = dir_root;           /* point at top of directory list */

	p->fname = (char *) MEMALLOC(strlen(buffer) + 1);
	strcpy(p->fname, buffer); /* save the first filename */

	while (next_file(buffer, &dta))
	{   n++;        /* looks like another winner */
	    p->next = (struct dirstruct *) MEMALLOC(sizeof (struct dirstruct));
	    p = p->next;    /* point at the next entry */
	    p->fname = (char *) MEMALLOC(strlen(buffer) + 1);
	    strcpy(p->fname, buffer);       /* save it */
	}
	p->next = 0;    /* end of the line */

	/* (n names) + NULL ptr */
	*names = (char **) MEMALLOC((n+1) * sizeof (char *));

	for (i = 0, p = dir_root; i < n; i++)   /* insertion sort on pointers */
	{       for (j = i; j > 0; j--)    /* where does this one go? */
		{       if (strcmp((*names)[j - 1], p->fname) < 0) break;
			(*names)[j] = (*names)[j - 1];
		}
		(*names)[j] = (char *)MEMALLOC(strlen(p->fname)+len_prefix+1);
		if(len_prefix)
		{
			strcpy((*names)[j],ptr_prefix);
			strcat((*names)[j], p->fname);
		}else
			strcpy((*names)[j], p->fname);

#ifdef VERBOSE
		printf("%s goes into slot %d.\n", p->fname, j);
#endif VERBOSE
		q = p;          /* save it so we can delete it */
		p = p->next;    /* after pressing on to the next entry */
		MEMFREE(q, sizeof (struct dirstruct));
	}

	(*names)[n] = (char *)NULL;	/* Provide a null terminator */

	if(len_prefix)
		MEMFREE(ptr_prefix,strlen(ptr_prefix));	/* Return the path memory */

#ifdef VERBOSE
	printf("Here they are:\n");
	for (i = 0; i < n; i++)
		printf("\t%s\n", (*names)[i]);

	printf("Should be null terminator: 0x%x\n",(unsigned)names[n]);
#endif VERBOSE
	return n;
}

first_file(filename, buffer, dta) /* set up FCB and get first file */
char *filename, *buffer;
struct EFCB *dta;
{       register char *s, *t;
	register int i, ret;

	for (s = filename; *s; s++) if (islower(*s)) *s = toupper(*s);
#ifdef	LARGEDATA
	ret = bdosx(SET_DTA, dta, 0);   /* set up data transfer area */
	ret = bdosx(FIND_FRST, filename, _dir_mode_);
#else
	ret = bdos(SET_DTA, dta, 0);   /* set up data transfer area */
	ret = bdos(FIND_FRST, filename, _dir_mode_);
#endif
	if (ret == NOTFOUND || ret == NOMORE) return NO;
	getname(buffer, dta);   /* transfer the filename found to the caller */
	return YES;
}

next_file(buffer, dta) /* look up next file with the same attributes */
char *buffer;
struct EFCB *dta;
{       int ret;

	if (bdos(FIND_NEXT, 0, 0) == NOMORE) return NO;
	getname(buffer, dta);
	return YES;
}

getname(buffer, dta)    /* transfer filename from dta to the buffer area */
char *buffer;
struct EFCB *dta;
{       strcpy(buffer, dta->file_name);
}


sel_disk(letter)        /* make this disk the current one */
char letter;
{       int ret;

	ret = bdos(SEL_DISK, letter - 'A', 0);
	return ret;
}

free_direct(names)
char ***names;
{
	int index;

	/*
	 * Release each of the filename strings. There's room for
	 * some twiddling here to avoid unnecessary strlen calls,
	 * but this should be so rarely called that the overhead should
	 * be acceptable.  Feel free to change it, however, if you wish...
	 */
	for(index = 0;(*names)[index] != (char *)NULL;index++)
		if(MEMFREE((*names)[index],(strlen((*names)[index])+1)))
			return(-1);	/* Corrupted--don't trust anything */

	/* Now release the pointer structure */
	if(MEMFREE(*names,(index * sizeof(char *))))
		return(-1);

	/* Finally, remove any temptation to touch that released memory */
	*names = (char **)NULL;

	return(0);
}
