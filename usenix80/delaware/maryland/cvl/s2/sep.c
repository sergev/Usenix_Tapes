int row[128];
char rrow[128];
main(argc,argv)
char *argv[];
{
	register i;
	register j;
	register shft;
	int in, out;
	shft = atoi(argv[1]);
	in = open(argv[2],0);
	out = creat(argv[3],0644);
	read(in,row,12);
	row[5] = 1;
	write(out,row,12);
	for(i=0;i<128;i++){
		read(in,row,256);
		for(j=0;j<128;j++)
			rrow[j] = (row[j] >> shft) & 15;
		write(out,rrow,128);
	}
}
