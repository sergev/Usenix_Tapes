#include "stdio.h"
#include "/usr/sys/filsys.h"
#define	DEV	1
#define	FSNAME	2
#define	VOLUME	3
 /* write fsname, volume # on disk superblock */
struct {
	char	fill1[512];
	struct filsys fs;
	char	fill2[5120-1024];
} super;

main(argc, argv)
char **argv;
{
	int	fsi, fso;
	long	curtime;
	int	i;

	if (argc != 4 && argc != 2 && argc != 5) {
showusage:
		fprintf(stderr, "Usage: labelit /dev/r??? [fsname volume [-n]]\n");
		exit(1);
	}
	if (argc == 5) {
		if (strcmp(argv[4], "-n") != 0)
			goto showusage;
		printf("Skipping label check!\n");
		goto do_it;
	}

	if ((fsi = open(argv[DEV], 0)) < 1)
		fprintf(stderr, "Cannot open device\n");

	if ((i =read(fsi, &super, 5120)) != 5120)
		fprintf(stderr, "Cannot read 5120 char block.\n");

	printf("Current fsname: %.6s, Current volname: %.6s\n",
		super.fs.s_fname, super.fs.s_fpack);
	printf("Date last mounted: %s", ctime(&super.fs.s_time));
	if (argc == 2)
		exit(0);
do_it:
	printf("NEW fsname: %.6s, NEW volname: %.6s\n[confirm]",
		argv[FSNAME], argv[VOLUME]);
	while (getchar() != '\n');
	sprintf(super.fs.s_fname, "%.6s", argv[FSNAME]);
	sprintf(super.fs.s_fpack, "%.6s", argv[VOLUME]);

	close(fsi);
	fso = open(argv[DEV], 1);
	if (write(fso, &super, 5120) < 0) {
		fprintf(stderr, "Cannot write label\n");
		exit(1);
	}
	exit(0);
}
