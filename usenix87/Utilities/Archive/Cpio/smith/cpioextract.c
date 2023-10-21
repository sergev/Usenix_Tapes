/*
 * Cpioextract.c -- extract a single file from a cpio archive
 * written with binary headers (i.e. without the -c option)
 *
 * Usage is: cpioextract cpio_archive_file archived_file_name
 *
 * Written by:
 * Roy Smith <allegra!phri!roy>
 * The Public Health Research Institute
 *   of the City of New York, Inc.
 * 455 First Avenue, New York, NY 10016
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

	/* attempt to open cpio archive file */
	if ((fd = open (argv[1], 0)) < 0)
	{
		printf ("can't open file\n");
		exit (1);
	}

	while (1)
	{
		/* read in a header */		
		n = read (fd, &header, sizeof (header));
	
		/* tape has high and low order times reversed, fix them */
		temp = header.h_mtime[0];
		header.h_mtime[0] = header.h_mtime[1];
		header.h_mtime[1] = temp;

		/* likewise, the file size */
		lwp = (struct longword *) &size;
		lwp->word[0] = header.h_filesize[1];
		lwp->word[1] = header.h_filesize[0];

		/* sanity check */
		if (header.h_magic != 070707)
		{
			printf ("out of sync!\n");
			exit (1);
		}

		/*
		 * if the file name is an odd number of characters, it is
		 * padded on the tape to be even; deal with this.
		 */
		if ((n = header.h_namesize) % 2 != 0)
			odd = 1;
		else
			odd = 0;

		/* read the file name from the archive */
		i = 0;
		while (n--)
		{
			read (fd, &c, 1);
			name [i++] = c;
		}
		if (odd)
			read (fd, &c, 1);

		/* is this the file we want? */
		if (strcmp (name, argv[2]) == 0)
		{
			copyout (fd, size);
			exit (0);
		}
		else
		{
			/*
			 * this wasn't the right file; skip past it (and one
			 * extra padding byte if the file size was odd).
			 */
			if (size % 2 != 0)
				size++;

			lseek (fd, size, 1);
		}
	}
}

/*
 * pull a file out of the archive.  fd is a file descriptor for the
 * cpio archive, n is the size of the file we are supposed to get.
 * the file is output to standard output.
 */ 	
copyout (fd, n)
int fd, n;
{
	int nk;
	char buf[1024];

	/*
	 * how many full kilobyte blocks are there,
	 * and how many bytes left over? */
	 */
	nk = n / 1024;
	n %= 1024;

	/* get all the whole blocks */
	while (nk--)
	{
		read (fd, buf, 1024);
		write (1, buf, 1024);
	}
	/* and get the residual fractional kilobyte */
	read (fd, buf, n);
	write (1, buf, n);
}
