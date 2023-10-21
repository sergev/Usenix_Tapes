/* direct.c - Search a directory for files matching the description, and
 *      return a sorted list of the matching files.  Supports drive name,
 *      path, and wildcards.  Expand your argv and amaze your friends.
 *
 * For use with Lattice C; last compiled with version 2.15.
 * Requires STDIO.H and DOS.H .
 *
 * Allocates space for the filenames with getmem, so be careful with
 * your rlsmem's.
 *
 * (c) 1984 James J. Gillogly.  All rights reserved.
 * Copyright abandoned 28 Sep 1985.
 *
 * Usage: n = directory(filename, names);
 *   integer n;      /* Returns number of files found. */
 *   char *filename; /* May include drive name, path, *, and ? */
 *   char ***names;  /* Address of the char ** you want pointing to the list. */
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
 */

#define NEWSTUFF        /* Make use of extended filenames */

#include <stdio.h>
#include <dos.h>

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


directory(filename, names)      /* look up filename in the directory */
char *filename, ***names;
{       char buffer[80];
	struct EFCB dta;
	register struct dirstruct *p,*q;/* trundle down the directory list */
	struct dirstruct *dir_root;     /* root of directory list */
	register int n;                 /* number of entries */
	register int i, j;

	if (!first_file(filename, buffer, &dta))
		return 0;       /* no files found */
	n = 1;                  /* one entry found  so far */
	p->next = 0;

	dir_root = (struct dirstruct *) getmem(sizeof (struct dirstruct));
	p = dir_root;           /* point at top of directory list */

	p->fname = (char *) getmem(strlen(buffer) + 1);
	strcpy(p->fname, buffer); /* save the first filename */

	while (next_file(buffer, &dta))
	{   n++;        /* looks like another winner */
	    p->next = (struct dirstruct *) getmem(sizeof (struct dirstruct));
	    p = p->next;    /* point at the next entry */
	    p->fname = (char *) getmem(strlen(buffer) + 1);
	    strcpy(p->fname, buffer);       /* save it */
	}
	p->next = 0;    /* end of the line */
	*names = (char **) getmem(n * sizeof (char *));  /* n names */
	for (i = 0, p = dir_root; i < n; i++)   /* insertion sort on pointers */
	{       for (j = i; j > 0; j--)    /* where does this one go? */
		{       if (strcmp((*names)[j - 1], p->fname) < 0) break;
			(*names)[j] = (*names)[j - 1];
		}
		(*names)[j] = (char *) getmem(strlen(p->fname) + 1);
		strcpy((*names)[j], p->fname);
#ifdef VERBOSE
		printf("%s goes into slot %d.\n", p->fname, j);
#endif VERBOSE
		q = p;          /* save it so we can delete it */
		p = p->next;    /* after pressing on to the next entry */
		rlsmem(q, sizeof (struct dirstruct));
	}
#ifdef VERBOSE
	printf("Here they are:\n");
	for (i = 0; i < n; i++)
		printf("\t%s\n", (*names)[i]);
#endif VERBOSE
	return n;
}

first_file(filename, buffer, dta) /* set up FCB and get first file */
char *filename, *buffer;
struct EFCB *dta;
{       register char *s, *t;
	register int i, ret;

	for (s = filename; *s; s++) if (islower(*s)) *s = toupper(*s);
	ret = bdos(SET_DTA, dta, 0);   /* set up data transfer area */
	ret = bdosx(FIND_FRST, filename, 0);
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
