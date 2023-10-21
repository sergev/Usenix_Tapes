/*
 *	DTREE - Print the tree structure of a directory
 *	4/7/83 name was changed from TREE to DTREE
 *	9/7/83 mods for 4.1c and 4.2 dirctory structure added
 *
 *	Dave Borman, Digital Unix Engineering Group
 *		decvax!borman
 *	Originally written at St. Olaf College, Northfield MN.
 *	Copyright (c) 1983 by Dave Borman
 *	All rights reserved
 *	This program may not be sold, but may be distributed
 *	provided this header is included.
 *
 *	Compile: PDP V7 w/split i&d    cc -O dtree.c -i -n -o dtree
 *		 VAX 4.1bsd	       cc -O dtree.c -n -o dtree
 *		 VAX 4.2bsd	       cc -O -DNEWDIR -DLINK dtree.c -n -o dtree
 *
 *	Usage:	dtree -[adfglnpsvx] [-b filenamesize] [-c linelength] [top]
 *	Flags:	-a) include non-directory entries in listing
 *		-d) sort tree with directories at the top
 *		-f) sort tree with files at the top
 *		-g) same as l, but use group name instead of user name
 *		-l) print stats with each listing
 *		    if both g & l flags are given, both owner and
 *		    group will be printed
 *		-n) do not sort the tree
 *		-p) include files starting with a '.' (except "." & "..")
 *		-s) use shorter stats. Implies -l if -g isn't given.
 *		-v) variable length columns off
 *		-x) do not cross mounted file systems.
 *              -b length) big directory names (define the max. name length)
 *		-c length) set max column length to "length" (if col.
 *                         length > file name length, this implies -b also)
 */

 /*     Modified by      Ed Arnold      CSU-CS   (csu-cs!arnold)     3-5-84
  *
  *     Allows symbolic links to both directories and individual files.
  *     With a '-l' or '-al' option, links are denoted with a 'l' in front of 
  *     file or directory permissions. In all other instances both links to 
  *     directories and files are represented just as files are. Contents of
  *     linked directories are not printed due to the possibility of 
  *     recursively linked directories.
  *
  *     Big directory name option added by:
  *                      Mike Vevea      CSU-CS (csu-cs!vevea)      3-22-84
  *	Toggle sense of -v (running 4.2), and eliminate some extraneous
  *		print info	Mike Meyer Energy Analysts (mwm@ea)	4/17/84
  *	Fix the exit status to correctly indicate what happened.
  *				Mike Meyer Energy Analysts (mwm@ea)	4/23/84
 */

#define STATS	/* comment out to remove stats, giving more core space	*/
		/* and thus the ability to tree larger tree structures	*/
		/* on PDP 11/70s */

/* #define	NEWDIR	/* directory structure ala Berkeley 4.1c or 4.2 */
 

/* #define LINK    /* allows links to be processed in Berkeley 4.2 version 
                      NEWDIR must be defined as well as LINK */
static	char Sccsid[]="@(#)dtree.c	2.3	2/14/84";

#ifdef LINK
static  char Rcsid[] ="$Header: /mnt/ntape/RCS/dtree.c,v 1.2 84/04/23 10:33:41 root Exp $";
#endif LINK

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifdef	STATS
# include	<pwd.h>
# include	<grp.h>
#endif	STATS
#ifdef	NEWDIR
# include	<sys/dir.h>
#else
# include	<sys/dir.h>
#endif	NEWDIR
#define	DEFDIRSIZ	255
#define DEFCOLWID       14
#ifndef MAXNAMLEN
# define MAXNAMLEN      DEFCOLWID
#endif
/*
 * TWIDDLE is a fudge factor.  It should be declared so that
 * sizeof(struct entry) is on a nice boundry.
 */
#define	TWIDDLE	4

#define	addr(b,o) ((struct entry *)\
			((int)(b) + (o)*(sizeof(struct entry)+Length-TWIDDLE)))
