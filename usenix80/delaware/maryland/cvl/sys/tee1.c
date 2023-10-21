#
#define LEN 512

char buff[LEN];
char *argv0;

main(argc, argv)
int	argc;
char	*argv[];
{
	register int len, result, out;

	argv0 = argv[0];
	if((out = creat(argv[1], 0664)) <= 0)
		errprnt("cannot creat %s", argv[1]);
	while((len = readnl(0, buff, LEN)) > 0) {
		if((result = write(out, buff, len)) != len)
			errprnt("write error on %s", argv[1]);
		if((result = write(1, buff, len)) != len)
			errprnt("write error %d", result);
	}
	if(len != 0) errprnt("read error %d", len);
}
