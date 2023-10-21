#
/*
**      TO -- Interface to make Vanilla Unix "mail" prog look like "to".
** Compile: cc -O -q to.c -o to
**      (c) psl 1979
**
** Calling sequence:   to username -Ssubject -Ttext   (NO error checking)
*/

main(argc, argv)
char    *argv[];
{
	int pfh[2];

	pipe(pfh);
	close(0);                     /* this fails for VERY long messages */
	dup(pfh[0]);
	close(1);
	dup(pfh[1]);
	close(pfh[0]);
	close(pfh[1]);
	printf("Subject: %s\n%s\n", &argv[2][2], &argv[3][2]);
	close(1);
	execl("/bin/mail", "mail", argv[1], 0);
	printf("\"/bin/mail\" execl failed\n");
	exit(3);
}
