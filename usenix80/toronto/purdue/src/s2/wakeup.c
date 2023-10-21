main(argc, argv)
char **argv;
{
	register wchan;
	register char *p;

	if (argc < 2) {
		write(2, "usage: wakeup ADDR\n", 19);
		exit();
	}
	wchan = 0;
	p = *++argv;
	while ('0' <= *p && *p <= '7')
		wchan = wchan * 8 + *p++ - '0';
	if (*p) {
		write(2, "bad octal digit\n", 16);
		exit();
	}
	if (wakeup(wchan))
		write(2, "wakeup: not super-user\n", 23);
}
