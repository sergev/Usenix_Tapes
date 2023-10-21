#include    <stdio.h>
/*
**      INTO -- End of a pipe that transfers std. input into a temp file
**          and, when EOF is read, gives it the name of the argument
**          Useful in cases like:   % rpl <foo "a" "b" | into foo
** Compile: cc -O -q into.c -lS -o into
**      (c) P. Langston, 1979
*/

char    *whatline "@(#)into.c	1.1  last mod 9/22/79 -- P.S.L. misc package";

main(argc, argv)
char **argv;
{
	register char *cp, *ep, *fp;
	char buf[256], tname[128];
	FILE *tfp;

	if (argc != 2) {
	    fprintf(stderr, "Usage: %s filename\n", argv[0]);
	    exit(2);
	}
	ep = fp = tname;
	for (cp = argv[1]; *fp = *cp++; )
	    if (*fp++ == '/')
		ep = fp;
	sprintf(ep, "%d", getpid());
	if ((tfp = fopen(tname, "w")) == NULL) {
	    perror(tname);
	    exit(3);
	}
	while (fgets(buf, sizeof buf, stdin) != NULL)
	    fputs(buf, tfp);
	unlink(argv[1]);
	if (link(tname, argv[1]) == -1) {
	    perror("Unable to link");
	    fprintf(stderr, "Output left in %s instead of %s\n",
	     tname, argv[1]);
	    exit(1);
	} else {
	    unlink(tname);
	    exit(0);
	}
}
