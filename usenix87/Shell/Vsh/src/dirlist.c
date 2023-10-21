#include "hd.h"
#include "mydir.h"
#include "command.h"
#include <pwd.h>

#define SHIFT1	13
#define SHIFT2	6
#define SHIFT3	3
#define MASK	07

char *pfstatus (fname) char * fname;
{

/* long listing of a file */
    static char *mchar [2] [8] = 
    {
	    "---", "--x", "-w-", "-wx",
	    "r--", "r-x", "rw-", "rwx",
	    "--s", "--s", "-ws", "-ws",
	    "r-s", "r-s", "rws", "rws"
    } ;

    static char fformat [8] = 
    {
#ifdef	VENIX
	'0', '1', '2', '3', ' ', 'c', 'd', 'b'
#else
#ifdef	V6
	'0', '1', '2', '3', ' ', 'c', 'd', 'b'
#else
	' ', 'c', 'd', 'b', '-', 'l', 's', '?'
#endif
#endif
    } ;

    register mode;
    struct passwd *getpwuid();
    static struct passwd *p;
#ifdef	SYMLINK
    static char slink[STRMAX];
    int sflag;

    sflag = 0;
    if (lstat (fname, &scr_stb)) 
#else
    if (stat (fname, &scr_stb)) 
#endif
    {
	printf ("Cannot Access");
	return 0;
    }
    mode = scr_stb.st_mode;

#ifdef	SYMLINK
	if ((mode&S_IFLNK) == S_IFLNK) {
		mode = readlink(fname, slink, STRMAX-1);	
		if (mode == -1) {
			printf ("Cannot read link");
			return 0;
		}
		slink[mode] = 0;
		sflag++;
		if (stat (fname, &scr_stb)) {
			printf ("Cannot Access");
			return 0;
		}
		mode = scr_stb.st_mode;
	}
#endif

    printf ("%c%s%s%s%s%2d",
	(mode & S_ISVTX) ? 't' : 
	fformat [(mode >> SHIFT1) & MASK],
	mchar [(mode & S_ISUID) != 0] [(mode >> SHIFT2) & MASK],
	mchar [(mode & S_ISGID) != 0] [(mode >> SHIFT3) & MASK],
	mchar [0] 		      [mode & MASK],
	scr_stb.st_nlink > 99 ? "" : " ",
	scr_stb.st_nlink);
	/* dyt */
    if (p == 0 || p->pw_uid != scr_stb.st_uid)
	p = getpwuid(scr_stb.st_uid);
    if (p)
	printf(" %8s", p->pw_name);
    else
	printf(" %8d", scr_stb.st_uid);
    mode &= S_IFMT;
    if (mode == S_IFCHR || mode == S_IFBLK)
	printf ("%5d, %3d",
#ifdef	V6
	major (scr_stb.st_addr[0]), minor (scr_stb.st_addr[0]));
#else
	major (scr_stb.st_rdev), minor (scr_stb.st_rdev));
#endif
    else
#ifdef	V6
	printf ("%10ld", (((long)scr_stb.st_size0&0377)<<16L)
		+ (unsigned)scr_stb.st_size1);
#else
	printf ("%10ld", scr_stb.st_size);
#endif
    printf (" %.24s", ctime (&scr_stb.st_mtime));
#ifdef	NDIR
    return (sflag ? slink : 0);
#endif
}

/* This prints out the protection modes of the files on the current
   page.  It knows the page from the global variable cpage.  The
   value NOREPLOT is always returned, to make globalcmd happy.
*/
int didlong;

longlist () 
{		/* long listing for entire page */
    register i, j;
	
    bufout ();
    j = pgend();
    for (i=0; i < j; i++) longfile (i);
    didlong = j;
    unbufout ();

    return CMD_DATE;	/* Run date command */
}

longfile (index) int index;
{
    register char *s;
    register int i;
    int putch();

    if (pageoff) {
	cpage += pageoff;
	pageoff = 0;
	i = column;
	column = 1;	/* Display only current page */
	dispdir(0);
	column = i;
    }
    atfile (index, 23);
    if (CE)
	tputs(CE, 0, putch);
    s = pfstatus (filename (index));
#ifdef	SYMLINK
    if (s) {
	if (xoff < 10) {
		atfile (index, OFFFILE-2);
		printf(" @");
	}
	printf("%-18.18s", s);
    }
#endif
}

/* Warning: some systems have macros which define these in <sys/types.h> */
#ifdef	 MACMAJOR
major(i)
dev_t i;
{
    return (i>>8);
}

minor(i)
dev_t i;
{
    return (i&0377);
}
#endif
