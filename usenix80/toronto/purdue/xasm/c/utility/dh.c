main(argc,argv)
	int argc;	char **argv;
{
	int dh0, dh1, dh2, dh3, dh4;
	int fd;
	register lock;
	register bcount;
	register i;

	if((fd=open("/dev/kmem",0)) < 0) {
		printf("can't open /dev/kmem for read\n");
		exit();
	}
	i = 0;

	if(argc == 2){
		i = basin(argv[1],10);
		}
	lock = 1;
	while(1){
		bcount = 0;
		seek(fd, 0160032, 0);
		read(fd, &dh0, 2);
		seek(fd, 0160052, 0);
		read(fd, &dh1, 2);
		seek(fd, 0160072, 0);
		read(fd, &dh2, 2);
		seek(fd, 0160112, 0);
		read(fd, &dh3, 2);
		seek(fd, 0160132, 0);
		read(fd, &dh4, 2);

		bcount =+ cbits(dh0);
		bcount =+ cbits(dh1);
		bcount =+ cbits(dh2);
		bcount =+ cbits(dh3);
		bcount =+ cbits(dh4);

	    if(lock){
		printf("DH0\tDH1\tDH2\tDH3\tDH4\n");
		lock = 0;
		}
	printf("%o\t%o\t%o\t%o\t%o\t",dh0,dh1,dh2,dh3,dh4);
	printf("%d xfers in progress.\n",bcount);
	if(argc == 1)goto done;
	if(i != 0)sleep(i);
	}
done:	i = 0;
}

cbits(thing)
	int thing;
{
	register mask,n,i;

	mask = 1;	n = 0;
	for(i = 0;i != 16;i++){
		if(mask & thing)n++;
		mask =<< 1;
		}
	return(n);
}
