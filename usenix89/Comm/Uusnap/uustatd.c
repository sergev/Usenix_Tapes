/*
 * uustatd.c
 * dennis bednar 05 23 86
 * Computer Consoles Inc, 11490 Commerce Park Drive, Reston VA 22091
 * seismo!rlgvax!dennis (UUCP address)
 *
 * uucp statistics program.  "d" stands for dennis to avoid confusion
 * with the standard "uustat" program.
 *
 * Read C. "work" files in the current directory, and
 * print statistics of what we have for outgoing traffic.
 *
 * Read X. "local execute" files in the current directory, and
 * print statistics of what we have for incoming traffic.
 *
 *	-x99 = debug flag
 *	-s<sys> = machine name or system name
 *	-dir<dir> = process directory other that /usr/spool/uucp
 *
 *
 *
 * NOTE: This command ONLY works on a version of uucp in which
 * there are *no* subdirectories in the UUCPSPOOLDIR.  It will
 * *NEED* modification to work on versions of uucp which
 * have sub-directories.  If you decide to modify this code,
 * then I suggest you #ifdef it with SUBDIR so that it can easily
 * be compiled to work with either kind of machine.  Send modifications
 * back to the original author (dennis@rlgvax.uucp).
 *
 * The external directory reading routines can be #ifdef'ed
 * for V7/S3/S5 14 character fixed length names or can be
 * #ifdefed for 4.2BSD variable (up to 255 chars) file names.
 *
 */

#include <stdio.h>
#include <ctype.h>	/* isdigit() */
#include <sys/types.h>
#include <sys/stat.h>
#include <dir.h>

#define TRUE		1
#define	FALSE		0
#define STR_SAME	!strcmp
#define STR_DIFF	strcmp
#define STRN_SAME	!strncmp
#define STRN_DIFF	strncmp
#define	MAXFILELEN	100
#define MAXLINE	256

/* routines that read directory entries */
extern	int	setdir();
extern	char	*getdir();

extern	char	*malloc();

#define MAXMCHNAME	20
struct statblock {
	/* s_next *must* be first */
	struct	statblock	*s_next;	/* next one in queue */

	char	s_mchname[MAXMCHNAME+1];	/* store machine name here */
	long	s_csize;	/* number of chars in all C. command files */
	long	s_dsize;	/* number of chars in all D. data files */
	long	s_xsize;	/* number of chars in all X. xqt files */
	int	s_numq;		/* number queued up */
	};

struct statblock	*getstatp();	/* get statistics block pointer */
long	filesize();
char	*u_errmesg();
char	*getmchname();
char	*getl_file();

char	*cmd;
int	debug = 0;	/* >=1 prints Warning msgs. >=10 prints all */
char	*flag_mch;	/* only interested in this machine */
char	*flag_dir;	/* special case to work in this directory
			 * instead of default UUCPSPOOLDIR.
			 */

/* shared by getstatb(), dump_stat(), and clean_stat() */
static	struct statblock	*mchlist = NULL;

#define UUCPSPOOLDIR "/usr/spool/uucp"


main(argc, argv)
	int	argc;
	char	*argv[];
{
	char	*wdir;		/* name of directory to work in */

	cmd = argv[0];

	getargs(argc, argv);

	if (flag_dir)
		wdir = flag_dir;	/* user overridden directory */
	else
		wdir = UUCPSPOOLDIR;	/* default directory */

	if (chdir( wdir ) == -1)
		{
		fprintf(stderr, "%s: cannot chdir(%s)\n", cmd, wdir);
		exit(1);
		}

	pass1();

	pass2();

	exit(0);

}


/*
 * Outgoing traffic is processed
 * Also each D. and X. file "attached" to a C. file is recorded now.
 * Later in pass 2, when the entire directory is rescanned, we
 * will report those "detached" D. and X. files.
 */
