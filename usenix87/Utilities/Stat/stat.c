/*
 *	Stat.c		Dump out inode info in nice form.
 *			pur-ee!pucc-j!rsk, rsk@purdue-asc.arpa
 *			Original version by Hoch in the dark days of v6;
 *			this one by Rsk for 4.2 BSD.
 */

#include	<stdio.h>
#include	<time.h>
#include	<sys/types.h>
#include	<ctype.h>
#include	<sys/stat.h>
#include	<pwd.h>
#include	<grp.h>

#define	FAIL	-1		/* Failure return code from call */	
#define OKAY	0		/* Success return code from call */

struct 	stat	Sbuf;		/* for return values from stat() call */
char	*ctime();		/* Time conversion	*/
struct	passwd	*getpwuid();	/* User entry */
struct	passwd	*pwent;		/* User structure */
struct	group	*getgrgid();	/* Group entry */
struct	group	*grent;		/* Group structure */

char	Mode[10];	/* File protection mode */

#define	LBUFSIZ	256	/* Length for symbolic link translation buffer */
char	Lbuf[LBUFSIZ];	/* Symbolic link translation buffer */

main(argc, argv)
int argc;
char *argv[];
{
	int i;

	i = 1;
	if(argc == 1) {
		fprintf(stderr,"Usage: stat file1 [file2 ...]\n");
		exit(1);
	}

	do {
		stat_it(argv[i]);
		if(  (argc > 1) && (i < (argc-1)) )
			printf("\n");
	}
	while(++i < argc);

	exit(0);
}

/*
 * stat_it() - Actually stat and format results from file.
 *		exit -    OKAY if no problems encountered
 *			  FAIL if couldn't open or other nastiness
 *
 */
