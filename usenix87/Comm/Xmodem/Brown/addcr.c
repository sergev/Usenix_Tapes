main(argc,argv)
int argc;
char *argv[]; {
	char buf[512];
	register int i;

	for (i = 1; i < argc; i++) {
		strcpy(buf,"mv ");
		strcat(buf,ADDCR(argv[i]));
		strcat(buf," ");
		strcat(buf,argv[i]);
		system(buf);
	}
}
