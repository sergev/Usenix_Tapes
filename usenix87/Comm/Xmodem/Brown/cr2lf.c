main(argc,argv)
int argc;
char *argv[]; {
	char buf[512];
	register int i;

	for (i = 1; i < argc; i++) {
		CR2LF(argv[i]);
	}
}
