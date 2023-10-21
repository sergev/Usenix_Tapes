/*
 * Find all executable files of the same name which
 * are in directories reference by the PATH variable:
 *
 * It is assumed that there are only a few executable files.
 * Findpaths could be sped up by changing the linear searches
 * through the data structures.
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>
#include <strings.h>

#define NDIRS 100		/* maximum # of directories in PATH  */
#define VOID void
#define FESIZE (sizeof (struct file_entry))
#define DESIZE (sizeof (struct dir_entry))

char *getenv();
VOID exit();
DIR *opendir();
struct direct * readdir();
char *strcat();
char *strcpy();
char *malloc();

struct dir_entry {
    char * where;		/* Pointer to directory name */
    struct dir_entry *next;	/* Pointer to other entries */
};

struct file_entry {
    struct file_entry *before;	/* Who comes before me */
    struct file_entry *after;	/* Who comes after me */
    char * name;		/* Who I am */
    struct dir_entry *firstdir;	/* Where I am */
} *file_table = NULL;

struct file_entry *last_entry = NULL;

/*
 * addentry adds an entry to the file table for the given file
 * pointing to the given directory.
 * A new file will cause an entry to be added to the file table
 * If a file is in the file table then the directory will be
 * added to the list of directories.
 * The file table is maintained in sorted order.
 * Since the directories are searched in PATH order, the first
 * directory entry is the one which will be found by execp
 */

void addentry(dir,file)
char *dir;
char *file;
{
    struct file_entry *ftp;	/* Search the table */
    struct file_entry *newftp;	/* Used for insertions */
    int cmpval;			/* value returned by strcmp */
    if (file_table == NULL) {	/* First call to addentry */
        file_table = (struct file_entry *)malloc(FESIZE);
	file_table->before = NULL;
	file_table->after = NULL;
	file_table->name = malloc(strlen(file)+1);
	strcpy(file_table->name,file);
	file_table->firstdir = (struct dir_entry *) malloc(DESIZE);
	file_table->firstdir->where = dir;
	file_table->firstdir->next = NULL;
	last_entry = file_table;
      } else {			/* Search the table for this file */
	for (ftp = file_table; ftp != NULL; ftp = ftp->after) {
	    cmpval = strcmp(ftp->name,file);
	    if (cmpval > 0) {	/* Have passed file, so not in table */
	        newftp = (struct file_entry *) malloc(FESIZE);
		newftp->before = ftp->before;
		newftp->after = ftp;
		if (ftp == file_table) {
		    file_table = newftp;
		} else {
		    (newftp->before)->after = newftp;
		}
		ftp->before = newftp;
		newftp->name = malloc(strlen(file)+1);
		strcpy(newftp->name,file);
		newftp->firstdir = (struct dir_entry *) malloc(DESIZE);
		newftp->firstdir->where = dir;
		newftp->firstdir->next = NULL;
		return;
	    }
	    if (cmpval == 0) {	/* Found a match, update the directory */
	        struct dir_entry *dp = ftp->firstdir;
		while (dp->next != NULL) dp = dp->next;
		dp->next = (struct dir_entry *) malloc(DESIZE);
		dp->next->where = dir;
		dp->next->next = NULL;
		return;
	    }
	}
	/* Fell off the end, so need a new entry */
	newftp = (struct file_entry *) malloc(FESIZE);
	newftp->before = last_entry;
	last_entry->after = newftp;
	last_entry = newftp;
	newftp->after = NULL;
	newftp->name = malloc(strlen(file)+1);
	strcpy(newftp->name,file);
	newftp->firstdir = (struct dir_entry *) malloc(DESIZE);
	newftp->firstdir->where = dir;
	newftp->firstdir->next = NULL;
    }
    return;
}

#define S_GEXEC (S_IEXEC >> 3)		/* Group execute bit */
#define S_WEXEC (S_IEXEC >> 6)		/* World exeucte bit */

/*
 * CanExec checks to see if the file can be executed by a user
 * with the given uid or gid
 */
