/* uuencode.c */
     
/*
uudecode and uuencode are easily implemented under MSDOS as well.  Here
are the sources for Microsoft C v3.0, but if you have another kind of C
compiler, there should be perhaps only 1 change -- the output file of
uudecode and the input file of uuencode must be in binary format.
(ie.  binary files, like .EXE files may have byte patterns that are the
same as ^Z, which signals end-of-file in non-binary (text) mode).

	Don Kneller
UUCP:	...ucbvax!ucsfcgl!kneller
ARPA:	kneller@ucsf-cgl.ARPA
BITNET:	kneller@ucsfcgl.BITNET
*/

#ifndef lint
static char sccsid[] = "@(#)uuencode.c	5.1 (Berkeley) 7/2/83";
#endif

/*
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) (((c) & 077) + ' ')

main(argc, argv)
char **argv;
{
	FILE *in;
	struct stat sbuf;
	int mode;

	/* optional 1st argument */
	if (argc > 2) {
#ifdef MSDOS
		/* Use binary mode */
		if ((in = fopen(argv[1], "rb")) == NULL) {
#else
		if ((in = fopen(argv[1], "r")) == NULL) {
#endif
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else
		in = stdin;

	if (argc != 2) {
		printf("Usage: uuencode [infile] remotefile\n");
		exit(2);
	}

	/* figure out the input file mode */
	fstat(fileno(in), &sbuf);
	mode = sbuf.st_mode & 0777;
	printf("begin %o %s\n", mode, argv[1]);

	encode(in, stdout);

	printf("end\n");
	exit(0);
}

/*
 * copy from in to out, encoding as you go along.
 */
encode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	int i, n;

	for (;;) {
		/* 1 (up to) 45 character line */
		n = fr(in, buf, 45);
		putc(ENC(n), out);

		for (i=0; i<n; i += 3)
			outdec(&buf[i], out);

		putc('\n', out);
		if (n <= 0)
			break;
	}
}

/*
 * output one group of 3 bytes, pointed at by p, on file f.
 */
outdec(p, f)
char *p;
FILE *f;
{
	int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
	c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
	c4 = p[2] & 077;
	putc(ENC(c1), f);
	putc(ENC(c2), f);
	putc(ENC(c3), f);
	putc(ENC(c4), f);
}

/* fr: like read but stdio */
int
fr(fd, buf, cnt)
FILE *fd;
char *buf;
int cnt;
{
	int c, i;

	for (i=0; i<cnt; i++) {
		c = getc(fd);
		if (c == EOF)
			return(i);
		buf[i] = c;
	}
	return (cnt);
}