#define	SIZEOFentry	(sizeof(struct entry) + Length - TWIDDLE)

#define	DEPTH	10	/* maximum depth that dtree will go */
#define	MAX	100	/* initial # of elements for list of files */
#define	PWD	"/bin/pwd"	/* program to get name of current dir */
#define	DATE	"/bin/date"	/* program to print current date */

#define	FFIRST	2	/* sort files first */
#define DFIRST	1	/* sort directories first */
#define	FAIL	-1	/* failure return status of sys calls */
#define	GREATER	1	/* return value of strcmp if arg1 > arg2 */
#define	LESSTHAN -1	/* return value of strcmp if arg1 < arg2 */
#define	SAME	0	/* return value of strcmp if arg1 == arg2 */

int	Index = 0;		/* current element of list[]	*/
int	Length = DEFDIRSIZ;	/* max length of a dir. name    */
int     CLength = DEFDIRSIZ;    /* max length of a column       */
int	All = 0;		/* all != 0; list non-directory entries */
int	File_dir = 0;		/* flag for how to sort */
int	Sort = 1;		/* flag to cause sorting of entries */
int	Point = 1;		/* skip point files if set	*/
int	Maxes[DEPTH];		/* array keeps track of max length in columns */
int	Level = 0;		/* counter for how deep we are	*/
int	Device;			/* device that we are starting tree on */
int	Xdev = 1;		/* set to allow crossing of devices */
int	Varspaces = 1;		/* set to allow compaction of column width */
#ifdef	STATS
int	Gflag = 0;		/* set for group stats instead of owner */
int	Longflg = 0;		/* set for long listing */
int	Compact = 0;		/* set for shortened long listing */
#endif	STATS
struct	stat Status;
#ifdef  LINK
struct  stat Lstat;   /* stat of link, if there is one */
#endif  LINK

struct entry {
	int next;			/* index to next element in list */
					/* could be a ptr, but realloc() */
					/* might screw us then */
#ifdef	STATS
	off_t	e_size;			/* size in blocks */
	unsigned short	e_mode;		/* file mode */
	short	e_uid;			/* uid of owner */
	short	e_gid;			/* gid of owner */
#endif	STATS
	short unsigned dir : 1;		/* entry is a directory */
	short unsigned last : 1;	/* last entry in the dir. */
	short unsigned dev : 1;		/* set if same device as top */
	short unsigned end : 13;	/* index of last subdir entry*/
	char	e_name[TWIDDLE];	/* name from directory entry */
					/* it will actually be larger */
} *List, *SaveList;

unsigned Size;				/* how big of space we've malloced */

char	*Spaces;	/* used for output */
char	Buf1[BUFSIZ];	/* buffers for stdio stuff.  We don't want	*/
char	Buf2[BUFSIZ];	/* anyone calling malloc, because then		*/
char	Buf3[BUFSIZ];	/* realloc() will have to move the whole list	*/

