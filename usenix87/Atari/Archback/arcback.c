
char cpyrid[] = "Copyright 1987 by Leif Samuelsson";

/*
 * ARCBACK.C - Back up ST disk using ARC
 *
 * This program may be copied and distributed free of charge.
 * It may be modified only if this header and copyright notice
 * is kept intact.
 */

/*
 * Version 1.0, 87-03-18
 *
 * Compile with Megamax C and install as ARCBACK.TTP
 */


/* Documentation *

This program is an attempt at making a backup utility which uses
as few floppies as possible.

ARCBACK employs the use of the separate program ARC to create and
update archives of your hard disk. Since ARC can not handle directory
structures, ARCBACK will create one archive per directory and keep a
catalog of all archives on the first floppy, showing which directory is
saved in which archive and on which floppy.

ARCBACK has one serious limitation and a few other drawbacks, please
read on.

PROS:
	- Uses less floppies than other backup programs.

	- All hard disks and partitions are backed up on one common
	  set of floppies. (This is not enforced - you are free to keep
	  a set per partition if you like).

	- Incremental backups are handled automatically by ARC.

	- Backs up a directory (and subdirectories) or a full partition
	  at a time.

	- Allows exclusion of certain subdirectories from backup. Useful
	  for avoiding backup of directories containing temporary data.

	- Easy to use.

CONS:
	- IMPORTANT: The program will fail if the compressed archive of
	  a directory does not fit on an empty floppy. ARCBACK can not
	  split directories! (But subdirectories are counted separately).
	  This limits the size of a directory to about 1/2 Mbyte
	  if using single-sided floppies and 1 Mbyte if they are
	  double sided.

	- Very slow. This can probably be helped with a very large
	  RAM-disk, but I think it would have to be at least twice as
	  big as a floppy due to the way that ARC uses temporary files.

	- Requires user to swap floppies frequently. This is done to
	  optimize disk usage.


USING ARCBACK
=============

Caveat: Because ARCBACK is a new, experimental utility I have the
following warning.

Before using ARCBACK for the first time, it might wise to make a full
backup of your hard disk using a reliable backup program. I do not
guarantee that ARCBACK does not trash you hard disk!

To start using ARCBACK, get a stack of empty, formatted floppies,
enough to hold all the data you want to back up. Make sure you have
enough, there is no way of pausing the backup to format new floppies.
Don't calculate on compression, instead leave yourself a decent margin.

Label the floppies carefully with numbers starting from 1. The program
will ask you to swap disks back and forth with very little room for
errors. (It does not check whether you have inserted the right disk!).

Put an empty file called NOBACKUP in every directory that you do not
want backed up. This excludes the files in that directory, but not it's
subdirectories.

Run ARCBACK.TTP with an argument that is either the name of a partition
like:

	ARCBACK.TTP C

or the full path of a directory, like:

	ARCBACK.TTP C:\SRC\GAMES

Run as many times as you like, sometimes with whole partitions,
sometimes with just a directory. ARC will only back up files that
have changed since the last time.

Make sure that you have a copy of ARC on a floppy. You need it if you
want to restore files from the archives.


THE ARCHIVES
============

It is ok to manually maintain the archives using ARC. You can add,
update and delete files from an archive and you can even delete an
archive.

Be very careful, however, not to edit or remove the ARCDIR..TXT catalog
file unless you have to. This file has four fields: Name of directory,
floppy volume number, name of archive and date of last modification.
The third field is for user reference only and is NOT read by the
program!  Instead, the program uses the line number (starting from zero)
to build the name of the archive.

Say for example, that you want to delete the archive for the directory
C:\OLDJUNK (either because you have deleted this directory or because
you don't want to keep a backup of it). In the file ARCDIR.TXT on the
first floppy, you will find the line:

C:\OLDJUNK	2	A15	1234567890

Now, delete the file A15.ARC on floppy 2, but do not delete the line
in ARCDIR.TXT! Instead, change it to something like:

X:\DELETED	0	A15	0


If ARCBACK should crash or be otherwise interrupted, an up-to-date copy
of ARCDIR.TXT can be found in the directory tempdir. You should copy
this to floppy 1.

Please send bug fixes, enhancements, suggestions, praise, flames etc to
me.

leif@sun.COM
leif@sun.UUCP
leif on bix

* * * */





