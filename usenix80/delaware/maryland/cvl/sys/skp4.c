main(argc,argv)
char *argv[];
{
	register i, j;
	register char *device;

	device = "/dev/nrw_rmt4";

	if(argv[0][3] == '0')
		device = "/dev/nrw_rmt0";

	if((argc < 2) || ((i = atoi(argv[1])) <= 0))
		i = 1;

	for(j=0;j<i;j++)
		close(open(device,0));
}