main(argc, argv)
char	**argv;
int	argc;
{
	register int i, j = 0;
	int	flag = 0;	/* used to jump over 'c' argument */
	char	top[128];	/* array for treetop name */
	char	home[128];	/* starting dir for multiple trees */
	char	*ptr;
	char	*malloc();
	char	*rindex();
	FILE	*istr, *popen();

	setbuf(stdout, Buf1);

	for (j=1; j<argc; j++) {
		if (flag) {	/* saw a 'c' or 'b', so jump over value */
			if (flag > 1) j += flag-1;
			flag = 0;
			continue;
		}
		if (argv[j][0] == '-') {
			for (i = 1; i < strlen(argv[j]); i++) {
				switch (argv[j][i]) {
				case 'a':
					All = 1;
					break;
				case 'b':
					Length = atoi(argv[j+1+flag]);
					if (Length > MAXNAMLEN)
						Length = MAXNAMLEN;
					else if (Length < 1)
						Length = DEFCOLWID;
					flag += 1;
					break;
				case 'c':
					CLength = atoi(argv[j+1+flag]);
					if (CLength > MAXNAMLEN)
						CLength = MAXNAMLEN;
					else if (CLength < 1)
						CLength = DEFCOLWID;
					if (Length < CLength) Length = CLength;
					flag += 1;
					break;
				case 'd':
					File_dir = DFIRST;
					break;
				case 'f':
					File_dir = FFIRST;
					break;
				case 'n':
					Sort = 0;
					break;
				case 'p':
					Point = 0;
					break;
				case 'v':
					Varspaces = 0;
					break;
				case 'x':
					Xdev = 0;
					break;
#ifdef	STATS
				case 'g':
					Gflag = 1;
					break;
				case 'l':
					Longflg = 1;
					break;
				case 's':
					Compact = 1;
					break;
#endif	STATS
				default:
					fprintf(stderr,
						"Bad flag: %c\n", argv[j][i]);
					fprintf(stderr,
#ifdef	STATS
		"Usage: dtree -[adfglnpsvx] [-b filenamesize] [-c linelength] [directory ... ]\n");
#else	STATS
		"Usage: dtree -[adfglnpsvx] [-b filenamesize] [-c linelength] [directory ... ]\n");
#endif	STATS
					exit(FAIL);
				}
			}
		} else
			break;
	}
#ifdef	STATS
	if (Compact && !Gflag)
		Longflg = 1;
#endif	STATS

	/* Establish where we are (our home base...) */
	if ((istr = popen(PWD, "r")) == NULL) {
		fprintf(stderr,"dtree: %s failed\n", PWD);
		exit(1);
	} else {
		setbuf(istr, Buf2);
		if (fgets(home, 128, istr) == NULL) {
			fprintf(stderr, "dtree: EOF from %s\n", PWD);
			exit(1);
		}
	}
	pclose(istr);
	if (home[strlen(home)-1] == '\n')
		home[strlen(home)-1] = '\0';

	Spaces = malloc(Length+2);
	for(i = 0; i < Length + 1; i++)
		Spaces[i] = ' ';
	Spaces[i] = '\0';

	/* Get initial Storage space */
	Size = SIZEOFentry * MAX;
	SaveList = (struct entry *)malloc(Size);

	/* adjust for no specified directory */
	if (j == argc)
		argv[--j] = ".\0";

/*	system(DATE); Why in gods name did he do this? */	
/*	printf("\n");*/
	/* walk down the rest of the args, treeing them one at at time */
	for (; j < argc; j++) {
		if (chdir(home) == -1) {
			fprintf(stderr, "Can't chdir back to %s\n", home);
			exit(1);
		}
		sprintf(top, "%s", argv[j]);

		if (chdir(top) == FAIL) {
			fprintf(stderr, "Can't chdir to %s\n", top);
			continue;
		} else if ((istr = popen(PWD, "r")) == NULL) {
			fprintf(stderr,"dtree: %s failed\n", PWD);
			continue;
		} else {
			setbuf(istr, Buf2);
			if (fgets(top, 128, istr) == NULL) {
				fprintf(stderr, "dtree: EOF from %s\n", PWD);
				pclose(istr);
				continue;
			}
			ptr = top;
			while (*ptr && (*ptr != '\n'))
				ptr++;
			*ptr = '\0';
			pclose(istr);
		}

		List = SaveList; Index = 0;
		ptr = rindex(top, '/');

		if (!ptr || *++ptr == '\0')
			sprintf(addr(List,Index)->e_name, "%.*s", Length, top);
		else
			sprintf(addr(List,Index)->e_name, "%.*s", Length, ptr);

		if(stat(top, &Status) == FAIL) {
			fprintf(stderr, "Can't stat %s\n", top);
			continue;
		}
		Device = Status.st_dev;
		addr(List,0)->dir = 1;
		addr(List,0)->last = 1;
		addr(List,0)->next = 1;
#ifdef	STATS
		addr(List,0)->e_mode = Status.st_mode;
		addr(List,0)->e_uid = Status.st_uid;
		addr(List,0)->e_gid = Status.st_gid;
		addr(List,0)->e_size = Status.st_size;
#endif	STATS
		Index = 1;
		for (i = 1; i < DEPTH; i++)
			Maxes[i] = 0;
		Maxes[0] = stln(addr(List,0)->e_name);
		Level = 1;

		/* search the tree */
		addr(List,0)->end = t_search(top, addr(List,0));

		if (Index == 1)				/* empty tree */
			addr(List,0)->next = 0;

/*		if (All)
		    printf("\nDirectory structure and contents of %s\n", top);
		else
		    printf("\nDirectory structure of %s\n", top);
		if (Point)
			printf("(excluding entries that begin with '.')\n");
*/
		pt_tree();				/* print the tree */
	}
	exit(0) ;
}


