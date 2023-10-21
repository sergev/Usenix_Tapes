
/*
 *      s_csh - csh server
 */


/*
 * Listen for a connection on socket number 2.  When one arrives there
 * are three items (lines) waiting to be read: name, password, and
 * command line.  The name line is read first;  if non-null,
 * "/etc/l_csh l_csh name" is invoked which does the (lengthy) job of
 * looking up and verifying the password (still in the kernel net
 * buffer), and execing a shell with the command line.   When the
 * name and pass are null (the usual case), this can be avoided and the
 * command execed immediately.  To further speed things up, we try
 * to avoid calling the shell if no special characters are in the
 * command line.
 *
 * All the children of this process are placed in a separate process
 * group so that they may be send a "hangup" from the remote csh.
 */


char    name[16];
char    command[256];
int     useruid 13;
char    userdir[]       "/usr/user";
char    shellchar[]     "|^]\\[?><;:*)('&\"";   /* must be descending */
char    *argv[128],**argp;
char    c1;


main()
{
	register char *sp,*cp,c;

	signal(1,0);
	mxserve(2);
	mxpgrp(0);

	cp = name;
	while(read(0,&c1,1) == 1) {
		*cp++ = c1;
		if(c1 == '\n') break;
	}
	if(name[0] != '\n')
		execl("/etc/l_csh","l_csh",name,0);

	read(0,&c1,1);          /* skip password */
	if(read(0,command,sizeof command) <= 0) exit();
	setuid(useruid);
	for(cp=command ; c = *cp++ ;) {
		for(sp=shellchar ; *sp > c ; sp++);
		if(*sp == c) goto needshell;
	}
	argp = &argv[0];
	c = 0;
	for(cp=command ; *cp ; cp++) {
		if(*cp == ' ' || *cp == '\n'){
			*cp = c = 0;
			continue;
		}
		if(!c) {
			*argp++ = cp;
			c++;
		}
	}
	if(*argv[0] == '/') execv(argv[0],argv);
	execv(cats("/bin/",argv[0]),argv);
	execv(cats("/usr/bin/",argv[0]),argv);
	while(--cp > command)
		if(!*cp)
			*cp = ' ';

needshell:
	execl("/bin/sh","sh_csh","-c",command,0);
	exit();
}

cats(a,b)
register char *a,*b;
{
	static char s[32];
	register char *sp;
	sp = s;
	while(*sp++ = *a++);
	sp--;
	while(*sp++ = *b++);
	return(s);
}
