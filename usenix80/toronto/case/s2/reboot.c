/*
 * reboot command
 */

main(argc,argv)
char **argv;
{
	register int i;

	if ((getgid()&0377) > 9) {
		printf("reboot: not priviledged enough\n");
		exit(-1);
	}
	if(argc == 1)		/* default system */
		reboot(-1);
	if(*argv[1] == '-')		/* ask the console */
		reboot(0);
	for(i=0; i<18; i++)
		if(argv[1][i] == '\0') {
			argv[1][i] = '\n';		/* end string with nl */
			reboot(argv[1]);
		}

	printf("File name too long: %s\n",argv[1]);
}
