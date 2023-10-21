int a[16];
main(argc,argv)
char *argv[];
{
	register nfrms, shft;

	if(argc == 1) {
		nfrms = 0;
		shft = 0;
	}
	else if(argc != 3) {
		printf("\nUSAGE --- freeze [nfrms shift]\n\n");
		exit();
	}
	else {
		nfrms = atoi(argv[1]);
		shft = atoi(argv[2]);
	}

	gopen(a,0,0,512,512);
	gcam(a,nfrms,shft);
}