#include <stdio.h>
#include <osbind.h>

#define MAXVOLS 15
#define MAXARCS 128





/*
 * Path for ARC
 */
char arcprog[] = "C:\\BIN\\ARC.TTP";

/*
 * Directory for the temporary ARC file. There must be at least
 * twice as much space available here as will fit on a floppy.
 * Using a RAM-disk is OK, if it meets this requirement.
 * This directory will be excluded from backup.
 */
char tempdir[] = "C:\\TMP";


/*
 * File to contain the catalog of archives. Never edit this
 * file unless you know what you are doing.
 */
char arcdir[] = "ARCDIR.TXT";



#define FALSE 0
#define TRUE 1


struct DTA {			/* Used in directory lookups */
	char sys[20];
	unsigned char attr;
	unsigned int time, date;
	unsigned long size;
	char name[14];
};


char disk = ' ';		/* Disk to back up.		*/
				/* You can set a default here.	*/

int diskno = 0;			/* Used to keep track of which	*/
				/* volume is in drive A.	*/

struct arc {			/* Table of archives, from file */
	char path[80];		/* arcdir[] */
	int diskno;
	unsigned long time;
} arcs[MAXARCS];

int n_arcs;			/* Number of archives */

unsigned long
	floppy_free[MAXVOLS],	/* Table of disk volumes */
	floppy_total;		/* Max storage on floppy */

char temparc[80], temp_arcdir[80], real_arcdir[80];

/*
 * diskspace - a function to return the remaining space of a disk.
 */
unsigned long diskspace(drive, flag)
int drive, flag;
{
struct disk_info { long b_free, b_total, b_secsiz, b_clsiz; } dip;

    Dfree(&dip, drive-'@');
    return((flag ? dip.b_total : dip.b_free) * dip.b_secsiz * dip.b_clsiz);
}


/*
 * main
 */
main(argc, argv)
int argc;
char *argv[];
{
register int i;
FILE *f;
char buf[15];
struct DTA *dtap;

    /* Check if ARC exists */
    dtap = (struct DTA *) malloc(sizeof(struct DTA));
    Fsetdta(dtap);
    if (Fsfirst(arcprog,0) < 0) {
	printf("Can't find %s\n", arcprog);
	prompt("Hit RETURN>");
	exit(1);
    }

    /* Parse arguments */
    if (argc == 2 && (!argv[1][1] || (argv[1][1] == ':' && argv[1][2] =='\\')))
	disk = argv[1][0];
    if (disk < 'B' || disk > 'Z') {
	printf("Usage: ARCBACK drive\n");
	printf("    or ARCBACK path\n");
	prompt("Hit RETURN>");
	exit(1);
    }

    /* Initialize tables and file names */
    for (i=0; i < MAXVOLS; i++)
	floppy_free[i] = -1;
    for (i=0; i < MAXARCS; i++)
	arcs[i].path[0] = '\0';
    sprintf(temparc, "%s\\TEMPARC.ARC", tempdir);
    sprintf(real_arcdir, "A:\\%s", arcdir);
    sprintf(temp_arcdir, "%s\\%s", tempdir, arcdir);
    Dcreate(tempdir);
    sprintf(buf, "%s\\NOBACKUP", tempdir);
    if ((i = Fcreate(buf, 0)) >= 0)
	Fclose(i);

    /* Read in catalog */
    swapdisk(1);
    floppy_total = diskspace('A', TRUE);
    n_arcs = 0;
    f = fopen(real_arcdir,"r");
    if (f != (FILE *) 0) { 
	while (fscanf(f,"%s %d %s %ld\n",
		arcs[n_arcs].path, &arcs[n_arcs].diskno, buf, &arcs[n_arcs].time) != EOF) {
#ifdef DEBUG
		printf("<%s> %d %ld\n", arcs[n_arcs].path, arcs[n_arcs].diskno,
					arcs[n_arcs].time);
#endif

	    n_arcs++;
	}
	fclose(f);
    }
    save_arcdir();

    /* Back up! */
    Dsetdrv(disk - 'A');
    if (argv[1][1] == ':' && argv[1][2] == '\\')
	backdir(argv[1]+2);
    else
	backdir("\\");

    /* Write back catalog */
    swapdisk(1);
    save_arcdir();
    copyfile(temp_arcdir, real_arcdir);

    /* Wait for user. You can comment this out if you are
     * always using a command shell.
     */
    prompt("Done. Hit return> ");
}



