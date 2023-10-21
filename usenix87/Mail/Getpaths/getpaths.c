/*
	getpaths.c
	Program to snarf up lots of Usenet paths
	Joseph T. Buck, Entropic Processing, Inc.
*/
#define SPOOLDIR "/usr/spool/news"
#include <stdio.h>
#define LINLEN 512
char *strcpy();

int stripuser = 1;

/* This function assumes the current directory is SPOOLDIR, and that
   the paths are of the form net/news/group/2345.
*/
void perror(), exit();

wp (name, ino)
char *name;
int ino;
{
    int status = 0, lng;
    char line[LINLEN], savpath[LINLEN];
    FILE *fd;
    char *p, *q, *rindex();
    char curngr[40];	/* current newsgroup */

/* Figure out the current newsgroup, by deleting whatever is
   after the last /, and changing '/' to '.'. Skip initial "./"
   if present
*/
    if (name[0] == '.' && name[1] == '/') name += 2;
    (void) strcpy (curngr, name);
    p = rindex (curngr, '/');
    if (p == NULL) return;
    *p = 0;
    while (p >= curngr) {
	if (*p == '/') *p = '.';
	p--;
    }
    lng = strlen (curngr);
/* Open the file */
    if ((fd = fopen (name, "r")) == NULL) {
	perror (name);
	return;
    }
    while (status != 3) {
	if (fgets (line, LINLEN, fd) == NULL) break;
	line[strlen(line)-1] = 0; /* delete \n */
	if (strncmp (line, "Newsgroups:", 11) == 0) {
	    p = line + 11;
	    while (*p == ' ' || *p == '\t') p++;
/* Detect cases like curngr == "net.micro", NG == "net.micro.pc,net.micro" */
	    if (p[lng] && p[lng] != ',' && p[lng] != ' ' && p[lng] != '\t')
		break;
	    else if (strncmp (p, curngr, lng) != 0) break;
	    status |= 2;
	}
	else if (strncmp (line, "Path:", 5) == 0) {
	    p = line + 5;
	    while (*p == ' ' || *p == '\t') p++;
/* Strip the username from the end of the path. Don't write anything
   if there's no ! in the path (a locally posted article)
*/
	    if (stripuser) {
		q = rindex (line, '!');
		if (q == NULL) break;
		*q = '\0'; /* Delete the tail */
	    }
	    (void) strcpy (savpath, p);
	    status |= 1;
	}
    }
    if (status == 3) (void) printf ("%s\n", savpath);
    (void) fclose (fd);
    return;
}

main (argc, argv)
char **argv;
{
    if (chdir (SPOOLDIR) != 0) {
	(void) fprintf (stderr, "Can't chdir to ");
	perror ("/usr/spool/news");
	exit (1);
    }
    if (argc > 1 && strcmp (argv[1], "-u") == 0) {
	stripuser = 0;
	argv++;
	argc--;
    }
    if (argc !=2) argv[1] = ".";
    scan_tree (argv[1], wp);
    return 0;
}

#include <sys/types.h>
#include <sys/stat.h>

is_dir (name)
char *name;
{
    struct stat sbuf;
    return (stat (name, &sbuf) == 0 && (sbuf.st_mode & S_IFMT) == S_IFDIR);
}
