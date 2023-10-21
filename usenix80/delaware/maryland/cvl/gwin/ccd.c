int a[16];
int in[512];
int out[512];
int flag;
int inc;
double sum;
main(argc,argv)
char *argv[];
{
	register i, j, k;

	if(argc > 1)
		inc = atoi(argv[1]);
	else
		inc = 2;
	flag = 0;
	if(argc > 2) flag++;
	gopen(a,0,0,512,512);

	for(i=0;i<512;i++) {
		if(flag)
			grrow(a,in,i,0,512,1);
		else
			grcol(a,in,i,0,512,1);
		for(j=0;j<512;j =+ inc) {
			sum = 0;
			for(k=0;k<inc;k++)
				sum =+ in[j+k];
			sum =/ inc;
			for(k=0;k<inc;k++)
				out[j+k] = sum;
		}
		if(flag)
			gwrow(a,out,i,0,512,1);
		else
			gwcol(a,out,i,0,512,1);
	}
}
