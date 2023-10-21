#

/*
 *      csh - connected shell
 *
 *      csh <host> [-l <user> <pass>] "<commands>"
 */

#include <stdio.h>


char    user[9];
char    pass[16];
char    *host;
char    command[128],buf[512];
long    rbytes,xbytes;
char    lsw,args;
int     xf,pid;


main(argc,argv)
char **argv;
{
	register char *cp;
	int lbuf;

	host = argv[1];
	argv =+ 2;  argc =- 2;
	if(argc <= 0)
		syntax();
	cp = command;
	while(argc-- > 0) {
		if(args == 0 && **argv == '-') {
			switch(*++*argv++) {
			case 'l':
				lsw++;
				if((argc =- 2) <= 0 ||
					strlen(argv[0]) >= sizeof user ||
					strlen(argv[1]) >= sizeof pass)syntax();
				strcpy(user,*argv++);
				strcpy(pass,*argv);
				while(**argv)
					*(*argv)++ = 'x'; /* cover pass */
				argv++;
				break;

			default:
				syntax();
			}
		}else{
			args++;
			while(**argv) {
				if(cp+1 >= &command[sizeof command])
					mes(1,"csh: too long:\n%s\n",
						command);
				*cp++ = *(*argv)++;
			}
			*cp++ = ' ';
			argv++;
		}
	}
	if((xf = mxfile()) < 0)
		mes(1,"csh: ran out of /dev/mx\n");
	if(mxscon(xf,host,2) < 0)
		mes(1,"csh: can't connect to host \"%s\"\n",host);
	lbuf = catbuf(user,"\n",pass,"\n",command,"\n",0);
	write(xf,buf,lbuf);

	while((pid = fork()) == -1)
		sleep(1);
	if(pid)
		recv();
	else
		xmit();
}

xmit()
{
	register i,j;
	int xmitx();
	close(1);
	signal(4,xmitx);
	while((i=read(0,buf,sizeof buf)) > 0 &&
		(j=write(xf,buf,i)) == i) xbytes =+ i;
	if(i > 0 && j != i)
		mes(-1,"csh: WARNING send truncated\n");
	mxeof(xf);
	xmitx();
}

recv()
{
	extern quit();
	register i;
	close(0);
	signal(1,quit);
	if(signal(2,quit) == 1) {
		signal(2,1);
	} else {
		signal(2,quit);
		signal(3,quit);
	}
	while((i=read(xf,buf,sizeof buf)) > 0 &&
		write(1,buf,i) == i) rbytes =+ i;
	kill(pid,4);
	wait();
	if(i != 0)
		mes(1,"csh: WARNING receive truncated\n");
	mes(0,"csh: %D bytes received\n",rbytes);
}

xmitx()
{
	mes(1,"csh: %D bytes sent\n",xbytes);
}

syntax()
{
	mes(1,"csh: syntax: csh <hostname> [-l <user> <pass>] \"<commands>\"\n");
}

quit()
{
	mxsig(xf,1);
	mes(1,"csh: INTERRUPT - hangup sent to remote process group\n");
}

mes(f,a1,a2,a3,a4)
{
	fprintf(stderr,a1,a2,a3,a4);
	if(f>=0) exit(f);
}

catbuf(a)
char *a;
{
	register char **sp,*cp,*dp;
	sp = &a;
	dp = buf;
	while(cp = *sp++) {
		while(*dp++ = *cp++);
		dp--;
	}
	return(dp-buf);
}
