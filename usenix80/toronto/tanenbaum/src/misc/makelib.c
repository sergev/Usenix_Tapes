#define CCMAGIC	(172|(0<<8))
#define ARCCMAGIC (172|(1<<8))
#define ARGUARD (172|(2<<8))

#define MAXPROC 100
#define MAXGLO 100

char procs[MAXPROC][8],glos[MAXGLO][8];
int nprocs,nglos;
char ent[10];
int ibuf[259],obuf[259];
int libfil;

main(argc,argv) char **argv; {
	register char *p,*q;
	register n;
	
	if(argc<2||argc>3) {
		printf("Usage: makelib compact [ externals ]\n");
		exit(-1);
	}
	if (fopen(argv[1],ibuf)<0) {
		printf("Can't open %s\n",argv[1]);
		exit(-1);
	}
	if ((libfil=open(argc==3 ? argv[2] : "externals",0))<0) {
		printf("Can't open externals file\n");
		exit(-1);
	}
	obuf[0] = 1;
	obuf[1] = 0;
	obuf[2] = 0;
	if (getw(ibuf)!= CCMAGIC) {
		printf("Bad input file\n");
		exit(-1);
	}
	putw(ARCCMAGIC,obuf);
	while (read(libfil,ent,9)==9)
	  switch (ent[0]) {
	    case 'p':
		p = &ent[1];
		if (nprocs>=MAXPROC) {
			printf("Too many external procs");
			exit(-1);
		}
		q = procs[nprocs++];
		n = 8;
		while (*q++ = *p++)
			n--;
		while (n--)
			*q++ = 0;
		break;
	    case 'g':
		p = &ent[1];
		if (nglos>=MAXGLO) {
			printf("Too many external glos");
			exit(-1);
		}
		q = glos[nglos++];
		n = 8;
		while (*q++ = *p++)
			n--;
		while (n--)
			*q++ = 0;
		break;
	    default:
		printf("Bad character in externals file\n");
		exit(-1);
	  }

	/*
	 * end of the externals file
	 */
	putw(nglos,obuf);
	putw(nprocs,obuf);
	q = &glos[nglos];
	for (p= glos; p<q; p++)
		putc(*p,obuf);
	q = &procs[nprocs];
	for (p= procs; p<q; p++)
		putc(*p,obuf);
	putw(ARGUARD,obuf);
	while ((n=getc(ibuf))>=0)
		putc(n,obuf);
	fflush(obuf);
}
