
char *source;		/* source file */
char *profile	"em1:profile";	/* pro file */

struct {int hi,lo;} *table;

int statbuf[18];	/* for size of file */

main(argc,argv) char *argv[];  {
	register line,fild,c;
	int ibuf[259];
	extern int fout[];

	if (argc<2) {
		printf("Usage: eprof sourcefile [profile file]\n");
		exit(-1);
	}
	source = argv[1];
	if (argc > 2)
		profile = argv[2];
	if ((fild=open(profile,0))<0) {
		printf("%s inaccessible\n",profile);
		exit(-1);
	}
	fstat(fild,statbuf);
	table = sbrk(statbuf[5]);
	read(fild,table,statbuf[5]);
	close(fild);
	if (fopen(source,ibuf)<0) {
		printf("Can't open %s\n",source);
		exit(-1);
	}
	line = 0;
	fout[0] = dup(1);
	while ((c=getc(ibuf))>=0) {
		line++;
		if (c == '\n') {
			printf("\n");
			continue;
		}
		if (table[line].lo != 0 || table[line].hi != 0)
			printf("%7s",locv(table[line].hi,table[line].lo));
		printf("\t%c",c);
		while ((c=getc(ibuf))>=0 && c != '\n')
			printf("%c",c);
		printf("\n");
	}
	flush();
}
