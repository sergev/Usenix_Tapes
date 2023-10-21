#
/*
**      REPLACE -- Replace all occurrences of argv[1] with argv[2]
**          in files argv[3] ... argv[argc-1]
**          Will do multiple files, preserves links and makes NO backup
**  Compile: cc -O -q replace.c -lP
**      Copyright (c) 1976, by Peter S. Langston
*/

char    *whatline "@(#)replace.c	1.1  last mod 6/25/79 -- P.S.L. misc package";

#define   EX_OK     0
#define   EX_BAD    1
#define   EX_SYN    2
#define   EX_SYS    3

char	*old, *new;
struct  iobuf {
     int  fildes, nleft;
     char *buffp, buff[512];
} ibuf, obuf;

main(argc,argv)
char **argv;
{
	register char *file;
	char *tmp;
	int count, total;

	if (argc < 4) {
		printf("Usage: replace old new file1 [ file2 ... ]\n");
		exit(EX_SYN);
	}
	tmp = tmpfil("replace", "");
	old = argv[1];
	new = argv[2];
	argc =- 2;
	argv =+ 2;
	while (--argc) {
		file = *++argv;
		printf("%s: ", file);
		if (fopen(file, &ibuf) < 0) {
			perror(file);
			exit(EX_SYS);
		}
		if (fcreat(tmp, &obuf) < 0) {
			perror(tmp);
			exit(EX_SYS);
		}
		count = replace();
		total =+ count;
		fflush(&obuf);
		close(ibuf.fildes);
		close(obuf.fildes);
		if (count > 0) {
			printf("%d replacement%s", count, splur(count));
			fcopy(tmp, file);
		}
		if (unlink(tmp) < 0) {
			perror(tmp);
			exit(EX_SYS);
		}
		printf("\n");
	}
	printf("	Total replacements : %d\n", total);
	exit(EX_OK);
}

replace()
{
	register char *cp, *ccp, c;
	int count;

	count = 0;
	cp = old;
	c = getc(&ibuf);
	do {
		if (c != *cp) {
			putc(c, &obuf);
			c = getc(&ibuf);
			continue;
		}
		while ((c=getc(&ibuf)) == *++cp);
		if (*cp) {
			ccp = old;
			while (ccp < cp)
				putc(*ccp++, &obuf);
		} else {
			cp = new;
			while (*cp)
				putc(*cp++, &obuf);
			count++;
		}
		cp = old;
	} while (c != -1);
	return(count);
}

fcopy(from, to)
char	*from, *to;
{
	register int i, ffh, tfh;
	char buf[512];

	if ((ffh = open(from, 0)) < 0) {
		perror(from);
		exit(EX_SYS);
	}
	if ((tfh = creat(to, 0600)) < 0) {
		perror(to);
		exit(EX_SYS);
	}
	while ((i = read(ffh, buf, 512)) > 0)
		write(tfh, buf, i);
	close(ffh);
	close(tfh);
}