pass1()
{
	char	*fname;		/* file name */

	/* open the current directory */
	if ( setdir(".") == -1)
		{
		fprintf(stderr, "%s: cannot open directory\n", cmd);
		exit(1);
		}


	/* read until eof on directory */
	while ( (fname = getdir()) != (char *)NULL)
		{
		if (debug > 10)
			printf("%s\n", fname);
		process("C.",  fname);
		}


	if (mchlist)		/* have statistics for at least one machine */
		{
		printf("	Outgoing Traffic\n");
		dump_stat();
		clean_stat();
		}

}

/*
 * Incoming traffic is processed
 */
pass2()
{
	char	*fname;		/* file name */

	/* open the current directory */
	if ( setdir(".") == -1)
		{
		fprintf(stderr, "%s: cannot open directory\n", cmd);
		exit(1);
		}


	/* read file names */
	while ( (fname = getdir()) != (char *)NULL)
		{
		if (debug > 10)
			printf("%s\n", fname);
		if (STRN_SAME(fname, "X.", 2) || STRN_SAME(fname, "D.", 2))
			chk_detach(fname);	/* see if detached */
		process("X.", fname);
		}

	if (mchlist)
		{
		printf("\n	Incoming Traffic (Unprocessed XQT files)\n");
		dump_stat();
		/* don't worry about clearing statistics 2nd and last time */
		}
}


/*
 * only process the C. files, because they contain text info
 * that points to the attached D. datafile, and attached X.
 * execute file.
 *
 * For example, for outgoing traffic:
 * the file C.dsi1AD0060 contains:
S D.dsi1BC0060 D.dsi1BC0060 network - D.dsi1BC0060 0666 network 
S D.rlgvaxXA0060 X.rlgvaxXA0060 network - D.rlgvaxXA0060 0666 network 
 * the file D.dsi1BC0060 contains a news batch created by 'batch',
 * and the fiile D.rlgvaxXA0060 contains the commands:
U network rlgvax
F D.dsi1BC0060
I D.dsi1BC0060
C rnews 
 *
 * For incoming traffic, there is an X.<remotesite>... file that
 * contains an "F" or "I" line which contains the name of the
 * local file which will be processed by /usr/lib/uuxqt.
 */
process(prefix, cfile)
	char	*cfile;			/* name of C. command file */
{
	FILE	*fp;
	char	*rtn1,
		*rtn2;
	char	line1[MAXLINE+2];
	char	line2[MAXLINE+2];	/* room for newline, null */
	char	*mch;			/* name of machine */
	char	*dfile;			/* name of datafile */
	char	*xfile;			/* name of execute file */
static	char	ddfile[MAXFILELEN+1];	/* room for null */

	if (debug > 10) printf("process(%s) called\n", cfile);

	if (!isregfile(cfile))
		{
		if (debug >= 1)
		fprintf(stderr, "%s: Warning, ignoring irregular file %s\n", cmd, cfile);
		return;
		}

	/* skip files that don't begin with file prefix */
	/* in pass1, (Outgoing files), skip files that don't begin with C. */
	/* in pass2, (Incoming files), skip files that don't begin with X. */
	if (STRN_DIFF(cfile, prefix, strlen(prefix)))
		return;

	/* skip files that don't have the desired machine embedded in them */

	mch = getmchname(cfile);
	if (debug > 10) printf("process: machine name was %s\n", mch);
	if (flag_mch && STR_DIFF(mch, flag_mch))
		return;


	fp = fopen(cfile, "r");
	if (fp == NULL)
		{
		fprintf(stderr, "%s: cannot open %s: %s\n", cmd, cfile, u_errmesg());
		exit(1);
		}

	/* if premature EOF on either line, ignore this file */

	if (STR_SAME(prefix, "C."))
		{
		/* process outgoing C. "work" file */
		rtn1 = fgets(line1, sizeof(line1), fp);

		rtn2 = fgets(line2, sizeof(line2), fp);

		if ( (rtn1 == NULL) || (rtn2 == NULL) )
			{
			if (debug >= 1) fprintf(stderr, "%s: Warning ignorning file w/o 2 lines: %s\n", cmd, cfile);
			return;
			}

		}
	else
		{
		/* process incoming X. file */
		/* in the X. file, search for an F file directive */
		/* the file name after it refers to a file on the local mch */
		while (fgets(line1, sizeof(line1), fp) != NULL)
			{
			if (line1[0] == 'F')
				break;
			}
		if (debug > 2) printf("read F line <%s>\n", line1);
		}

	fclose(fp);


	/* Warning *must* copy static dfile to ddfile because getl_file
	 * reuses the static buffer within.
	 */

	if (STR_SAME(prefix, "C."))
		{
		/* process outgoing C. file */
		dfile = getl_file("S ", line1);	/* get name of D. file from line */
		if (dfile && debug > 10) printf("process: Data File = %s\n", dfile);
		if (dfile) strcpy(ddfile, dfile);

		xfile = getl_file("S ", line2, cfile);
		if (xfile && debug > 10) printf("process: XQT  File = %s\n", xfile);

		if (dfile && xfile)
			add_stat(mch, cfile, ddfile, xfile);
		else
			fprintf(stderr, "%s: Warning, possible bad format in file '%s'\n", cmd, cfile);
		}
	else
		{
		/* process incoming X. file */
		xfile = cfile;		/* file name passed */
		dfile = getl_file("F ", line1, cfile);
		if (dfile)
			add_stat(mch, NULL, dfile, xfile);
		else
			fprintf(stderr, "%s: Warning, skipping file '%s'\n", cmd, cfile);
		}

}