/*
 * filesize - a function to return the size of a file.
 */
unsigned long filesize(path)
char *path;
{
struct DTA *dtap;

    Fsfirst(path, 0);
    dtap = (struct dta *) Fgetdta();
    if (dtap)
	return(dtap->size);
    else
	return(0L);
}

/*
 * filetime - a function to return the last modification time of a file.
 */
unsigned long filetime(path)
char *path;
{
struct DTA *dtap;

    Fsfirst(path, 0);
    dtap = (struct dta *) Fgetdta();
    if (dtap)
	return((((unsigned long)(dtap->date))<<16) + dtap->time);
    else
	return(0L);
}

/*
 * backdir - does all the work.
 * Recursive routine to back up a directory.
 */
backdir(path)
char *path;
{
register int i;
int attr;

char	buf[80];
struct DTA *dtap;

    Dsetpath(path);
    dtap = (struct DTA *) malloc(sizeof(struct DTA));
    Fsetdta(dtap);

    /* Check if the file NOBACKUP exists */
    if (path[1])
	sprintf(buf, "%s\\NOBACKUP", path);
    else
	sprintf(buf, "%sNOBACKUP", path);
    i = Fsfirst(buf, 0);
    if (i < 0)			/* Did not exist, OK to backup */
	backup_files(path);
    else
	printf("Skipping %c:%s\n",disk,path);

    /* recursively back up all sub-directories */
    if (path[1])
	sprintf(buf, "%s\\*.*", path);
    else
	sprintf(buf, "%s*.*", path);
    i = Fsfirst(buf, 1<<4);		/* Files and Directories */

    while (i >= 0) {
	if (strcmp(dtap->name, ".") && strcmp(dtap->name, "..")) {
	    if (!path[1])
		sprintf(buf,"%s%s", path, dtap->name);
	    else
		sprintf(buf,"%s\\%s", path, dtap->name);
	    attr = Fattrib(buf, 0, 0);
	    if (attr & (1<<4))		/* Directory? */
		backdir(buf);
	    Dsetpath(path);
	}
	Fsetdta(dtap);
	i = Fsnext();
    }
    free(dtap);
}


/*
 * backup_files - calls ARC to update archive for given directory.
 */
