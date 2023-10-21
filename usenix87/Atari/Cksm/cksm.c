#include <stdio.h>
#ifdef vax
#include <fcntl.h>
#else
#include <osbind.h>
#endif

#define BUFRSIZ	1024
#define FD_STDIN 0

char buff[BUFRSIZ];

main(argc, argv)	/*  cksm:  Checksum files	*/
int argc;
char *argv[];

{
	/*  Compilation instructions:  cc -O cksm.c cksblk.c		*/

	int fildes;

	if (argc == 1)  {	/*  no args, so process std. input	*/
		chksum(FD_STDIN, "standard input");
	}
	else  {
		while (--argc > 0)  {
			if ((fildes = 
#ifdef vax
			      open(*++argv, O_RDONLY | O_NDELAY)) <= 0) {
#else
			      Fopen(*++argv, 0)) <= 0) {
#endif
				printf("cksm: can't open '%s' error no = %d\n", *argv, fildes);
			}
			else  {
				chksum(fildes, *argv);
				close(fildes);
			}
		}
	}
	printf("Hit return to continue\n");
	scanf("%c",buff);
}

chksum(fildes, fname)	/*  Read file, count bytes, compute checksum.	*/
int fildes;
char *fname;

{
	int csum, nlines;
	long bcnt, bgot;
	extern char buff[BUFRSIZ];
	extern errno;

	csum = 0;
	bcnt = 0;
	nlines = 0;

#ifdef vax
	while ((bgot = read(fildes, buff, BUFRSIZ)) > 0)  {
#else
	while ((bgot = Fread(fildes, (long)BUFRSIZ, buff)) > 0)  {
#endif
		bcnt += bgot;
		csum = cksblk(buff, bgot, csum, &nlines);
	}

	if (bgot < 0)  {
		printf("cksm: error %ld reading from %s\n", bgot, fname);
	}

#ifdef vax
	printf("bytes =%9ld(%9ld)   cksm =%5d   %s\n", bcnt, bcnt+nlines, csum, fname);
#else
	printf("bytes =%9ld   cksm =%5d   %s\n", bcnt, csum, fname);
#endif
}