/*
 * return UNIX error message associated with errno
 * more flexible than perror(3)
 */

char *
u_errmesg()
{
	extern	int	errno;
	extern	int	sys_nerr;
	extern	char	*sys_errlist[];
static	char	buffer[50];

	if (errno < 0 || errno >= sys_nerr)
		{
		sprintf( buffer, "errno %d undefined (%d=max)", errno, sys_nerr);
		return(buffer);
		}

	return( sys_errlist[errno] );
}


/*
 * return true if the file is a regular file (ie not directory, pipe, etc.)
 */
isregfile(fname)
	char	*fname;		/* file name terminated by null */
{
	int	fd;		/* file descriptor */
	struct	stat statbuf;	/* read info about a file into here */
	int	rtn;		/* return code from system call */

	fd = open(fname, 0);
	if (fd == -1)
		{
		fprintf(stderr, "%s: isregfile: cannot open %s: %s\n", cmd, fname, u_errmesg());
		return FALSE;
		}

	rtn = fstat(fd, &statbuf);
	if (rtn == -1)
		{
		fprintf(stderr, "%s: cannot stat %s: %s\n", cmd, fname, u_errmesg());
		exit(1);
		}

	close(fd);		/* ignore error */

	if ( (statbuf.st_mode & S_IFMT) == S_IFREG)	/* is a reg file */
		return TRUE;
	else
		return FALSE;

}



/*
 * return number of characters in a file
 */
long
filesize(fname)
	char	*fname;		/* file name terminated by null */
{
	int	fd;		/* file descriptor */
	struct	stat statbuf;	/* read info about a file into here */
	int	rtn;		/* return code from system call */

	/* no file name was passed */
	if (fname == NULL)
		return 0L;

	fd = open(fname, 0);
	if (fd == -1)
		{
		if (debug >= 1) fprintf(stderr, "%s: Warning, cannot open %s to get size: %s\n", cmd, fname, u_errmesg());
		return 0L;
		}

	rtn = fstat(fd, &statbuf);
	if (rtn == -1)
		{
		fprintf(stderr, "%s: cannot stat %s: %s\n", cmd, fname, u_errmesg());
		exit(1);
		}

	close(fd);		/* ignore error */

	return (long) statbuf.st_size;
}



/*
 * get machine name from a UUCP file name
 * format is D.machineXY0000
 * 	     C.machineAY0000
 * "machine" could end with a digit such as "dsi1"
 *
 */
