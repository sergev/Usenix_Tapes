#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

struct stat sb;
extern char *index(), *basename();
char *mtimeof();

main(argc, argv) char **argv; {
	FILE *f;
	char usagefile[1024], *cp;
	int usage, cnt;

	if (argc != 3) {
		fprintf(stderr, "Usage: indechof file indmark\n");
		exit(0);
	}
	sprintf(usagefile, "%s/.users", argv[1]);
	if ((f = fopen(usagefile, "r")) == NULL) {
		fprintf(stderr, "Who?\n");
		exit(-1);
	}
	fscanf(f, "%d", &usage);
	fclose(f);
	if (strcmp(argv[1], argv[2]) == 0) {
		printf("%-.14s\nUsers: %4d Created: %.16s\n\n", basename(argv[1]), usage, mtimeof(argv[1]));
		exit(0);
	}
	cnt = 0, cp = argv[1];
	while (argv[1][cnt] == argv[2][cnt])
		cnt ++, cp ++;
	cnt = 2;
	while ((cp = index(cp + 1, '/')) != NULL)
		cnt += 2;
	printf("%*s%-.14s\nUsers: %4d Created: %.16s\n\n", cnt, "", basename(argv[1]), usage, mtimeof(argv[1]));
}

char *mtimeof(file) char *file; {
	static char timestr[80];

	if(stat(file, &sb) < 0)
		return("Never created or modified.");
	sprintf(timestr, "%.16s\n", ctime(&(sb.st_mtime)));
	return(timestr);
}

char *rindex();

char *basename(file) char *file; {
	char *f;

	return((f = rindex(file, '/')) == NULL? file : f + 1);
}
