/*
 * Cpiolist.c -- list a cpio tape written with binary headers
 * (i.e. written without the -c option).  Usage is "cpiolist file"
 * where 'file' is the cpio archive (probably /dev/rmt12).
 *
 * Writen, produced, and directed by:
 * Roy Smith <allegra!phri!roy>
 * The Public Health Research Institute
 *   of the City of New York, Inc.
 * 455 First Avenue
 * New York, NY 10016
 */

struct hdr
{
	short		h_magic;
	short		h_dev;
	unsigned short	h_ino;
	unsigned short	h_mode;
	unsigned short	h_uid;
	unsigned short	h_gid;
	short		h_nlink;
	short		h_rdev;
	short		h_mtime[2];
	short		h_namesize;
	short		h_filesize[2];
};

struct longword
{
	short word[2];
};

main (argc, argv)
int argc;
char *argv [];
{
	struct hdr header;
	int n, i, fd, odd, size;
	struct longword *lwp;
	char c, *ctime(), name[100];
	short temp;

	/*
	 * Attempt to open the cpio archive file.
	 */
	if ((fd = open (argv[1], 0)) < 0)
	{
		printf ("can't open file\n");
		exit (1);
	}

	while (1)
	{
		/* Read in a header */
		n = read (fd, &header, sizeof (header));
	
		/*
		 * The high and low order words of the time are
		 * in the wrong order on the tape; swap them.
		 */
		temp = header.h_mtime[0];
		header.h_mtime[0] = header.h_mtime[1];
		header.h_mtime[1] = temp;

		/* likewise, swap the high and low order sizes */
		lwp = (struct longword *) &size;
		lwp->word[0] = header.h_filesize[1];
		lwp->word[1] = header.h_filesize[0];

		/* check for trouble */
		if (header.h_magic != 070707)
		{
			printf ("out of sync!\n");
			exit (1);
		}

		/* print the header info */
		printf ("%5d", header.h_ino);
		printf ("%9#o  ", header.h_mode);
		printf ("%.24s", ctime (header.h_mtime));
		printf ("%7d ", size);

		/*
		 * read in the file name -- notice that if the name is
		 * an odd number of bytes long, it is padded to be even.
		 */
		if ((n = header.h_namesize) % 2 != 0)
			odd = 1;
		else
			odd = 0;

		i = 0;
		while (n--)
		{
			read (fd, &c, 1);
			name [i++] = c;
		}
		printf ("%s\n", name);
		if (odd)
			read (fd, &c, 1);

		/* the file data is likewise padded */
		if (size % 2 != 0)
			size++;

		/*
		 * we don't actually want the file,
		 * so just skip to the next header.
		 */
		lseek (fd, size, 1);
	}
}