char	*
getmchname(fname)
	char	*fname;		/* CANNOT have slash in file name */
{
	register	char	*cp;
#define MCHNAMESIZE	20
	static	char	mchname[MCHNAMESIZE+1];	/* room for null */
	int	len;

	cp = fname;

	cp += strlen(fname);	/* points to null terminator */

	--cp;			/* last char of fname */

	/* skip backwards over digits until a letter is seen */
	/* this works even if the number of digits is not 4 */
	while ( (cp > fname) && isdigit(*cp))
		--cp;

	/* have come to 2nd letter of pair */
	cp--;		/* cp now points to first letter of pair, or one
			 * past the machine name.
			 */

	len = (cp - (fname+2) );	/* compute length of machine name */

	if ( (len <= 0) || (len > MCHNAMESIZE) )
		{
		fprintf(stderr, "%s: getmchname(%s) failed\n", cmd, fname);
		exit(1);
		}

	strncpy(mchname, fname+2, len);
	mchname[len] = '\0';		/* strncpy doesn't copy a NULL */

	return mchname;
}



/*
 * get name of file from the line that looks like:
S file ... more stuff
 */
char	*
getl_file(prefix, line, rdfile)
	char	*prefix;	/* either "S " or "F " */
	char	*line;		/* line of data from 'rdfile' */
	char	*rdfile;	/* name of file in case of error */
{
	static	char	filename[MAXFILELEN+1];	/* room for null */
	char	*cp;
	char	*dp;
	int	len;

	/* skip over the "S " or "F " part of the line */
	cp = line;
	if (STRN_DIFF(line, prefix, strlen(prefix)))
		{
badformat:
		fprintf(stderr, "%s: prefix '%s' expected in file '%s', line '%s'\n", cmd, prefix, rdfile, line);
		return NULL;
		}
	cp += 2;

#define iswhite(c) ( (c == ' ') || (c == '\t') || (c == '\n') || (c == '\0'))

	if ( iswhite(*cp) )
		{
		fprintf(stderr, "%s: iswhite char (%c) found at line+2\n", cmd, *cp);
		goto badformat;
		}


	/* copy the file name from 'cp' to 'filename' */

	dp = filename;
	len = 0;
	while ( !iswhite(*cp))
		{
		*dp++ = *cp++;
		if (++len > MAXFILELEN)
			{
			fprintf(stderr, "%s: filename in line %s was too long\n", cmd, line);
			exit(1);
			}
		}

	*dp = '\0';

	return filename;
}


/*
 * add statistics for this machine name.
 */
add_stat(mch, cfile, dfile, xfile)
	char	*mch,
		*cfile,
		*dfile,
		*xfile;

{
	struct	statblock	*sp;

	if (debug > 10)
	printf("addstat(mch=%s, cfile=%s, dfile=%s, xfile=%s) called\n",
	 mch, cfile, dfile, xfile);


	/* get an existing or brand-new statistics block for this machine */
	sp = getstatp(mch);

	if (cfile)
		sp->s_csize += filesize(cfile);

	if (dfile)
		{
		sp->s_dsize += filesize(dfile);
		addtolist(dfile);		/* D. file is attached */
		}

	if (xfile)
		{
		sp->s_xsize += filesize(xfile);
		addtolist(xfile);		/* X. file is attached */
		}

	sp->s_numq++;

	if (debug > 10) printf("add_stat:	%s	%ld	%ld	%ld	%d\n",
	 sp->s_mchname, sp->s_csize, sp->s_dsize, sp->s_xsize, sp->s_numq);
}


/*
 * get a pointer to a statistics block.
 * Search the existing list, if found, return it.
 * Otherwise allocate a new block, hook onto end of list, zero out
 * statistics counters, and return it.
 */


