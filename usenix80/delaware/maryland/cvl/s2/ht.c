#
#define OFFSET  20
int a[16];
int row1[512];
int row2[512];
int crow[516];
int tab[4096];
int lp;
char orow[OFFSET+86];
int vals[6] {1,2,4,8,16,32};

main(argc, argv)
char *argv[];
{
	register x, error, *r1;
	int *r2, *tmp, y;

	lp = 1; /* Standard output */

	write(lp,"\n\n\n\n\n\n\n\n\n\n",10);

	gopen(a, 0, 0, 512, 512);
	grtab(a,tab,0,4096);

	crow[0] = crow[515] = crow[514] = crow[513] = crow[512] = crow[511] = 4032;
	orow[OFFSET+84] = 5; orow[OFFSET+85] = '\n';

	for(x=0;x<OFFSET;x++) orow[x] = 0100;

	r1 = row1;
	r2 = row2;

	grrow(a, r1, 479, 0, 512, 1);
	for(x=0;x<512;x++)
		r1[x] = tab[r1[x]];

	for(y=478;y>0;y--) {
		grrow(a, r2, y, 0, 512, 1);
		for(x=0;x<512;x++)
			r2[x] = tab[r2[x]];

		for(x=1;x<511;x++) {
			crow[x] = r1[x] < 2016 ? 0 : 4032;
			error = r1[x] - crow[x];
			r1[x+1] =+ (error * 7) >> 4;
			r2[x-1] =+ (error * 3) >> 4;
			r2[ x ] =+ (error * 5) >> 4;
			r2[x+1] =+ error >> 4;
		}
		prow();

		tmp = r1;
		r1 = r2;
		r2 = tmp;
	}
	write(lp,"\n\014",2);
}

prow()
{
	register i, j, *v;

	v = vals;

	for(i=0;i<84;i++) {
		orow[OFFSET+i] = 0100;
		for(j=0;j<6;j++) {
			if(crow[i*6+j] == 0)
				orow[OFFSET+i] =| v[j];
		}
	}

	write(lp, orow, OFFSET+86);
}
