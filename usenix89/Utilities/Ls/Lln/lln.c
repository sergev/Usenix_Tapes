
/* define your OS type:
 * USG			S3 or S5
 * BSD4X		V7 or 4.2 or 4.3
 * CCI632		Sys5 r2 on CCI Power 6/32
 */
#define CCI632	1

/* the OS type, in turn, defines the lower level #defines that
 * are typical for that system.  If your system is usual,
 * add its type either alongside USG or BSD4X, otherwise if
 * your machine is weird, add it near last #else.
 */
#ifdef USG
#	undef MTAB	/* use /etc/mnttab and mnttab.h */
#	undef BSD_FILESYS	/* use 14 char fix len dir entries */
#	undef BSD_SYMLINK	/* don't have symbolic links */
#else	/* !USG */
/* CCI632 Sys5r2 really has a 4.2 engine under the hood :-) */
#if defined(BSD4X) || defined(CCI632)
#	define MTAB 1		/* have /etc/mtab and mtab.h instead of /etc/mnttab and mnttab.h */
#	define BSD_FILESYS 1	/* have BSD4_2 file system */
#else	/* !BSD4X && !CCI632 && !USG */
	Unknown System.  This causes a compiler error.
#endif /* BSD4X */
#endif /* USG */

#undef DEBUG		/* enables debug code */
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#if defined(MTAB)
#include <fstab.h>
#include <sys/param.h>	/* NMOUNT */
#include <mtab.h>
#define	MNTTAB	mtab
#define MT_FILSYS	m_path
/* MT_FILSYS is field in mtab with directory name */
#else
#include <mnttab.h>
#define MNTTAB	mnttab
#define MT_FILSYS	mt_filsys
#endif
#ifdef BSD_SYMLINK
#define	STAT	lstat
	/* special stat() to handle regular and symbolically linked files */
#else
#define STAT	stat	/* regular old stat() sys call */
#endif
#define BOOL	char


/*
	lln: list links for files

	not (c) 1986 Paul Heffner @ AT&T-IS Altamonte Springs, FL.
		 use it however you wish.

	rlgvax!dennis 01 06 86 Added MTAB, and BSD_FILESYS,
	and BSD_SYMLINK #ifdefs to work on CCI632.
	Enquiries, advice, and calm criticism to:

	ihnp4   \
	akgua   - bsdpkh!heff
	attmail /
*/

#ifdef MTAB
#define MTE_SIZE	sizeof(struct mtab)
#else
#define MTE_SIZE	sizeof(struct mnttab)
#endif
#define DIRESIZ		sizeof(struct direct)
#define PATHMAX	512
#define STR_SAME	!strcmp
#if defined(BSD_FILESYS)
#if !defined(DEBUG)
#define OPENDIR	opendir
#define CLOSEDIR closedir
#else /* debug */
/* use OPENDIR(), CLOSEDIR() debug routines that are default */
/* don't redefine OPENDIR as OPENDIR, because macro recursion expansion */
DIR	*OPENDIR();	/* fw ref */
void	CLOSEDIR();	/* fw ref */
#endif
#endif
char	*u_errmesg();	/* fw ref - return perror() like string */

extern int errno;

/*
	program globals
*/
static int inn;		/* inode number of file being searched for... */
static int found;	/* count of found directory entries.. */
static int need;	/* count of directory entries to find.. */
static int curdev;	/* device # of disk holding fs of objective file. */
static char pathbuf[PATHMAX];	/* holds current search path */

