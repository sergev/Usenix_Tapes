#ifndef lint
char sccsid[] = "@(#) uuwhere.c 1.12 85/11/29";
#endif
/*
 * uuwhere:
 * This is a frill which may be called by uuhosts to annotate routing
 * information produced by pathalias(1L) with the location of each host.
 * It takes the ASCII UUCP routing database PATHS on standard input,
 * and produces the annnotated version on standard output, to be put
 * in WHERE.  It gets the location information from the UUCP map directory
 * produced by uuhosts from the map posted to the USENET newsgroup
 * mod.map.uucp by the UUCP Project.
 *
 * Uuwhere is usually called by uuhosts, and most of the time just
 * checks to see if WHERE needs to be updated.  Uuhosts itself
 * displays the annotated routing information to the user.
 *
 * The host location information is first put into a dbm(3x)
 * database so that generating the final output will not take forever.
 * If you don't have dbm, this won't work.
 */

#include <stdio.h>
#include <ctype.h>
#include <dbm.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef UUCPINDEX
#define UUCPINDEX "/usr/local/lib/news/maps/mod.map.uucp/Index"
#endif
#ifndef TMP
#define TMP "/usr/local/lib/nmail.tmp"
#endif
#ifndef PATHS
#define PATHS "/usr/local/lib/nmail.paths"
#endif
#ifndef WHERE
#define WHERE "/usr/local/lib/nmail.where"
#endif
#ifndef DATABASE
#define DATABASE	"/usr/local/lib/uuindex"
#endif

static int verbose, flagcreat, flagupdate, flaginput;

main(argc, argv)
int argc;
register char **argv;
{
	register FILE *fin;
	flagcreat = 0;
	flagupdate = 1;
	flaginput = 0;

	(void) umask(0);
	while (*++argv != NULL) {
		if (**argv != '-') {
			flaginput = 1;
			flagupdate = 0;
		} else
		switch (argv[0][1]) {
			case '\0':
				flaginput = 1;
				flagupdate = 0;
				break;
			case 'c':
				flagcreat = 1;
				flagupdate = 0;
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				usage();
				break;
		}
		if (flaginput)
			break;
	}
	if (verbose) {
		if (flagcreat)
			fprintf(stderr, "Force creation of database %s.\n",
				DATABASE);
		if (flagupdate)
			fprintf(stderr, "Attempt update of %s.\n",
				WHERE);
		if (flaginput)
			fprintf(stderr, "Input files (%s ...) specified.\n",
				*argv);
	}
	if (!flaginput) {
		(void)fclose(stdout);
		(void)fclose(stdin);
	}
	init(flagcreat, flagupdate, flaginput);
	if (!flaginput) {
		char tmpbuf[64];

 		if ((fin = fopen(PATHS, "r")) == (FILE *)NULL) {
			perror (PATHS);
			exit (1);
		}
		(void)sprintf(tmpbuf, "%s.%d", TMP, getpid());
		if (freopen(tmpbuf, "w", stdout) == (FILE *)NULL) {
			perror (tmpbuf);
			exit (1);
		}
		parsefile(fin, PATHS);
		(void)fclose(stdout);
		if (unlink (WHERE) == -1 || link(tmpbuf, WHERE) == -1) {
			perror(WHERE);
			exit(1);
		}
		(void)unlink(tmpbuf);
		exit (0);
	}
	for (; *argv != NULL; argv++) {
		if (strcmp (*argv, "-") == 0) {
			parsefile(stdin, "stdin");
			continue;
		}
		if ((fin = fopen(*argv, "r")) != (FILE *)NULL) {
			parsefile(fin, *argv);
			continue;
		}
		perror (*argv);
	}
	exit(0);
}

usage()
{
	fprintf (stderr, "usage:  uuwhere [-c] [-v] [infiles]\n");
	exit (1);
}

