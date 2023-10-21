
/*
 *      s_con - mx tty connection server
 */

char	pty[]	"/dev/ptyX";
char	tty[]	"/dev/ttyX";
char	ttys[]	"%]:,{}@;\\ ~|";

int     fp,ft,pidl,pidc;
char    buf[512];
int     sttyb[3];

extern	par_uid;

main()
{
	char *cp,c;
	int i;

	mxserve(1);
	cp = ttys;
	while(c = *cp++) {
		pty[8] = tty[8] = c;
		if((fp=open(pty,2)) < 0)
			continue;
		goto copy;
	}
	ps("no pty's\n");
	exit();

copy:
	while((pidc=fork()) == -1)
		sleep(5);

	if(pidc)
		send();
	else
		recv();
}

send()
{
	register i;
	while((i=read(fp,buf,sizeof buf)) > 0 &&
		write(1,buf,i) == i);
	kill(pidc,9);
	exit();
}

recv()
{
	register i;
	while((i=read(0,buf,sizeof buf)) > 0 &&
		write(fp,buf,i) == i);
	kill(par_uid,9);
	exit();
}

ps(s)
char *s;
{
	do
		write(1,s,1);
	while(*++s);
}
