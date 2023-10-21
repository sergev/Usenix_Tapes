/* From: larry@teltone.UUCP (Larry J. Barello)	*/

#include <stdio.h>
 
char *getenv();
char *index();

int
main(ac, av)
    int ac;
    char **av; {
	char *path, *cp;
	char buf[200];
	char patbuf[512];
	int quit, none;

	if (ac < 2) {
		fprintf(stderr, "Usage: %s cmd [cmd, ..]\n", *av);
		exit(1);}
	av[ac] = 0;
	for(av++ ; *av; av++) {
		quit = 0;
		none = 1;
		strcpy(patbuf, getenv("PATH"));
		path = patbuf;
		cp = path;

		for(;;) {
			cp = index(path, ':');
			if (cp == NULL) {
				quit++;}
			else {
				*cp = '\0';}

			sprintf(buf, "%s/%s", path, *av);
			path = ++cp;

			if (access(buf, 1) == 0) {
				printf("%s\n", buf);
				none = 0;}
			if (quit) {
				if (none) {
					printf("No %s in %s\n", *av, getenv("PATH"));}
				break;}}}
	exit(0);}