t_search(dir, addrs)
char *dir;
struct	entry *addrs;
{
	int	bsort;			/* index to begin sort */
	int	stmp;			/* save temporary index value */
	struct entry *sstep;		/* saved step in list */
	int	nitems;			/* # of items in this directory */
#ifdef	NEWDIR
	DIR	*dirp;			/* pointer to directory */
#else
	FILE	*dirp;
#endif
	char	sub[MAXNAMLEN+1];	/* used for subdirectory names */
	int	i;
#ifdef	NEWDIR
	struct direct *dp;
#else
	struct direct dirent;
	struct direct *dp = &dirent;
#endif	NEWDIR
	int	n_subs = 0;
	int	tmp = 0;
	extern	qsort();
	int	compar();	/* comparison routine for qsort */
	char	*realloc();


#ifdef	NEWDIR
	dirp = opendir(".");
#else
	dirp = fopen(".", "r");
#endif	NEWDIR
	if (dirp == NULL) {
		fprintf(stderr, "Cannot open %s\n", dir);
		return(0);
	}
#ifndef	NEWDIR
	setbuf(dirp, Buf3);
#endif	NEWDIR

	bsort = Index;
	sstep = addr(List,bsort); /* initialize sstep for for loop later on */
	nitems = Index;
	/* get the entries of the directory that we are interested in */
#ifndef	NEWDIR
	while (fread((char *)(dp), sizeof(struct direct), 1, dirp) == 1) {
#else
	while ((dp = readdir(dirp)) != NULL) {
#endif	NEWDIR

		if (!dp->d_ino
		    || (strcmp(dp->d_name, ".") == SAME)
		    || (strcmp(dp->d_name, "..") == SAME)
		    || (Point && dp->d_name[0] == '.'))
				continue;

#ifndef NEWDIR						/***  Here's the fix ***/
		strncpy(sub, dp->d_name, 14);
		sub[14] = 0;
#else
		sprintf(sub, "%s", dp->d_name);
#endif	NEWDIR
#ifdef LINK
		if (lstat (sub,&Lstat) == FAIL) {
			fprintf(stderr, "%s:lstat can't find\n", sub);
			continue;
                }
#endif LINK
		if ((tmp = stat(sub, &Status)) == FAIL) {
			fprintf(stderr, "%s:stat can't find\n", sub);
			continue;
		}
#ifdef LINK
		if (((Lstat.st_mode & S_IFMT) == S_IFLNK)  &&
		    ((Status.st_mode & S_IFMT) == S_IFDIR)) 
		        addr(List,Index)->dir = 0;	
		else if ((((Lstat.st_mode & S_IFMT) == S_IFLNK) &&
			((Status.st_mode & S_IFMT) != S_IFDIR)) && (All)) 
                        addr(List,Index)->dir = 0;
#endif LINK
		else if ((Status.st_mode & S_IFMT) == S_IFDIR)
			addr(List,Index)->dir = 1;
		else if (All)
			addr(List,Index)->dir = 0;
		else
			continue;
		sprintf(addr(List,Index)->e_name, "%.*s", Length, dp->d_name);
		addr(List,Index)->last = 0;
		addr(List,Index)->end = 0;
#ifdef LINK
		if ((Lstat.st_mode & S_IFMT) == S_IFLNK) {
		     addr(List,Index)->dev = (Device == Lstat.st_dev);
		     addr(List,Index)->e_mode = Lstat.st_mode;
		     addr(List,Index)->e_uid = Lstat.st_uid;
		     addr(List,Index)->e_gid = Lstat.st_gid;
		     addr(List,Index)->e_size = Lstat.st_size;
                }
                else {
#endif LINK
		     addr(List,Index)->dev = (Device == Status.st_dev);
#ifdef STATS
		     addr(List,Index)->e_mode = Status.st_mode;
		     addr(List,Index)->e_uid = Status.st_uid;
		     addr(List,Index)->e_gid = Status.st_gid;
		     addr(List,Index)->e_size = Status.st_size;
#endif STATS
#ifdef LINK
                }
#endif LINK
		if (stln(addr(List,Index)->e_name) > Maxes[Level])
			Maxes[Level] = stln(addr(List,Index)->e_name);
		++Index;
		if (Index*SIZEOFentry >= Size) {
			Size += 20*SIZEOFentry;
			if (!(List =
			    (struct entry *)realloc((char *)List, Size))) {
				fprintf(stderr, "Out of space\n");
				break;
			}
		}
	}
#ifdef	NEWDIR
	closedir(dirp);
#else
	fclose(dirp);
#endif	NEWDIR

	nitems = Index - nitems;	/* nitems now contains the # of */
					/* items in this dir, rather than */
					/* # total items before this dir */

	if (Sort)
		qsort(addr(List,bsort), nitems, SIZEOFentry, compar);

	addr(List,Index-1)->last = 1;	/* mark last item for this dir */
	n_subs = nitems;
	stmp = Index;

	/* now walk through, and recurse on directory entries */
	/* sstep was initialized above */

	for (i = 0; i < nitems; sstep = addr(List,stmp - nitems+(++i))) {
		if (sstep->dir && (Xdev || sstep->dev)) {
			sstep->next = Index;
			sprintf(sub, "%.*s", Length, sstep->e_name);
			tmp = n_subs;
			Level++;
			if (chdir(sub) == FAIL)
				fprintf(stderr,
					"Can't chdir to %s/%s\n", dir, sub);
			else {
				n_subs += t_search(sub, sstep);
				if (chdir("..") == FAIL) {
					fprintf(stderr,
						"No '..' in %s/%s!\n",dir, sub);
					exit(1);
				}
			}
			--Level;
			if (n_subs - tmp <= 0)
				sstep->next = 0;
			else
				--n_subs;
		}
		else
			sstep->next = 0;
	}
	addrs->end = (unsigned)n_subs;
	return(n_subs);
}

/*
 *	comparison routine for qsort
 */

compar(a, b)
struct entry *a, *b;
{
	if (!File_dir)		/* straight alphabetical */
		return(strncmp(a->e_name, b->e_name, Length));

