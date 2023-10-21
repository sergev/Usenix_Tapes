#include <stdio.h>
/*
	This command depends on chroot(2), which exists in 4BSD, System V,
	Version 7, and probably all related systems.
 */

#ifndef MAPS
#define MAPS "/usr/local/lib/news/maps"
#endif

main(argc,argv)
int argc;
char **argv;
{
	char *rootdir = MAPS;
	char *command = "/bin/sh";

	if (geteuid() != 0) {
		fprintf (stderr, "mapsh must be setuid to root\n");
		exit(1);
	}
	if (chroot(rootdir) == -1) {
		fprintf (stderr, "mapsh:  chroot(%s) failed\n", rootdir);
		perror ("");
		exit(1);
	}
	if (setuid(getuid()) == -1) {
		perror ("mapsh:  setuid(getuid()) failed");
		exit(1);
	}
	if (chdir("/") == -1) {
		fprintf (stderr, "mapsh:  chdir(%s) failed\n", "/");
		perror ("");
		exit(1);
	}
	execvp (command, argv);
	fprintf (stderr, "mapsh:  %s not found\n", command);
	perror ("mapsh:  execvp(2) failed");
	exit(1);
}