main(argc,argv)
int argc;
char **argv;
{
#ifdef MTAB
static char *mtfil = "/etc/mtab";
#else
static char *mtfil = "/etc/mnttab";
#endif
	struct stat ssfil, junk1, junk2, ssfs;
	/* junk1, junk2 are dummy vars because lstat() would write
	 * past end of ssfil into ssfs, and corrupt it otherwise.
	 * lstat(2) bug on rlgvax!!!
	 */
	struct MNTTAB mte;
	int i, mth, x;
#ifdef	BSD_FILESYS
	DIR	*dirfd;
#else
	int	dirfd;		/* 14 char fixed length directory entries */
#endif

	if (argc < 2) exit(0);

	fclose(stdin);	/* free up an extra fd (just in case) */

	if ((mth = open(mtfil,O_RDONLY)) == -1)
	{	fprintf(stderr,"%s: errno %d, cannot open %s\n",argv[0],errno,mtfil);
		exit(16);
	}
#ifdef DEBUG
	printf("opened mount table file okay\n");
#endif
	for (i = 1; i < argc; ++i)
	{
		if (STAT(argv[i],&ssfil))
		{	fprintf(stderr,"%s:errno %d can not access %s\n",argv[0],errno,argv[i]);
			continue;
		}
		if ((ssfil.st_mode & S_IFMT) == S_IFDIR)
		{	fprintf(stderr,"%s: %s is a directory file.\n",argv[0],argv[i]);
			continue;
		}
		inn = ssfil.st_ino;	/* establish search inode number */
		found = 0;		/* set count of # found */
		need = ssfil.st_nlink;	/* set count of # of links */
		curdev = ssfil.st_dev;	/* # of device holding file */
#ifdef DEBUG
		printf("lln: debug: dev = %d, inode = %d, links = %d\n",
		curdev, inn, need);
#endif

/*
	Scratch through mount table looking for fs hosting file
	seeking a match on device number...
*/

		for (x = 0; x < NMOUNT; ++x)
		{	if (read(mth,&mte,MTE_SIZE) != MTE_SIZE)
			{	fprintf(stderr,"%s: errno %d reading mnttab\n",argv[0],errno);
				exit(32);
			}
			STAT(mte.MT_FILSYS,&ssfs);	/* name of directory mounted as */
#ifdef DEBUG
			printf("lln: debug: Looking at mounted dir %s\n", mte.MT_FILSYS);
			printf(" st_dev=%d, st_rdev=%d, want dev # = %d\n", ssfs.st_dev, ssfs.st_rdev, curdev);
#endif	/* debug */
			if (ssfs.st_dev == ssfil.st_dev) break;
#ifdef DEBUG
			else printf("debug: wrong dir (%d != %d)\n", 
			ssfs.st_dev, ssfil.st_dev);
#endif
		}

#ifdef DEBUG
		printf("lln: debug: found mounted dir starting point\n");
#endif

/*
	The path name is constructed in 'pathbuf'.
	We start with mounted directory from /etc/mnttab entry...
*/
#ifdef BSD_FILESYS
		strncpy(pathbuf,mte.MT_FILSYS,strlen(mte.MT_FILSYS)+1);
#else
		strncpy(pathbuf,mte.MT_FILSYS,14);
#endif
#ifdef DEBUG
		printf("lln: debug: search starting at mounted dir %s\n", pathbuf);
#endif
		if (pathbuf[1] == '\0') pathbuf[0] = '\0';	/* special case for root fs (avoids // at front of path) */
		if (chdir(mte.MT_FILSYS) == -1)
			{
			fprintf(stderr, "lln: chdir(%s) failed\n", mte.MT_FILSYS);
			exit(1);
			}
#ifdef BSD_FILESYS
		if ((dirfd = OPENDIR(mte.MT_FILSYS)) == (DIR *)NULL)
#else
		if ((dirfd = open(mte.MT_FILSYS,O_RDONLY)) == -1)
#endif
		{	fprintf(stderr,"%s: errno %d opening %s\n",argv[0],errno,mte.MT_FILSYS);
			exit(-1);
		}
		if (lnsrch(dirfd))
			fprintf(stderr,"%s incomplete list: only %d of %d links were found.\n",argv[0],found,need);
#ifdef BSD_FILESYS
		CLOSEDIR(dirfd);
#else
		close(dirfd);
#endif
		lseek(mth,0L,0);	/* reset to beginning of mnttab */
	}
	close(mth);
	exit(0);
}

lnsrch(dirfd)
#ifdef BSD_FILESYS
	DIR	*dirfd;
