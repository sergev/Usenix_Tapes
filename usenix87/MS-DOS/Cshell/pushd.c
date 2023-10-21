#define NULL (void *)0

static char *dirstack[10];
static int dsp = -1;

pushd(argc,argv)
char *argv[];
{
	char *getcwd(), *savestr();
	static char *usage = "usage : pushd newdir";
	static char *pusherr = "pushd : dir stack overflow";
	char dirbuf[64];
	if (argc == 1)
	{
		write(2,usage,strlen(usage));
		crlf();
		return -1;
	}
	if (NULL ==  getcwd(&dirbuf[1],64))
	{
		perror("pushd");
		return -1;
	}
	if (++dsp == 10)
	{
		write(2,pusherr,strlen(pusherr));
		crlf();
		return -1;
	}
	dirbuf[0] = '\\';
	if (-1 == chdir(*(++argv)))
	{
		--dsp;
		perror("pushd");
		return -1;
	}
	dirstack[dsp] = savestr(dirbuf);
	return 0;
}

popd()
{
	register int returnval = 0;
	static char *poperr = "popd : dir stack underflow";
	if (dsp == -1)
	{
		write(2,poperr,strlen(poperr));
		crlf();
		return -1;
	}
	if (-1 == chdir(dirstack[dsp]))
	{
		perror("popd");
		write(2,dirstack[dsp],strlen(dirstack[dsp]));
		crlf();
		returnval = -1;
	}
	free(dirstack[dsp--]);
	return returnval;
}
