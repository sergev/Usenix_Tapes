/*	SCAME scame-reaper.c		*/

/*	Revision 1.0.2  1985-02-19	*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";


#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include "config.h"

#define MAXPIDS 200

#define min(a,b)  ((a)<(b)?(a):(b))

int nentries= 0;
int nunlinked= 0;
struct direct dirent, *dp = &dirent;
extern FILE *popen();
char tempdir[] = TMPFILE;

extern char *rindex();

main(argc,argv)
int argc;
char **argv;
{
int trace = (argc > 1);
int pids[MAXPIDS], i,n,pid;
#ifdef DIRBLKSIZ
DIR *d;
#endif
FILE *f;
char line[80];
long t;
char *ctime();
char *s, *cp;

	if ((cp = rindex(tempdir, '/')) != (char *)0)
		*cp = '\0';
	chdir(tempdir);
	if (trace) {
		printf("Scame-Reaper:\n");
		system("date");
		system("pwd");
	}
#ifdef S5
	if ((f = popen("ps -el", "r")) == NULL)
#else
	if ((f = popen("ps -ax", "r")) == NULL)
#endif
		fatal("Can't do ps");
	i = 0;
	fscanf(f, "%*[^\n]");
	do fscanf(f, "%d%*[^\n]", &pids[i]);
 	while ((pids[i] != 0 || i == 0) && ++i < MAXPIDS);
	n=i;
	pclose(f);

#ifdef DIRBLKSIZ
	if ((d=opendir(".")) == NULL)
		fatal("Can't open directory file");
	while ((dp = readdir(d)) != 0) {
#else
	if ((f=fopen(".", "r")) == NULL)
		fatal("Can't open directory file");
	while (fread(dp, sizeof dirent, 1, f) != 0) {
#endif
		if (dp->d_ino == 0 || dp->d_name[0] != 'S')
 			continue;
		for (i=1; i <= 10; i++)
			if (dp->d_name[i] < '0' || dp->d_name[i] > '9')
				goto loop;
		nentries++;
		sscanf(dp->d_name, "%*[^0123456789]%5d", &pid);
		for (i=0; i<n && pids[i] != pid; i++);
		if (i==n)
			if (unlink(dp->d_name) != 0)
 				printf("Can't remove %s\n", dp->d_name);
			else nunlinked++;
loop:		continue;
	}
	pclose(f);
	if (trace) printf("Done. %d files, %d removed\n", nentries, nunlinked);
}

fatal(s)
char *s;
{
	printf("%s, ", s);
	perror();
	exit(1);
}