init(flagcreat, flagupdate, flaginput)
int flagcreat, flagupdate, flaginput;
{
	char datadir[512], datapag[512];
	struct stat statuucp, statpaths, statwhere;
	int fd;
	char c;

/*
 * If any of the stats fail,
 * an open will fail later and produce a diagnostic.
 */
	if (flagupdate
	&& stat(WHERE, &statwhere) != -1
	&& stat(PATHS, &statpaths) != -1
	&& stat(UUCPINDEX, &statuucp) != -1) {
		if (statwhere.st_mtime > statpaths.st_mtime
		&& statwhere.st_mtime > statuucp.st_mtime) {
			if (verbose)
				fprintf (stderr, "%s up to date\n", WHERE);
			exit(0);
		}
		if (statuucp.st_mtime > statwhere.st_mtime
		|| statuucp.st_mtime > statpaths.st_mtime)
			flagcreat = 1;
	}
	if (!flaginput) {
		/* touch WHERE to forestall duplicate uuwheres */
		if ((fd = open(WHERE, 2)) < 0) {
			perror(WHERE);
			exit(1);
		}
		if (read (fd, &c, 1) == 1)
			(void) write (fd, &c, 1);
		(void) close (fd);
	}
	(void) sprintf (datadir, "%s.dir", DATABASE);
	(void) sprintf (datapag, "%s.pag", DATABASE);
	if (flagcreat || access(datadir, 0) == -1 || access(datapag, 0) == -1) {
		flagcreat = 1;
		if (verbose)
			fprintf (stderr, "Creating database %s...\n", DATABASE);
		if (makefile(datadir) == -1 || makefile(datapag) == -1)
			exit(1);
	}
	if (dbminit(DATABASE) < 0)
		exit(1);
	if (!flagcreat)
		return;
	if (!initit(UUCPINDEX))
		exit(1);
	if (verbose)
		fprintf (stderr, "...database %s created.\n", DATABASE);
}

makefile(name)
char *name;
{
	register int fd;

	if ((fd = creat(name, 0666)) == -1) {
		perror(name);
		return(-1);
	}
	(void) close(fd);
	return (0);
}

initit(name)
char *name;
{
	register FILE *fp;
	char buffer[1024], site[128], where[128];
	datum key, data;

	if (verbose)
		fprintf(stderr, "Reading %s...", name);
	if ((fp = fopen(name, "r")) == (FILE *)NULL) {
		perror(name);
		return (0);
	}
 	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		if (sscanf(buffer, "%s%s", site, where) != 2) {
			fprintf (stderr, "Can't parse:  %s\n", buffer);
			continue;
		}
		key.dptr = site;
		key.dsize = strlen(key.dptr) + 1;
		data.dptr = where;
		data.dsize = strlen(data.dptr) + 1;
		store (key, data);
	}
	(void)fclose(fp);
	if (verbose)
		fprintf(stderr, ".\n");
	return(1);
}

parsefile(fin, name)
register FILE *fin;
char *name;
{
	char buffer[128];
	register char *cp;
	register int c;
	register int inside;
	register int last = 0;

	if (verbose)
		fprintf(stderr, "%s\n", name);
	for (inside = 0; (c = getc(fin)) != EOF; putchar(c)) {
		if (isalnum(c) || c == '.' || c == '-' || c == '_') {
			if (!inside) {
				cp = buffer;
				inside = 1;
			}
			*cp++ = c;
			continue;
		}
		if (inside) {
			*cp = '\0';
			if (cp != buffer
/* %s */	&& !((cp - buffer) == 1 && last == '%' && buffer[0] == 's'))
				doit(buffer);
			inside = 0;
		}
		last = c;
	}
}

doit (site)
char *site;
{
	datum key, data;

	key.dptr = site;
	key.dsize = strlen(key.dptr) + 1;
	data = fetch(key);
	if (data.dptr != NULL)
		printf ("(%s)", data.dptr);
}
