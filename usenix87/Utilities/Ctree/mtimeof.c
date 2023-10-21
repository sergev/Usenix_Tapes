#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

struct stat sb;

main(argc, argv) char **argv; {
	if (argc != 2) {
		fprintf(stderr, "Usage: mtimeof file\n");
		exit(-1);
	}
	if(stat(argv[1], &sb) < 0) {
		printf("Never created or modified.");
		exit(0);
	}
	printf("%.16s\n", ctime(&(sb.st_mtime)));
}