	/* sort alphabetically if both dirs or both not dirs */

	if ((a->dir && b->dir) || (!a->dir && !b->dir))
		return(strncmp(a->e_name, b->e_name, Length));

	if (File_dir == FFIRST) {	/* sort by files first */
		if (a->dir)
			return(GREATER);
		else
			return(LESSTHAN);
	}

	if (a->dir)			/* sort by dir first */
		return(LESSTHAN);
	else
		return(GREATER);
}


pt_tree()
{
	register int	i,j;
	struct entry *l;
	struct entry *hdr[DEPTH];
	int posit[DEPTH];		/* array of positions to print dirs */
	int con[DEPTH];			/* flags for connecting up tree */
	char	flag;			/* flag to leave blank line after dir */
	struct	entry *stack[DEPTH];	/* save positions for changing levels */
	int	top = 0;		/* index to top of stack */
	int	count = 1;		/* count of line of output */
#ifdef	STATS
	char	*getmode();
	char	*guid(), *ggid();
#endif	STATS

	Level = 0;			/* initialize Level */

	/* this loop appends each entry with dashes or spaces, for */
	/* directories or files respectively */

	for (i = 0; i < Index; i++) {
		for (j = 0; j < Length; j++) {
			if (!addr(List,i)->e_name[j])
				break;
		}
		if (addr(List,i)->dir) {
			for (; j < Length; j++)
				addr(List,i)->e_name[j] = '-';
		} else {
			for (; j < Length; j++)
				addr(List,i)->e_name[j] = ' ';
		}
	}

	/* adjust the Maxes array according to the flags */

	for (i = 0; i < DEPTH; i++) {
		if (Varspaces) {
			if (Maxes[i] > CLength )
				Maxes[i] = CLength;
		} else
			Maxes[i] = CLength;
	}

	/* clear the connective and position flags */

	for (i = 0; i < DEPTH; i++)
		con[i] = posit[i] = 0;

	/* this is the main loop to print the tree structure. */
	l = addr(List,0);
	j = 0;
	for (;;) {
		/* directory entry, save it for later printing */
		if (l->dir != 0 && l->next != 0) {
			hdr[Level] = l;
			posit[Level] = count + (l->end + 1)/2 - 1;
			flag = 1;
			stack[top++] = l;
			l = addr(List,l->next);
			++Level;
			continue;
		}

#ifdef	STATS
	do_it_again:
#endif	STATS
		/* print columns up to our entry */
		for (j = 0; j < (flag ? Level-1 : Level); j++) {
			if (!flag && posit[j] && posit[j] <= count) {
				/* time to print it */
				if (hdr[j]->e_name[CLength-1] != '-')
					hdr[j]->e_name[CLength-1] = '*';
				printf("|-%.*s",Maxes[j],hdr[j]->e_name);
				posit[j] = 0;
				if (hdr[j]->last != 0)
				    con[j] = 0;
				else
				    con[j] = 1;
#ifdef	STATS
				if (Gflag || Longflg) {
				    if ((i = j+1) <= Level)
					printf("| %.*s", Maxes[i], Spaces);
				    for (i++; i <= Level; i++) {
					printf("%c %.*s",
					    (con[i] ? '|' : ' '),
					    Maxes[i], Spaces);
				    }
				    if (!Compact) {
					printf("%s ", getmode(hdr[j]->e_mode));
					if (Longflg)
					   printf("%8.8s ",guid(hdr[j]->e_uid));
					if (Gflag)
					   printf("%8.8s ",ggid(hdr[j]->e_gid));
					printf("%7ld\n",
					    (hdr[j]->e_size+511L)/512L);
				    } else {
					printf(" %04o ",hdr[j]->e_mode & 07777);
					if (Longflg)
					    printf("%5u ", hdr[j]->e_uid);
					if (Gflag)
					    printf("%5u ", hdr[j]->e_gid);
					printf("%7ld\n",
					    (hdr[j]->e_size+511L)/512L);
				    }
				    goto do_it_again;
				}
#endif	STATS
			} else
				printf("%c %.*s", (con[j] ? '|' : ' '),
					Maxes[j], Spaces);
		}
		if (flag) {	/* start of directory, so leave a blank line */
			printf(con[j] ? "|\n" : "\n");
			flag = 0;
			continue;
		} else {
				/* normal file name, print it out */
			if (l->e_name[CLength-1] != '-' &&
			    l->e_name[CLength-1] != ' ')
			    l->e_name[CLength-1] = '*';
			printf("|-%.*s",Maxes[Level],l->e_name);
			if (l->last) {
				con[j] = 0;
			} else {
				con[j] = 1;
			}
#ifdef	STATS
			if (Gflag || Longflg) {
				if (Compact) {
					printf(" %04o ", l->e_mode & 07777);
					if (Longflg)
					    printf("%5u ", l->e_uid);
					if (Gflag)
					    printf("%5u ", l->e_gid);
					printf("%7ld",
					    (l->e_size+511L)/512L);
				} else {
					printf("%s ", getmode(l->e_mode));
					if (Longflg)
					    printf("%8.8s ",guid(l->e_uid));
					if (Gflag)
					    printf("%8.8s ",ggid(l->e_gid));
					printf("%7ld",
					    (l->e_size+511L)/512L);
				}
			}
#endif	STATS
		}
		printf("\n");

		if (l->last) {
			/* walk back up */
			while (l->last) {
				--Level;
				if (--top <= 0)
					return;
				l = stack[top];
			}
		}
		l = addr(l,1);
		++count;
	}
}

#ifdef	STATS

char *
guid(uid)
short uid;
{
	static char tb[10];
	extern struct passwd *getpwuid();
	struct passwd *pswd;

	pswd = getpwuid(uid);
	if (pswd == NULL)
		sprintf(tb,"%u", uid);
	else
		sprintf(tb, "%8s", pswd->pw_name);
	return(tb);
}

char *
ggid(gid)
short gid;
{
	static char tb[10];
	extern struct group *getgrgid();
	struct group *grp;

	grp = getgrgid(gid);
	if (grp == NULL)
		sprintf(tb,"%u", gid);
	else
		sprintf(tb, "%8s", grp->gr_name);
	return(tb);
}

/* take the mode and make it into a nice character string */

char	*
getmode(p_mode)
unsigned short p_mode;
{
	static char a_mode[16];
	register int	i = 0, j = 0;

	a_mode[j++] = ' ';

	switch (p_mode & S_IFMT) {
#ifdef LINK
	case S_IFLNK:
		a_mode[j++] = 'l';
		break;
#endif LINK
	case S_IFDIR:
		a_mode[j++] = 'd';
		break;
#ifdef	S_IFMPC		/* defined in stats.h if you have MPX files */
	case S_IFMPC:
		a_mode[j-1] = 'm';
		/* FALL THROUGH */
#endif	S_IFMPC
	case S_IFCHR:
		a_mode[j++] = 'c';
		break;
#ifdef	S_IFMPB		/* defined in stats.h if you have MPX files */
	case S_IFMPB:
		a_mode[j-1] = 'm';
		/* FALL THROUGH */
#endif	S_IFMPB
	case S_IFBLK:
		a_mode[j++] = 'b';
		break;
	case S_IFREG:
	default:
		a_mode[j++] = (p_mode & S_ISVTX) ? 't' : ' ';
		break;
	}
	a_mode[j++] = ' ';
	for( i = 0;i<3;i++ ) {
		a_mode[j++] = (p_mode<<(3*i) & S_IREAD) ? 'r' : '-';
		a_mode[j++] = (p_mode<<(3*i) & S_IWRITE) ? 'w' : '-';
		a_mode[j++] = (i<2 && (p_mode<<i & S_ISUID)) ? 's' :
			      ((p_mode<<(3*i) & S_IEXEC ) ? 'x' : '-');
		a_mode[j++] = ' ';
	}
	a_mode[j] = '\0';
	return(a_mode);
}
#endif

/* stln - sortof like strlen, returns length up to Length-1 */
stln(st)
register char *st;
{
	register int t;

	for (t=0; t<Length-1; ++t)
		if (!st[t])
			return (++t);
	return (++t);
}

/*
 * Return a pointer into str at which the character
 * c last appears; NULL if not found.
*/

char *
rindex(str, c)
register char *str, c;
{
	register char *ptr;

	ptr = NULL;
	do {
		if (*str == c)
			ptr = str;
	} while (*str++);
	return(ptr);
}