stat_it(filename)
char	*filename;
{
	int	count;

	if( lstat(filename,&Sbuf) == FAIL) {
		fprintf(stderr,"Can't lstat %s\n",filename); 
		return(FAIL);
	}

	if( (Sbuf.st_mode & S_IFMT) == S_IFLNK) {
		if( (count=readlink(filename,Lbuf,LBUFSIZ)) == FAIL) {
			fprintf(stderr,"Can't readlink %s\n", filename);
			return(FAIL);
		}
		if( count < LBUFSIZ)
			Lbuf[count] = '\0';
		printf("  File: \"%s\" -> \"%s\"\n",filename,Lbuf);
	}
	else
		printf("  File: \"%s\"\n", filename);

	printf("  Size: %-10d", Sbuf.st_size);
 	printf("   Allocated Blocks: %-10ld", Sbuf.st_blocks);

	printf("   Filetype: ");
	switch( Sbuf.st_mode & S_IFMT) {
		case	S_IFDIR:	printf("Directory\n");
					break;
		case	S_IFCHR:	printf("Character Device\n");
					break;
		case	S_IFBLK:	printf("Block Device\n");
					break;
		case	S_IFREG:	printf("Regular File\n");
					break;
		case	S_IFLNK:	printf("Symbolic Link\n");
					break;
		case	S_IFSOCK:	printf("Socket\n");
					break;
		default		:	printf("Unknown\n");
	}

	strcpy(Mode,"----------");
	if(Sbuf.st_mode & 0000001)	/* Other execute */
		Mode[9] = 'x';
	if(Sbuf.st_mode & 0000002)	/* Other write */
		Mode[8] = 'w';
	if(Sbuf.st_mode & 0000004)	/* Other read */
		Mode[7] = 'r';
	if(Sbuf.st_mode & 0000010)	/* Group execute */
		Mode[6] = 'x';
	if(Sbuf.st_mode & 0000020)	/* Group write */
		Mode[5] = 'w';
	if(Sbuf.st_mode & 0000040)	/* Group read */
		Mode[4] = 'r';
	if(Sbuf.st_mode & 0000100)	/* User execute */
		Mode[3] = 'x';
	if(Sbuf.st_mode & 0000200)	/* User write */
		Mode[2] = 'w';
	if(Sbuf.st_mode & 0000400)	/* User read */
		Mode[1] = 'r';
	if(Sbuf.st_mode & 0001000)	/* Sticky bit */
		Mode[9] = 't';
	if(Sbuf.st_mode & 0002000)	/* Set group id */
		Mode[6] = 's';
	if(Sbuf.st_mode & 0004000)	/* Set user id */
		Mode[4] = 's';
	switch( Sbuf.st_mode & S_IFMT) {
		case	S_IFDIR:	Mode[0] = 'd';
					break;
		case	S_IFCHR:	Mode[0] = 'c';
					break;
		case	S_IFBLK:	Mode[0] = 'b';
					break;
		case	S_IFREG:	Mode[0] = '-';
					break;
		case	S_IFLNK:	Mode[0] = 'l';
					break;
		case	S_IFSOCK:	Mode[0] = 's';
					break;
		default		:	Mode[0] = '?';
	}
	printf("  Mode: (%04o/%s)", Sbuf.st_mode&07777,Mode);


	(void) setpwent();
	if( (pwent = getpwuid(Sbuf.st_uid)) == NULL) {
		fprintf(stderr,"getpwuid() failed\n");
		exit(1);
	}
	printf("         Uid: (%5d/%8s)", Sbuf.st_uid, pwent->pw_name);

	(void) setgrent();
	if( (grent = getgrgid(Sbuf.st_gid)) == NULL) {
		fprintf(stderr,"getgrgid() failed\n");
		exit(1);
	}
	printf("  Gid: (%5d/%8s)\n", Sbuf.st_gid, grent->gr_name);


	printf("Device: %-5d", Sbuf.st_dev);
	printf("  Inode: %-10d", Sbuf.st_ino);
	printf("Links: %-5d", Sbuf.st_nlink);

	/* Only meaningful if file is device */

	if(  ( (Sbuf.st_mode & S_IFMT) == S_IFCHR)
		|| ( (Sbuf.st_mode & S_IFMT) == S_IFBLK) )
		printf("     Device type: %d\n",Sbuf.st_rdev);
	else
		printf("\n");

	/* The %.24s strips the newline from the ctime() string */

#ifdef SINCE
	printf("Access: %.24s",ctime(&Sbuf.st_atime));
	tsince(Sbuf.st_atime);
	printf("Modify: %.24s",ctime(&Sbuf.st_mtime));
	tsince(Sbuf.st_mtime);
	printf("Change: %.24s",ctime(&Sbuf.st_ctime));
	tsince(Sbuf.st_ctime);
#else SINCE
	printf("Access: %s",ctime(&Sbuf.st_atime));
	printf("Modify: %s",ctime(&Sbuf.st_mtime));
	printf("Change: %s",ctime(&Sbuf.st_ctime));
#endif SINCE

/*
* 	Should I put this in somewhere?  No.
*
* 	printf("Optimal Blocksize: %ld\n", Sbuf.st_blksize);
*/

	return(OKAY);
}

#ifdef SINCE
tsince(time_sec)
long	time_sec;
{
	long	time_buf;
	long	d_since;	/* days elapsed since time */
	long	h_since;	/* hours elapsed since time */
	long	m_since;	/* minutes elapsed since time */
	long	s_since;	/* seconds elapsed since time */

	time(&time_buf);

	if(time_sec > time_buf) {
		fprintf(stderr,"Time going backwards\n");
		exit(1);
	}

	s_since = time_buf - time_sec;
	d_since = s_since / 86400l ;
	s_since -= d_since * 86400l ;
	h_since = s_since / 3600l ;
	s_since -= h_since * 3600l ;
	m_since = s_since / 60l ;
	s_since -= m_since * 60l ;

	printf("(%05ld.%02ld:%02ld:%02ld)\n",d_since,h_since,m_since,s_since);

	return(OKAY);
}
#endif SINCE
