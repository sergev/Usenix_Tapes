/* agef		02 March 1987
  
   David S. Hayes, Site Manager
   Army Artificial Intelligence Center
   Pentagon HQDA (DACS-DMA)
   Washington, DC  20310-0200
  
   Phone:  202-694-6900
   Email:  merlin@hqda-ai.UUCP   dshayes@brl.ARPA
  
   +=======================================+
   | This program is in the public domain. |
   +=======================================+
  
   This program scans determines the amount of disk space taken up
   by files in a named directory.  The space is broken down
   according to the age of the files.  The typical use for this
   program is to determine what the aging breakdown of news
   articles is, so that appropriate expiration times can be
   established.
  
   Call via
  
	agef fn1 fn2 fn3 ...
  
   If any of the given filenames is a directory (the usual case),
   agef will recursively descend into it, and the output line will
   reflect the accumulated totals for all files in the directory.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>


char           *program;	/* our name */

#define SECS_DAY (24L * 60L * 60L)	/* seconds in one day */

#define ceiling(x,y) ((x+y-1)/y)

#define AGES 6			/* how many age categories */
#define TOTAL (AGES-1)		/* column number of total column */
#define SAME 0			/* for strcmp */

char           *header[] = {	/* column headers */
			    "7 days",
			    "14 days",
			    "30 days",
			    "45 days",
			    "46+ days",
			    "Total"};

int             ages[] = {	/* age categories */
			  7,
			  14,
			  30,
			  45,
			  999999,
			  999999};

int             inodes[AGES],	/* inode count */
                blocks[AGES];	/* block count */

char            topdir[MAXPATHLEN];	/* our starting directory */
long            today,
                time();		/* today's date */


main(argc, argv)
    int             argc;
    char           *argv[];
{
    int             i;
    int             summary;
    int             total_inodes[AGES],	/* for grand totals */
                    total_blocks[AGES];

    program = *argv++;		/* save our name for error messages */
    argc--;

    summary = argc > 1;		/* should we do a grant total? */

    getwd(topdir);		/* find out where we are */
    today = time(0) / SECS_DAY;

    /* print our column headers */
    for (i = 0; i < TOTAL; i++)
	printf("%10s ", header[i]);
    printf("%12s  Name\n", header[TOTAL]);
    for (i = 0; i < TOTAL; i++)
	printf("---------- ");
    printf("------------  ----\n");

    for (i = 0; i < AGES; i++)
	total_inodes[i] = total_blocks[i] = 0;

    /* now do some work */
    for (; argc; argv++, argc--) {
	for (i = 0; i < AGES; i++)
	    inodes[i] = blocks[i] = 0;

	chdir(topdir);		/* be sure to start from the same place */
	get_data(*argv);	/* this may change our cwd */

	display(*argv, inodes, blocks);
	for (i = 0; i < AGES; i++) {
	    total_inodes[i] += inodes[i];
	    total_blocks[i] += blocks[i];
	};
    };

    if (summary) {
	putchar('\n');
	display("Grand Total", total_inodes, total_blocks);
    };
};


 /*
  * Get the aged data on a file whose name is given.  If the file is a
  * directory, go down into it, and get the data from all files inside. 
  */
get_data(path)
    char           *path;
{
    struct stat     stb;
    int             i;
    long            date,	/* date of modification */
                    age;	/* file age in days */

    stat(path, &stb);
    if ((stb.st_mode & S_IFMT) == S_IFDIR)
	down(path);
    if ((stb.st_mode & S_IFMT) == S_IFREG) {
	date = stb.st_mtime / SECS_DAY;
	age = today - date;

	for (i = 0; i < TOTAL; i++) {
	    if (age <= ages[i]) {
		inodes[i]++;
		blocks[i] += roundup(stb.st_size, 1024);
		break;
	    };
	};
	inodes[TOTAL]++;
	blocks[TOTAL] += roundup(stb.st_size, 1024);
    };
}


 /*
  * We ran into a subdirectory.  Go down into it, and read everything
  * in there. 
  */

down(subdir)
    char           *subdir;
{
    DIR            *dp;
    char            cwd[MAXPATHLEN];
    struct direct  *file;

    if (strcmp(subdir, ".") == SAME || strcmp(subdir, "..") == SAME)
	return;

    if ((dp = opendir(subdir)) == NULL) {
	fprintf(stderr, "%s: can't read %s/%s\n", program, topdir, subdir);
	return;
    };

    getwd(cwd);			/* remember where we are */
    chdir(subdir);		/* go into subdir */
    for (file = readdir(dp); file != NULL; file = readdir(dp))
	get_data(file->d_name);

    chdir(cwd);			/* go back where we were */
    closedir(dp);		/* shut down the directory */
}



display(name, inodes, blocks)
    char           *name;
    int             inodes[],
                    blocks[];
{
    char            tmpstr[30];
    int             i;

    for (i = 0; i < TOTAL; i++) {
	tmpstr[0] = '\0';
	if (inodes[i])
	    sprintf(tmpstr, "%d %4dk", inodes[i], blocks[i]);
	printf("%10s ", tmpstr);
    };
    sprintf(tmpstr, "%d %5dk", inodes[TOTAL], blocks[TOTAL]);
    printf("%12s  %-s\n", tmpstr, name);
}
