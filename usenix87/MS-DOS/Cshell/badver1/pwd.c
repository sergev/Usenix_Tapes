#define NULL (void *)0
pwd(argc,argv)
char *argv[];
{
	char *getcwd();
	register char *dir;
	if (!(dir = getcwd(NULL,256)))
	{
		perror("pwd");
		return -1;
	}
	write(1,"\\",1);
	write(1,dir,strlen(dir));
	crlf();
	if (dir)
		free(dir);
	return 0;
}
