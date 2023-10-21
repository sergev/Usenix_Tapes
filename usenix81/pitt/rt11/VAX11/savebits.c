/*
 *  Patch the header of an executable binary file
 *  so that the relocation data is included in
 *  the data segment where it can be examined
 *  by the executing program.
 *  Note that this displaces the bss segment so that
 *  the executing program may have to clear the bss
 *  itself before it can run (correctly).
 *
 *  This is the VAX version.
 *  It was developed under 4.0BSD and is expected to
 *  work under 32V, 3BSD, and 4.1BSD (but has not been
 *  tested in these environments).
 *
 *	Daniel R. Strick
 *	4/18/81
 */

#include <a.out.h>

main (argc, argv)
char **argv;
{
	int fd;
	int rsize;
	struct exec header;

	if (argc != 2) {
		printf ("usage:	 %s  executable-file\n", argv[0]);
		exit (1);
	}
	fd = open(argv[1], 2);
	if (fd < 0) {
		perror (argv[1]);
		exit (1);
	}
	if (read(fd, &header, sizeof header)  !=  sizeof header) {
		perror (argv[1]);
		exit (1);
	}
	if (header.a_magic != OMAGIC) {
		printf ("%s: must be impure executable binary file\n", argv[1]);
		exit (1);
	}
	rsize = header.a_trsize + header.a_drsize;
	if (rsize == 0) {
		printf ("%s: relocation data has been stripped\n", argv[1]);
		exit (1);
	}
	header.a_data += rsize;
	header.a_trsize = 0;
	header.a_drsize = 0;
	lseek (fd, 0L, 0);
	if (write (fd, &header, sizeof header)  !=  sizeof header) {
		perror(argv[1]);
		exit (1);
	}
	close (fd);
	exit(0);
}
