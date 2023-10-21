/*
 * macsbugmap - generate map for use with macsbug
 *
 * usage:  macsbugmap MACSEGMENTORIGIN		(typically macsbugmap 4e48)
 *
 * Reads b.out and produces b.map file which is used with macsbug.
 * [This should be written in AWK, but there's no hex/octal input!]
 *
 * history
 * 10/19/84	Geyer/Nodine@BBN	Created.
 * 10/22/84	Croft	Hacked into single C program; renamed.
 */

#include <stdio.h>
#include <sys/file.h>

char	*argv0;

main (argc, argv)
char **argv;
{
	int offset;
	int addr;
	char type;
	char buffer[100];
	char name[100];
	FILE *fin, *fout, *popen();

	argv0 = argv[0];
	if (argc != 2)
		err("usage: %s MACSEGMENTORIGIN", argv0);
	unlink("b.map");
	if (access("b.out", 04) != 0)
		err("can't open b.out");
	if ((fin = popen("nm68 -nh b.out", "r")) == NULL)
		err("can't run nm68");
	if ((fout = fopen("b.map", "w")) == NULL)
		err("can't open output file, b.map");

	sscanf (argv[1], "%x", &offset);

	for (;;) {
		fgets (buffer, 100, fin);
		if (feof (fin))
			break;
		sscanf (buffer, "%x %c %s", &addr, &type, name);
		fprintf (fout, "%06x %c %s\n", addr+offset, type, name);
	}
	fclose(fout);
	exit(0);
}

err(s,a,b,c)
{
	fprintf(stderr, "%s: ", argv0);
	fprintf(stderr, s, a, b, c);
	putc('\n', stderr);
	exit(1);
}
