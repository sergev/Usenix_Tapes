main(argc,argp)
char **argp;
{
	int fd,count;
	char buf[512];

	if(argc <= 1)
		fd = 0;
	else {
		if((fd = open(argp[1],0)) < 0) {
			printf("can\'t open %s\n",argp[1]);
			exit(1);
		}

		if (argc > 2) seek(fd, 0, 2);
	}

	for(;;)  {
		while((count=read(fd,buf,512))>0) 
			write(1,buf,count);
		sleep(4);
	}
}
