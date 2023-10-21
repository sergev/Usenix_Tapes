char copr[] "Copyright (c) 1975 Michael D.  Tilson";

/* not that anyone will want to steal it */
/* Revised Jan 9 1976 by Robert Pike */

main(argc,argv)
int argc;
char *argv[];
{
int tvec[2];
char *c;
	/* set signals in case we are the default shell */
	signal(1,0);
	signal(2,0);
	signal(3,1);
	if (argc>2)
	{
		printf("bad news arg\n");
		exit();
	}
	time(tvec);
	c=ctime(tvec);
	c[16]='\0';
	if(argc==1 || (argv[1][0]=='p' && argv[1][1]=='\0'))
	{
		printf("%s   ***  news  ***\n\n\n",c);
		execl("/bin/cat","/bin/cat","/case/usr/news/.mail",0);
	}
	if (argv[1][0]=='s' && argv[1][1]=='\0')
	{
		execl("/bin/mail","/bin/mail","news",0);
	}
	if (argv[1][0]=='o' && argv[1][1]=='\0')
	{
		printf("%s   ***  old news  ***\n\n\n",c);
		execl("/bin/cat","/bin/cat","/case/usr/news/mbox",0);
	}
	if (argv[1][0]=='h' && argv[1][1]=='\0')
	{
		printf("%s   ***  History  ***\n\n\n",c);
		execl("/bin/cat","/bin/cat","/case/usr/news/history",0);
	}

	printf("bad news arg\n");
	exit();
}
