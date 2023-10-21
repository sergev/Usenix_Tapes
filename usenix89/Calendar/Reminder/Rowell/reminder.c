
/*
 * This program basically just echos the command line into a file
 * called .egg in your home dir. It is to leave yourself messages.
 * 'egg' by itself cats the file $HOME/.egg
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0
#define HOMESIZE 256

struct stat statbuf;
char *progname;

main (argc, argv)
int argc;
char *argv[];
{
    FILE   *fp, *efopen ();
    char   *home, *getenv(), eggfile[HOMESIZE + 6];
    char   *ctime ();
    char   *timezone ();
    long   time ();
    struct tm *localtime();
    struct timeval tv;
    struct timezone tz;
    long   clock;
    char   date[26];
    int    c;

    progname = argv[0];

    if (!(home = getenv("HOME"))) {
	fprintf (stderr, "%s: HOME: environment variable not set\n", progname);
	exit (2);
    } else if (strlen(home) > HOMESIZE) {
	fprintf (stderr, "%s: HOME: environment variable too long\n", progname);
	exit (3);
    }
    strcpy (eggfile, home);
    strcat (eggfile, "/.egg");

    if (argc == 1) {
	fp = efopen(eggfile, "r");
	if (fstat(fileno(fp), &statbuf) != 0) {
	    perror(progname);
	    exit (4);
	}
	if ((statbuf.st_mode & S_IFMT)==S_IFDIR) {
	    (void) fprintf(stderr,
	    "%s: %s is a directory.\n", progname, eggfile);
	    exit (5);
	}
	while ((c = getc (fp)) != EOF)
		putchar (c);
    } else {
	fp = efopen(eggfile, "a");
	if (fstat(fileno(fp), &statbuf) != 0) {
	    perror(progname);
	    exit (6);
	}
	if ((statbuf.st_mode & S_IFMT)==S_IFDIR) {
	    (void) fprintf(stderr,
	    "%s: %s is a directory.\n", progname, eggfile);
	    exit (7);
	}
	time (&clock);
	gettimeofday (&tv, &tz);

	tv.tv_sec += tz.tz_minuteswest*60L;
	strcpy (date, ctime (&clock));

	date[10] = date[13] = date[19] = '\0';

	fprintf (fp, "%s:", date);
	for (;argc>1; argc--) {
	    fprintf (fp, " %s", *(++argv));
	}
	fprintf (fp, "\n");
    }
}

/*
 * fopen file, die if can't.
 */
FILE *efopen (file, mode)
char *file, *mode;
{
    FILE *fp, *fopen();
    char s[2*HOMESIZE+9];

    if ((fp = fopen (file, mode)) != NULL)
	return (fp);
    strcpy (s, progname);
    strcat (s, ": ");
    strcat (s, file);
    perror (s);
    exit (1);
}



