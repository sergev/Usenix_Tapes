#
#define OFFSET  20
int a[16];
char outr[OFFSET+86];
int row[520];
int tab[4098];
int vals[6] {1,2,4,8,16,32};
main(argc,argv)
{
	register i, j, k;
	int inc, f, huh;

	gopen(a,0,0,512,512);
	grtab(a,tab,0,4096);
	huh = 4095;
	if(argc != 1) huh = 0;
	if(argc > 2) {
		genter(a,4,0,0,0,0);
		tab[0] = tab[4097] = 0;
		tab[4096] = 4096;
	}
	else
		tab[4097] = huh;
	for(i=0;i<520;i++) row[i] = 4097;
	f = 1;
	write(f,"\n\n\n\n\n\n\n\n\n\n",10);
	for(j=0;j<OFFSET;j++) outr[j] = 0100;
	outr[OFFSET+84] = 5;
	outr[OFFSET+85] = '\n';
	for(i=479;i>=0;i--) {
		for(j=0;j<84;j++) outr[OFFSET+j] = 0100;
		grrow(a,row,i,0,512,1);
		inc = 0;
		for(j=0;j<84;j++) {
			for(k=0;k<6;k++) {
				if(tab[row[inc++]] != huh)
					outr[OFFSET+j] =| vals[k];
			}
		}
		write(f,outr,OFFSET+86);
	}
	write(f,"\014",1);
}
