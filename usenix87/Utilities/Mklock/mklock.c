/* mklock.c to create .lock file */

#include <stdio.h>
main(argc,argv) int argc;
char *argv[];
{
	int f;
	if (argc != 2) {
		fprintf(stderr,"%s: exactly one argument expected\n",argv[0]);
		exit(1);
		}
	if (strcmp(argv[1],"-help") == 0) {
		fprintf(stderr,"Usage: %s file\n",argv[0]);
		exit(1);
		}
	if ((f = creat(argv[1],0)) < 0) {
		fprintf(stderr,"%s: ",argv[0]);
		perror("");
		exit(1);
		}
	else {
		close(f);
		exit(0);
		}
}
