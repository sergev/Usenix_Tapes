/*
 * fskip - skip files on magtape
 *
 * Written by Hughes O'Neil, 14/Nov/77.
 *
 * Modified for V7 by Alexis Kwan (HCR for DCIEM), 16/Feb/81.
 */

#define	BUFFSIZE	512*60

/* This must be aligned on a word boundary */
char	buff[BUFFSIZE];

main (argc, argv)
	int argc;
	char *argv[];
{
	int mt, num_files, n;

	if (argc <= 1)
		num_files = 1;
	else
		num_files = atoi(argv[1]);
	/* Make sure tape is rewound first */
	if ((mt = open("/dev/rmt0",0)) == -1) {
		perror("fskip");
		exit(1);
	}
	close(mt);
	if ((mt = open("/dev/nrmt0",0)) == -1) {
		perror("fskip");
		exit(1);
	}
	for (n = 1; n<=num_files; n++) {
		while (read(mt,buff,BUFFSIZE) > 0)
			;
	}
	close(mt);
	exit(0);
}
