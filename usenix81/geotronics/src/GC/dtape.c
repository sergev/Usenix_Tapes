/* dtape	transfers card images from disk to magtape. */

#include	<stdio.h>
#define	BUFSIZE	80
static char buf[BUFSIZE];
static int fd;

main(argc, argv)
int argc;
char *argv[];
{

	if ((fd = open("/dev/rmt0", 1)) == -1)
	{
		fprintf(stderr, "cannot open tape file\n");
		exit(1);
	}
	if (--argc == 0)
		check("stdin");
	else
		while (--argc >= 0)
		{
			if (freopen(*++argv, "r", stdin) != NULL)
				check(*argv);
			else
				fprintf(stderr, "cannot open %s\n", *argv);
		}
	close(fd);
}

check(fil)
char *fil;
{
	int j = 0;
	int c;

	while ((c = getchar()) != EOF)
	{
		if (c != '\n')
			if (j < BUFSIZE)
				buf[j++] = c;
			else
			{
			ungetc(c, stdin);
			fprintf(stderr, "line longer than 80 chars %s\n", fil);
			outrec(j);
			j = 0;
			}
		else	// if c = newline.
		{
			outrec(j);
			j = 0;
		}
	}
	if (j != 0)
		outrec(j);
}

outrec(l)
int l;
{

	while (l < BUFSIZE)
		buf[l++] = ' ';
	if (write(fd, buf, BUFSIZE) != BUFSIZE)
	{
		fprintf(stderr, "bad write\n");
		exit(1);
	}
}
