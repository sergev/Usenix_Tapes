# include <stdio.h>
# include <ctype.h>
# define DIR 2048

char *drvnam[] = {
	"/dev/rx0c",
	"/dev/rx1c"
};

main(argc, argv)
int argc;
char *argv[];
{
	int fd, i, drive;
	char buf[DIR];

	if (argc != 2 || isdigit(*argv[1]) == 0) {
		fputs("Usage:  zero 0|1\n", stderr);
		exit(1);
	}
	drive = atoi(argv[1]);
	if (drive < 0 || drive >= (sizeof drvnam / sizeof drvnam[0])) {
		fprintf(stderr, "%s bad drive number\n", argv[1]);
		exit(1);
	}
	if ((fd = creat(drvnam[drive], 0644)) == -1) {
		fprintf(stderr, "%s: cannot open\n", drvnam[drive]);
		exit(1);
	}
	for (i = 0; i < DIR; i++)
		buf[i] = 0xe5;
	if (write(fd, buf, DIR) != DIR) {
		fputs("write error\n", stderr);
		exit(1);
	}
	close(fd);
	exit(0);
}
