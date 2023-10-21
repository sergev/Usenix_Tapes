main()
{
	int dh0, dh1, dh2, dh3, dh4;
	int fd;
	register nsamples;
	register bcount;
	register i;

	if((fd=open("/dev/kmem",0)) < 0) {
		printf("can't open /dev/kmem for read\n");
		exit();
	}

	nsamples = bcount = 0;
	while(1){
		seek(fd, 0160032, 0);
		read(fd, &dh0, 2);
		seek(fd, 0160052, 0);
		read(fd, &dh1, 2);
		seek(fd, 0160072, 0);
		read(fd, &dh2, 2);
		seek(fd, 0160112, 0);
		read(fd, &dh3, 2);
/*	ignore dh4
		seek(fd, 0160132, 0);
		read(fd, &dh4, 2);
*/

		nsamples++;
	if((nsamples % 6)== 0)printf("%o, %o, %o, %o\t",dh0,dh1,dh2,dh3);
		bcount =+ cbits(dh0);
		bcount =+ cbits(dh1);
		bcount =+ cbits(dh2);
		bcount =+ cbits(dh3);
/* ignore dh4
		bcount =+ cbits(dh4);
*/
	if((nsamples % 6) == 0)printf("With %d samples, I have %d bits.\n",nsamples,bcount);
		sleep(3);
		}
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
