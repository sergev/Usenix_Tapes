
static char *dir, *name, *tty;
static int inited;

char *logdir()
{
	if (!inited)
		if (init() < 0) return(0);
	return(dir);
}


char *logname()
{
	if (!inited)
		if (init() < 0) return(0);
	return(name);
}


static init()
{
	int uid, i;
	static char pwbuf[80];
	char *p;

	uid = getuid() & 0377;
	if(getpw(uid, pwbuf) != 0) {
		printf("can't read password file\n");
		return(-1);
	}
	p = name = pwbuf;
	for (i = 0; i < 5; i++) {
		while (*p++ != ':');
		*(p-1) = 0;
	}
	dir = p;
	while(*p++ != ':');
	*(p-1) = 0;
	inited++;
	return(0);
}


char *logtty()
{
	extern ttyn();

	if (tty) return(tty);
	tty = "x";
	tty[0] = ttyn(0);
	return(tty);
}


char *logename()
{
	int uid, i;
	static char pwbuf[80];
	char *p;

	uid = (getuid()>>8)&0377;
	if (getpw(uid, pwbuf) != 0) {
		printf("can't read password file\n");
		return(0);
	}
	for (p = pwbuf; *p != ':'; p++);
	*p = '\0';
	return(pwbuf);
}
