
char *source;		/* source file */
char *flowtab	"em1:flowtab";	/* flow file */

char	*table;		/* pointer to flow table */

int statbuf[18];	/* for size of file */

main(argc,argv) char *argv[];  {
	register line,fild,c;
	int ibuf[259];
	extern int fout[];

	if (argc<2) {
		printf("Usage: eflow sourcefile [flowtab file]\n");
		exit(-1);
	}
	source = argv[1];
	if (argc > 2)
		flowtab = argv[2];
	if ((fild=open(flowtab,0))<0) {
		printf("%s inaccessible\n",flowtab);
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
		printf("%5d",line);
		if (!((table[line>>3])&(1<<(line&07))))
			printf("*");
		printf("\t%c",c);
		while ((c=getc(ibuf))>=0 && c != '\n')
			printf("%c",c);
		printf("\n");
	}
	flush();
}
