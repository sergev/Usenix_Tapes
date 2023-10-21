/*
 * Patch the header of an executable binary file
 * so that the relocation bits are included in
 * the data segment where they may be examined
 * by the executing program.
 * Note that this displaces the bss segment so that
 * the executing program may have to clear the bss
 * itself before it can run (correctly).
 *
 *	Daniel R. Strick
 *	April, 1979
 *	7/21/81 -- changed seek() to lseek()
 */

main (argc, argv)
char **argv;
{
	int fd;
	int header[8];

	if (argc != 2) {
		printf ("usage:	 %s  executable-file\n", argv[0]);
		return;
	}
	fd = open(argv[1], 2);
	if (fd < 0) {
		perror (argv[1]);
		return;
	}
	if (read(fd, header, sizeof header)  !=  sizeof header) {
		perror (argv[1]);
		return;
	}
	if (header[0] != 0407) {
		printf ("%s: must be simple executable binary file\n", argv[1]);
		return;
	}
	if (header[7] != 0) {
		printf ("%s: relocation bits have been stripped\n", argv[1]);
		return;
	}
	header[2] += header[1] + header[2];
	header[7] = 1;
	lseek(fd, 0L, 0);
	if (write(fd, header, sizeof header)  !=  sizeof header) {
		perror(argv[1]);
		return;
	}
	close(fd);
}
