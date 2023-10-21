
invalid(argc,argv)
	char *argv[];
{
	register int i;
	static char *invmsg = "sh : bad command : ";
	write(2,invmsg,strlen(invmsg));
	for (i = 0; i < argc;i++)
	{
		write(2,argv[i],strlen(argv[i]));
		write(2," ",1);
	}
	return -1;
}
