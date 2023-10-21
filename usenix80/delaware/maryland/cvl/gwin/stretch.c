int a[16];
int tab[4096];
int lp;
int hp;
int ld;
int hd;
int npsteps;
int ndsteps;
int num;
int shift;
int value;
double stepinc;
double dvalue;
main(argc,argv)
char *argv[];
{
	register i, j, tloc;

	if(argc != 3) {
		printf("\n\tUSAGE --- stretch low high\n\n");
		exit();
	}

	shift = 6;

	lp = atoi(argv[1]);
	hp = atoi(argv[2]);

	ld = 0;
	hd = (1 << shift) - 1;

	npsteps = hp - lp + 1;
	ndsteps = hd - ld + 2; /* 2 so the entire range is mapped */
	stepinc = (ndsteps + 0.0) / (npsteps + 0.0);

	j = lp << shift;
	for(i=0;i<j;i++)
		tab[i] = 0;

	dvalue = 0.0;
	num = 4096 / (hd + 1);
	for(i=lp;i<=hp;i++) {
		tloc = i << shift;
		value = dvalue;
		value =<< shift;
		for(j=0;j<num;j++)
			tab[tloc+j] = value;
		dvalue =+ stepinc;
	}
	for(i=tloc+j;i<4096;i++)
		tab[i] = 4095;

	gopen(a,0,0,512,512);
	gwtab(a,tab,0,4096);
}
