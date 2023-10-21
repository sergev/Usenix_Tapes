#
#define		LOCK		"/tmp/slock"
struct
{
	char	plname;
	char	pltype;
	int	plpid;
	int	pluid;
}
	fileform;
main()
{
register int	i;
if((i = open(LOCK,2)) < 0)
{
	printf("Cannot open lock file\n");
	exit();
};
while(read(i,&fileform,sizeof fileform) == sizeof fileform)
{
	kill(fileform.plpid,9);
	printf("Killed: %d\n",fileform.plpid);
};
unlink(LOCK);
}
