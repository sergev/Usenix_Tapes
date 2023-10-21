/*
 * emitc -- convert time into an int.  compile with
 * getdate.o, which can be found with netnews source.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>

main(argc, argv)
char	**argv;
{
	if (argc == 1)
		printf("%ld\n", time((time_t *) 0));
	else {
		char	buf[BUFSIZ], *bptr = buf;

		*bptr = 0;
		while (--argc) {
			strcpy(bptr, *++argv);
			bptr += strlen(bptr);
			*bptr++ = ' ';
		}
		*--bptr = 0;
		printf("%ld\n", getdate(buf, (struct timeb *) 0));
	}
	exit(0);
}
