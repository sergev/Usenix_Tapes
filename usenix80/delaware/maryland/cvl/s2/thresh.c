int a[16];
int buf[4096];
int t;
rubout()
{
	printf("Twelve bit threshold = %d\n",t << 3);
	printf("Nine bit threshold = %d\n",t);
	printf("Eight bit threshold = %d\n",t >> 1);
	printf("Six bit threshold = %d\n",t >> 3);
	exit();
}
main(argc, argv)
char *argv[];
{
	register i, j, v1;
	int v2;

	v1 = 4095;
	v2 = 0;
	if(argc > 1) {
		v1 = atoi(argv[1]);
		if(argc > 2)
			v2 = atoi(argv[2]);
	}
	signal(2,rubout);
	gopen(a,0,0,512,512);
	for(;;) {
		grcur(a,buf,1);
		i = (t = buf[0]) * 8;
		for(j=0;j<i;j++) buf[j] = v2;
		for(j=i;j<4096;j++) buf[j] = v1;
		gwtab(a,buf,0,4096);
	}
}