int CanExec(uid,gid,filestat)
int uid;
int gid;
struct stat *filestat;
{
    if ((filestat->st_mode & S_IFMT) != S_IFREG)
        return(0);		/* Only regular files are executable */
    if (uid == 0) return(1);	/* Root can execute anything */
    if ((uid == filestat->st_uid)   /* If I'm owner and owner can execute */
        && ((filestat->st_mode & S_IEXEC) == S_IEXEC)) return(1);
    if ((gid == filestat->st_gid)   /* If I'm in group and group can */
	&& ((filestat->st_mode & S_GEXEC) == S_GEXEC)) return(1);
    if ((filestat->st_mode & S_WEXEC) == S_WEXEC) return(1); /* If world can */
    return(0);			/* you lose */
}

main(argc,argv)
int argc;
char *argv[];
{
    char filename[MAXNAMLEN];	/* path to file to examine */
    char *p;			/* pointer into path for directory checking */
    char *path;			/* pointer to PATH environment value */
    char *dirs[NDIRS];		/* array of pointers to individual paths */
    int i;			/* Good ole i */
    int myuid;			/* real uid of invoker */
    int mygid;			/* real gid of invoker */
    int ndirs;			/* number of directory entries + 1 */
    int verbose = 0;		/* True to print progress (-v option) */
    int printdir = 0;		/* True to print directories (-d option) */
    DIR *thisdir;		/* Current checkout directory */
    struct direct *entry;	/* File to examine */
    struct stat mystat;		/* stat for the file being examined */
    struct file_entry *ftp;	/* Index through filetable */

/*
 * Crack the arguments
 */
    for (i = 1; i < argc; i++) {
      if (argv[i][0] != '-') goto usage;
      for (p = &argv[i][1]; *p != '\0'; p++) {
	switch (*p) {
	case 'v': verbose++; break;
	case 'd': printdir++; break;
	default: goto usage;
	}
      }
    }

/*
 * Get information about the user for later reference
 */

    myuid = getuid();
    mygid = getgid();

    if ((path = getenv("PATH")) == NULL) {
	(void) fprintf(stderr,"No PATH environment variable.\n");
	exit(1);
	/*NOTREACHED*/
    }

/*
 * Convert the path variable into a collection of strings, with
 * one string per path directory
 */

    p = path;
    i = 0;
    dirs[i++]  = p;
    while (*p != '\0') {	/* look at each character in path */
	if (*p == ':') {	/* If it's a colon then */
	    *p++ = '\0';	/* make in a null */
	    dirs[i++] = p;	/* and set the next entry to point past it */
	    if (i >= NDIRS) {	/* check for overflow */
	        (void) fprintf(stderr,"More than %d directories.\n", i);
		exit(1);
		/*NOTREACHED*/
	    }
	} else {		/* Not a colon, so just skip */
	    p++;
	}
    }

/*
 * Go through the directories and find the executable files
 */

    ndirs = i;
    for (i = 0; i < ndirs; i++) {
      if (verbose)
	(void) fprintf(stdout,"processing %s\n", dirs[i]);
	if ((thisdir = opendir(dirs[i])) == NULL) {
	    (void) fprintf(stderr,"Directory %s:  ", dirs[i]);
	    perror("Can't open");
	} else {
	    for (entry = readdir(thisdir); entry != NULL;
	    	 entry = readdir(thisdir)) {
		(void) strcpy(filename,dirs[i]);
		(void) strcat(filename,"/");
		(void) strcat(filename,entry->d_name);
		if (stat(filename,&mystat) < 0) {
		    (void) fprintf(stderr,"%s:  ", filename);
		    perror("Can't stat");
		} else {
		    if (CanExec(myuid,mygid,&mystat)) {
			addentry(dirs[i],entry->d_name);
		    }
		}
	    }
	    closedir(thisdir);
	}
    }

/*
 * Now look through the table and find entries with duplicates
 */

    for (ftp = file_table; ftp != NULL; ftp = ftp->after) {
	if (ftp->firstdir->next != NULL) {
	    struct dir_entry * dp;
	    (void) fprintf(stdout,"%s:\n", ftp->name);
	    if (printdir)
	      for (dp = ftp->firstdir; dp != NULL; dp = dp->next) {
		(void) fprintf(stdout,"    %s\n", dp->where);
	      }
	}
    }
    exit(0);

  usage:
    (void) fprintf(stderr,"Usage:  %s [-v] [-d]", argv[0]);
    exit(1);
}
