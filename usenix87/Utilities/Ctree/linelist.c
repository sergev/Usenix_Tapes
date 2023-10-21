#include <stdio.h>
main(argc, argv) char **argv; {
	int lineno;
	FILE *f;
	char line[256];

	if (argc != 2)
		f = stdin;
	else if ((f = fopen(argv[1], "r")) == NULL)
		exit(-1);
	lineno = 0;
	while (fgets(line, 256, f) != NULL)
		printf("Line %d:\n%s\n", ++lineno, line);
	fclose(f);
}
