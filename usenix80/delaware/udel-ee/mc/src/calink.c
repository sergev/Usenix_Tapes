main(argc,argv)
char **argv;
{
 register int i;
 int fildes1, fildes2, num;
 char buf[512];

 if (argc < 3) exit();
 fildes1 = open(argv[1],1);
 if (seek(fildes1,0,2) < 0)
    {
     fildes1 = creat(argv[1],0666);
     if (fildes1 < 0)
	{
	 printf("%s: can't open %s\n",argv[0],argv[1]);
	 exit(2);
	}
    }

 for (i=2; i<argc; i++)
    {
     if ((fildes2 = open(argv[i],0)) < 0)
	{
	 printf("%s: can't open input %s\n",argv[0],argv[i]);
	 exit(3);
	}

     while(1)
	{
	 num = read(fildes2,buf,512);
	 if (num <= 0) break;
	 write(fildes1,buf,num);
	}

     close(fildes2);
    }
}