#else
int dirfd;
#endif
{	struct stat curf;
#ifdef BSD_FILESYS
	struct direct *cd;
	DIR	*newdfd;
#else
	struct direct cd;
	int newdfd;
#endif
	char *pathend;
	BOOL	isdir;
#ifdef BSD_SYMLINK
	BOOL	issymlink;
#endif

#ifdef BSD_FILESYS
	/* skip over . and .. in read loop of directory */
#else
	lseek(dirfd,(long) (DIRESIZ << 1),0);	/* go past . and .. */
	/* BUG FIX (long) added for mchs with diff size int & long. rlgvax!dennis */
#endif
#ifdef BSD_FILESYS
	while (cd = readdir(dirfd) )
	{
		if (STR_SAME(cd->d_name, ".") || STR_SAME(cd->d_name, "..") )
			continue;	/* skip . and .. entries */
		if (STAT(cd->d_name,&curf))
		{	fprintf(stderr,"lln: stat error %s/%s\n",pathbuf,cd->d_name);
#else
	while (read(dirfd,&cd,DIRESIZ) == DIRESIZ)	/* bug fix - don't let -1 be okay, rlgvax!dennis */
	{
	if (!cd.d_ino) continue;	/* skip empty entries (i = 0) */
		if (STAT(cd.d_name,&curf))
		{	fprintf(stderr,"lln: stat error %s/%s\n",pathbuf,cd.d_name);
#endif
			continue;
		}
		isdir =  ((curf.st_mode & S_IFMT) == S_IFDIR);
#ifdef BSD_SYMLINK
		issymlink = ((curf.st_mode & S_IFMT) == S_IFLNK);
		if (isdir && issymlink)
			continue;	/* avoid infinite recursion */
		/* recursion when /a/b/c symlinked to /a */
#endif
		if ( isdir )		/* sub-dir?? */
		{
/*
	ignore MT directories and keep search to objective fs (dev)
*/
#ifdef BSD_FILSYS
			if (curf.st_dev != curdev) continue; /* diff mount point */
#else
			if (curf.st_size == (2*DIRESIZ) || /* . && .. ie dir empty */
			    curf.st_dev != curdev) continue; /* wrong mount pt */
#endif

#ifdef BSD_FILESYS
#ifdef DEBUG
			/* track down cannot visit problem */
			printf("Will OPENDIR(%s) Current directory is ", cd->d_name);
			fflush(stdout);
			system("pwd");
#endif
			if ((newdfd = OPENDIR(cd->d_name)) == (DIR *)NULL)
			{	fprintf(stderr,"lln: Can't open %s/%s: %s\n",pathbuf,cd->d_name, u_errmesg());
#else
			if ((newdfd = open(cd.d_name,O_RDONLY)) == -1)
			{	fprintf(stderr,"lln: Can't open %s/%s\n",pathbuf,cd.d_name);
#endif
				continue;
			}
/*
	Maintain current path in pathbuf global area
*/
			for (pathend = pathbuf; *pathend; ++pathend) ;
			*pathend = '/';
#ifdef BSD_FILESYS
			strncpy(pathend + 1,cd->d_name,cd->d_namlen + 1);
			chdir(cd->d_name);
#else /* !bsd_filsys */
			strncpy(pathend + 1,cd.d_name,14);
			chdir(cd.d_name);
#endif
#ifdef DEBUG
			printf("lln: debug: visiting %s\n", pathbuf);
#endif /* debug */
			if (!lnsrch(newdfd))
				{
#ifdef BSD_FILESYS
#ifdef DEBUG
				printf("WILL CLOSEDIR\n");
#endif
				CLOSEDIR(newdfd);
#else
				close(newdfd);
#endif
				return 0;	/* recursively return 0 if all were found */
				}
#ifdef BSD_FILESYS
#ifdef DEBUG
			printf("WILL CLOSEDIR()!\n");
#endif
			CLOSEDIR(newdfd);
#else
			close(newdfd);
#endif
			chdir("..");
			*pathend = '\0';
			continue;
		}

#ifdef DEBUG
		else
			printf("debug: reg file: %s/%s\n", pathbuf,
#ifdef BSD_FILESYS
			cd->d_name);
#else
			cd.d_name);
#endif /* bsd */
#endif /* debug */

#ifdef BSD_FILESYS
		if (cd->d_ino == inn)	/* Print out path on match!! */
		{	printf("%s/%.14s\n",pathbuf,cd->d_name);
#else
		if (cd.d_ino == inn)	/* Print out path on match!! */
		{	printf("%s/%.14s\n",pathbuf,cd.d_name);
#endif
			if (++found == need) return 0;
		}
	}
	return -1;
}


#if defined(BSD_FILESYS) && defined(DEBUG)
/*
 * OPENDIR and CLOSEDIR are debug routines instead of the
 * regular OPENDIR(), CLOSEDIR() routines.  It is used
 * to help track down a problem that I think is "too many
 * files open".
 */

static	open_count = 0;		/* number of open files */
#define MAXFILES _NFILE+1	/* one extra for precaution */
static	char	*fname[MAXFILES];	/* file names, fname[0] is first file name */
static	DIR	*fptr[MAXFILES];	/* store DIR * ptrs in parellel w fname[] */

/*
 * the array fname[] is needed since we cannot derive the directory
 * name when we are closing it.
 */

DIR *
OPENDIR(name)
	char	*name;
{
	DIR	*dirp;
	char	*str_malloc();	/* fw ref */

	fname[open_count] = str_malloc(name);
	dirp = opendir(name);
	fptr[ open_count ] = dirp;
	++open_count;
	printf("OPENDIR(%s) %d\n", name, open_count);	/* number open *after* open */
	return (dirp);
}

void
CLOSEDIR( dirp )
	DIR	*dirp;
{
	int	i;
	if (open_count < 1 )	/* nothing open now */
		error("CLOSEDIR without OPENDIR");

	/* must check all slots, not just first open_count,
	 * because of "holes" in middle!!! */
	for (i = 0; i < MAXFILES; ++i)
		if (dirp == fptr[i])
			break;	/* found ptr */
	if (i >= MAXFILES)
		error("not found in table");
	--open_count;
	printf("CLOSEDIR(%s) %d\n", fname[i], open_count);
	free( fname[i]);
	return;
}

/*
 * malloc space for a string and copy it in
 * return ptr to malloc'd string
 */
char	*
str_malloc( s )
	char	*s;
{
	extern	char	*malloc();
	char	*cp;
	cp = malloc( strlen(s) + 1);
	if (!cp )
		error("out of space");
	strcpy( cp, s );
	return cp;
}

error(msg)
	char	*msg;
{
	fprintf(stderr, "%s\n", msg);
	exit (1);
}
#endif /* bsd && debug */

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
static	char	buffer[50];	/* don't loose it !!! */

	if (errno < 0 || errno >= sys_nerr)
		{
		sprintf( buffer, "errno %d undefined (%d=max)", errno, sys_nerr);
		return(buffer);
		}

	return( sys_errlist[errno] );
}
