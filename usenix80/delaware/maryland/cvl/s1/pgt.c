int a[16];
char outr[90];
int row[520];
int vals[6] {1,2,4,8,16,32};
main(argc,argv)
char *argv[];
{
	register i, j, k;
	int inc, f, huh;

	huh = 1;
	if(argc != 1) huh = (1 << atoi(argv[1])) & 4095;
	f = open("/dev/lp",1);
	write(f,"\n\n\n\n",4);
	gopen(a,0,0,512,512);
	outr[88] = 5;
	outr[89] = '\n';
	for(i=0;i<88;i++) outr[i] = 0152;
	write(f,outr,90);
	outr[0] = 0140;
	outr[87] = 0140;
	for(i=511;i>=0;i--) {
		for(j=0;j<520;j++) row[j] = huh;
		for(j=1;j<87;j++) outr[j] = 0100;
		gwrow(a,row,i,0,512,1);
		grrow(a,row,i,0,512,1);
		inc = 0;
		for(j=0;j<86;j++) {
			for(k=0;k<6;k++) {
				if((row[inc++] & huh) != huh)
					outr[j+1] =| vals[k];
			}
		}
		write(f,outr,90);
		if(outr[0] == 0140)
			outr[0] = outr[87] = 0100;
		else
			outr[0] = outr[87] = 0140;

	}
	for(i=0;i<88;i++) outr[i] = 0152;
	write(f,outr,90);
	write(f,"\014",1);
}