backup_files(path)
char *path;
{
register int arcno, d, j, n;
unsigned long l,time,size;
char	buf[80],
	cpath[80],
	arcpath[80];
struct DTA *dtap;

    printf("%c:%s\n",disk,path);
    dtap = (struct DTA *) Fgetdta();

    /* Find out if archive already exists */
    sprintf(cpath, "%c:%s", disk, path);
    arcno = 0;
    while (arcno < n_arcs && strcmp(cpath, arcs[arcno].path))
	arcno++;
    sprintf(arcpath, "A:\\A%d.ARC", arcno);
    Fdelete(temparc);
    if (arcno == n_arcs) {		/* New archive */
	strcpy(arcs[arcno].path, cpath);
	n_arcs++;
    }
    else {			/* Old archive, copy to tempdir */
	if (arcs[arcno].diskno > 0) {
	    /* Find out if any files are younger than archive */
	    time = 0;
	    j = Fsfirst("*.*", 0);
	    while (j >= 0) {
		l = (((unsigned long)(dtap->date))<<16) + dtap->time;
		if (l > time)
		    time = l;
		j = Fsnext();
	    }
	    if (arcs[arcno].time == 0L) {
		swapdisk(arcs[arcno].diskno);
		arcs[arcno].time = filetime(arcpath);
	    }
	    if (time <= arcs[arcno].time)
		return;		/* Don't back up  */
	    swapdisk(arcs[arcno].diskno);
	    copyfile(arcpath, temparc);
	}
    }

    /* Call ARC */
    sprintf(buf, " u %s", temparc);
    buf[0] = strlen(buf)-1;
    Pexec(0, arcprog, buf, 0L);


    /* Copy archive to the first volume with enough space. */
    size = filesize(temparc);
#ifdef DEBUG
    printf("arcsize = %ld\n", size);
#endif
    if (size > floppy_total) {
	/* Archive is too big. (This part is not tested yet) */
	printf("Archive too big (%ld bytes). Skipping.\n", size);
	Fdelete(temparc);
	return;
    }
    Fdelete(arcpath);
    floppy_free[diskno] = diskspace('A', FALSE);
    d = 0;
    j = 1;
    while (d < 15 && !d) {
	if (floppy_free[j] == -1)
	    swapdisk(j);
#ifdef DEBUG
	printf("diskspace = %ld\n", floppy_free[j]); fflush(stdout); */
#endif
	/* Check if it fits, but leave some room on vol 1 */
	if (floppy_free[j] - ((j==1) ? 10*1024 : 0) > size)
	    d = j;
	j++;
    }
    if (!d) {
	printf("Out of disks\n");
	swapdisk(1);
	copyfile(temp_arcdir,real_arcdir);
	exit(1);
    }
    swapdisk(d);

    arcs[arcno].diskno = d;
    arcs[arcno].time = filetime(temparc);
    copyfile(temparc,arcpath);
    floppy_free[diskno] = diskspace('A', FALSE);
    save_arcdir();
}

prompt(str)
char *str;
{
    printf(str);
    fflush(stdout);
    while (getchar() != '\n')
	;
}

swapdisk(d)
{
char	buf[80];

    if (diskno != d) {
	sprintf(buf, "Insert disk %d and hit RETURN>", d);
	prompt(buf);
	diskno = d;
	floppy_free[d] = diskspace('A', FALSE);
    }
}

copyfile(from, to)
char *from,*to;
{
char buf[512];
register char *p;
register int ffrom, fto;
register long count;

    if((ffrom = Fopen(from, 0)) < 0) {
	printf("Can't open %s\n", from);
	return;
    }
    if((fto = Fcreate(to, 0)) < 0) {
	/* May already exist */
	if((fto = Fopen(to, 1)) < 0) {
		printf("Can't create %s\n", to);
		Fclose(ffrom);
		return;
	}
    }
    while((count = Fread(ffrom, 512L, buf)) > 0)
    {
	if(Fwrite(fto, count, buf) != count)
	{
	    printf("Error writing %s", to);
	    Fclose(ffrom);
	    Fclose(fto);
	    return;
	}
    }
    Fclose(ffrom);
    Fclose(fto);
    return;
}

save_arcdir()
{
FILE *f;
register int i;

    f = fopen(temp_arcdir,"w");
    if (f != (FILE *) 0) { 
	for (i=0; i<n_arcs; i++) {
	    fprintf(f,"%s\t", arcs[i].path);
	    if (strlen(arcs[i].path) < 16)
		putc('\t',f);
	    if (strlen(arcs[i].path) < 8)
		putc('\t',f);
	    fprintf(f, "%2d\tA%d\t%ld\n", arcs[i].diskno, i, arcs[i].time);
	}
	fclose(f);
    }
}