struct	statblock *
getstatp(mch)
	char	*mch;
{
	struct	statblock	**p;	/* previous item.ptr in list */
	struct	statblock	*n;	/* next item in list */
	struct	statblock	*new;	/* new malloced statistics block */

	/* search existing list */
	for (p = &mchlist; n = *p; p = (struct statblock **)n)
		{
		if (STR_SAME(mch, n->s_mchname))
			return n;
		}

	/* not found, p points to ptr where new stat block ptr will be stored */
	/* note how this code is cleverly written to work even if the
	 * list is empty the first time, or when the list is non-empty
	 * the 2nd, 3rd, etc. times.
	 */

	new = (struct statblock *)malloc(sizeof(struct statblock));
	if (new == NULL)
		{
		fprintf(stderr, "%s: malloc failed\n", cmd);
		exit(1);
		}

	/* hook onto end of existing list if any */
	*p = new;

	new->s_next = NULL;		/* ground end of linked list */
	strcpy(new->s_mchname, mch);	/* store new machine name */
	new->s_csize = new->s_dsize = new->s_xsize = 0L;
	new->s_numq = 0;

	return new;
}


dump_stat()
{
	struct	statblock	*p;

	p = mchlist;

	printf("Machine	C Bytes	D Bytes	X Bytes	Files\n");

	while (p)
		{
		printf("%s	%ld	%ld	%ld	%d\n",
		 p->s_mchname, p->s_csize, p->s_dsize, p->s_xsize, p->s_numq);
		p = p->s_next;
		}
}

/*
 * discard old statistics by freeing statistics blocks in linked list
 */
clean_stat()
{
	struct	statblock	*p,	/* current statblock item */
				*n;	/* next one after this one */

	for (p = mchlist; p; p = n)
		{
		n = p->s_next;		/* save next item ptr ... */
		free(p);		/* before discarding current item */
		}

	mchlist = NULL;			/* list is empty again */
}


/*
 * get arguments from command line, assign global flag variables
 */
getargs(argc, argv)
	int	argc;
	char	**argv;
{
	register	int	i;

	if (argc != 1)
		for (i = 1; i < argc; ++i)
			{
			if ( STRN_SAME(argv[i], "-x", 2) )
				debug = atoi(&argv[1][2]);
			else if (STRN_SAME( argv[i], "-s", 2) )
				flag_mch = argv[i]+2;
			else if (STRN_SAME( argv[i], "-dir", 4) )
				flag_dir = argv[i]+4;
			}
}

struct node {
	struct list	*l_next;	/* MUST be first, next item in list */
	char		l_name[1];	/* domain name + 1 for null */
	};

struct node	*head = NULL;

/*
 * standard FIFO implemented by linked list.
 * NO sorting.
 */
addtolist(name)
	char	*name;
{
	register	struct	node	**p,	/* previous item */
					*n;	/* next item */
	struct	node	*np;			/* node pointer */

	/* find node pointed to by p which is the end of the list to tack onto */
	for (p = &head; n = *p; p = (struct node **)n)
		if (STR_SAME(n->l_name, name))
			return;		/* already in list */

	/* use same trick that cpio uses */
	np = (struct node *) malloc( sizeof(struct node) + strlen(name) );

	if (np == NULL)
		{
		perror("out of memory");
		exit(1);
		}

	/* stuff node with domain name */
	np->l_next = NULL;	/* ground end of list */
	strcpy(np->l_name, name);

	/* hook onto end of list */
	*p = np;


/*	printf("dbg: %s\n", name);	/* print domains as we hit them */

	/* done */
}


/*
 * search the linked list, returning TRUE iff found.
 * The arg passed will be a D. or X. file name saved during
 * pass 1, hopefully.  If not, its detached.
 */
inlist(fname)
	char	*fname;
{
	register	struct	node	**p,	/* previous item */
					*n;	/* next item */

	for (p = &head; n = *p; p = (struct node **)n)
		if (STR_SAME(n->l_name, fname))
			return 1;
	return 0;
}

chk_detach(fname)
	char	*fname;
{
	if (!inlist(fname))
		fprintf(stderr, "%s: Warning: %s is detached\n", cmd, fname);
}

