#include <stdio.h>
#include <ctype.h>

char curfile[512];
FILE *curfp = NULL;
int cxtrange = 3;

char *gets();

main(argc, argv)
char **argv; {
	char context[512], fline[512], *cp, *fcp;
	long cxtln, lolino, hilino, curln;

	if (argc > 2) {
		fprintf(stderr, "Usage: context [nlines] < listfile\n");
		exit(1);
	}
	if (argc == 2)
		if ((cxtrange = atol(argv[1])) < 1 || cxtrange > 25)
			cxtrange = 3;
	while (gets(context) != (char *) 0) {
		for (cp = context; *cp != '.' && *cp != '/' && *cp != '-' && *cp != '_' && !isalnum(*cp); cp++)
			if (*cp == '\0')
				break;
		if (*cp == '\0')
			continue;
		strcpy(fline, cp);
		for (fcp = cp, cp = fline; *cp == '_' || *cp == '.' || *cp == '/' || *cp == '-' || isalnum(*cp); cp++, fcp++)
			;
		if (*cp == '\0')
			continue;
		*cp = '\0';
		if (curfp == (FILE *) 0 || strcmp(curfile, fline) != 0) {
			if (curfp != (FILE *) 0)
				fclose(curfp);
			if ((curfp = fopen(fline, "r")) == (FILE *) 0) {
				perror(fline);
				continue;
			}
			strcpy(curfile, fline);
			curln = 0;
		}
		for (; *fcp != '\0' && !isdigit(*fcp); fcp++)
			;
		if (*fcp == '\0')
			continue;
		cxtln = atol(fcp);
		lolino = (cxtln < cxtrange - 1? 1: cxtln - cxtrange);
		hilino = cxtln + cxtrange;
		if (lolino < curln) {
			fseek(curfp, 0L, 0);
			curln = 0;
		}
		if (cxtln == curln) {		/* already shown */
			printf("*****\n* %s\n*****\n\n", context);
			continue;
		}
		printf("**************\n* %s\n*****\n", context);
		while (fgets(fline, 512, curfp) != (char *) 0 && ++curln < lolino)
			;
		if (curln < lolino)
			continue;
		out(fline, curln == cxtln);
		while (fgets(fline, 512, curfp) != (char *) 0 && ++curln <= hilino)
			out(fline, curln == cxtln);
		putchar('\n');
	}
}

out(s, flg)
char *s; {
	printf("%c %s", (flg? '*': ' '), s);
}
