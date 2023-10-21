/*
	fsize - recursively print sizes of files

	Taken from "The C Programming Language" by Kernighan & Ritchie
*/

#include        <stdio.h>
#include        <sys/types.h>   /* typedefs */
#include        <sys/dir.h>     /* directory entry structure */
#include        <sys/stat.h>    /* structure returned by stat */
#define BUFSIZE 256

main(argc, argv)        /* fsize: print file sizes */
char *argv[];
{
	char buf[BUFSIZE];

	if (argc == 1) {        /* default: current directory */
		strcpy(buf, ".");
		fsize(buf);
	} else
		while (--argc > 0) {
			strcpy(buf, *++argv);
			fsize(buf);
		}
}

fsize(name)     /* print size for name */
char *name;
{
	struct stat stbuf;
	long ltemp;

	if (stat(name, &stbuf) == -1) {
		fprintf(stderr, "fsize: can't find %s\n", name);
		return;
	}
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
		directory(name);
	l3tol(&ltemp, &stbuf.st_size, 1);       /* for 6th edition UNIX */
	printf("%8ld %s\n", ltemp, name);
}

directory(name) /* fsize for all files in name */
char *name;
{
	struct direct dirbuf;
	char *nbp, *nep;
	int i, fd;

	nbp = name + strlen(name);
	*nbp++ = '/';   /* add slash to directory name */
	if (nbp+DIRSIZ+2 >= name+BUFSIZE)       /* name too long */
		return;
	if ((fd = open(name, 0)) == -1)
		return;
	while (read(fd, (char *)&dirbuf, sizeof(dirbuf))>0) {
		if (dirbuf.d_ino == 0)  /* slot not in use */
			continue;
		if (strcmp(dirbuf.d_name, ".") == 0
		  || strcmp(dirbuf.d_name, "..") == 0)
			continue;       /* skip self and parent */
		for (i=0, nep=nbp; i < DIRSIZ; i++)
			*nep++ = dirbuf.d_name[i];
		*nep++ = '\0';
		fsize(name);
	}
	close(fd);
	*--nbp = '\0';  /* restore name */
}
