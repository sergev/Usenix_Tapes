main(argc, argv)
int argc;
char *argv[];
{
	register cnt;
	register int *ip;
	int in, out, iblk, oblk;
	int buffer[256];

	if(argc != 5 && argc != 6) {
		printf("Usage: cpblk file1 block file2 block [count]\n");
		exit(1);
	}

	in = open(argv[1], 0);
	if ((out=open(argv[3], 2)) < 0)
		out = creat(argv[3], 0644);
	if(in < 0) {
		printf("Can't read from %s\n", argv[1]);
		exit(1);
	}
	if(out < 0) {
		printf("Can't create %s\n", argv[3]);
		exit(1);
	}

	if((*argv[2] < '0' || *argv[2] > '9') || (*argv[4] < '0' || 
	    *argv[4] > '9')) {
		printf("Bad block number\n");
		exit(1);
	}

	iblk = atoi(argv[2]);
	oblk = atoi(argv[4]);
	cnt = 1;

	if(argc == 6)
		if(*argv[5] < '0' || *argv[5] > '9') {
			printf("Bad block count\n");
			exit(1);
		}
		else
			cnt = atoi(argv[5]);

	seek(in, iblk, 3);
	seek(out, oblk, 3);
	while(cnt--) {
		/*
		 * clear buffer in case I/O error on raw read
		 * gets partial count
		 */

		for (ip = &buffer; ip < &buffer[256]; *ip++ = 0);
		if(read(in, buffer, 512) != 512) {
			printf("Read error on %s\n", argv[1]);
		}
		if(write(out, buffer, 512) != 512) {
			printf("Write error on %s\n", argv[3]);
		}
	}
}
