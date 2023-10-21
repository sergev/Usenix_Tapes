int fi, buf[256];
main(argc, argv)
char *argv[];
{
	register i, *j, lim;

	if(argc < 3)
		error("arg count");
	printf("%s:\n", argv[1]);
	if((fi = open(argv[1], 0)) < 0)
		error("open");
	lim = i = atoi(argv[2]);
	if(argc > 3)
		lim = atoi(argv[3]);
	if(seek(fi, i, 3) < 0)
		error("seek");
	for( ; i <= lim; i++) {
		printf("	block %u:\n", i);
		if(read(fi, buf, 512) < 512)
			error("read");
		for(j = buf; j < &buf[256]; ) {
			printf("0%o ", *j++);
			if((j - buf) % 10 == 0)
				printf("\n");
		}
		printf("\n");
	}
}
error(str)
{
	extern errno;
	printf("error %d:  %s\n", errno, str);
	exit();
}
