/*
 *	Set user-id to any value
 *
 * This program can be run by super-user to start up a shell with
 * a specified user id.  The syntax is either:
 *
 * # suid <num>
 *
 * where "num" is a numeric uid, or
 *
 * # suid <name>
 *
 * where "name" is a login name.  If this second format is used,
 * the uid is reported just before the shell starts up.
 *
 * An illegal login name will cause a failure message to be printed.
 */

main(argc, argv)
char **argv;
{
	register j, k;
	char name[9];
	char buf[80];

	if (getuid() != 0){
		printf("suid: not super-user\n");
		exit();
	}
	if (argc < 2){
		printf("usage: suid <UID or UNAME>\n");
		exit();
	}

	if (argv[1][0] >= '0' && argv[1][0] <= '9'){
		j = atoi(argv[1]);
		goto change;
	}

	j = 0;
	for(k=0; k<8; k++)
		if (j || argv[1][k] == 0){
			j++;
			name[k] = ' ';
		} else name[k] = argv[1][k];
	name[8] = 0;

	if (getpwentry(name, buf)){
		printf("Unrecognized login name \"%s\"\n", name);
		exit();
	}

	j = k = 0;
	while(buf[k++] != ':');
	while(buf[k++] != ':');
	while(buf[k++] != ':') j = j*10 + buf[k-1] - '0';

	printf("UID = %d\n", j);
change:
	if (setuid(j) < 0){
		printf("Can't do \"setuid\"\n");
		exit();
	}
	execl("/bin/sh", argc == 3 ? "-":"sh", 0);
	printf("No shell!\n");
}

getpwentry(name, buf)
char *name, *buf;
{
	extern fin;
	int fi, r, c;
	register char *gnp, *rnp;

	fi = fin;
	r = 1;
	if ((fin = open("/etc/passwd", 0)) < 0)
		goto ret;
loop:
	gnp = name;
	rnp = buf;
	while ((c=getchar()) != '\n') {
		if (c <= 0)
			goto ret;
		*rnp++ = c;
	}
	*rnp++ = '\0';
	rnp = buf;
	while (*gnp++ == *rnp++);
	if ((*--gnp!=' ' && gnp<name+8) || *--rnp!=':')
		goto loop;
	r = 0;
ret:
	close(fin);
	fin = 0;
	(&fin)[1] = 0;
	(&fin)[2] = 0;
	return(r);
}
