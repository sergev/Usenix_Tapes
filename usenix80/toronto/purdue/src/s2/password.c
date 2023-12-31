#
/*
 * enter a password in the password file
 * this program should be suid with owner
 * with an owner with write permission on /etc/passwd
 * the old password must be given as an argument for security
 */

#define	GUEST_UID	13	/* guest account "user" */
				/* not allowed to change password */
char	*tfile	{ "/tmp/ptmp" };
char	*pfile	{ "/etc/6passwd" };
int	tbuf[259];
int	pbuf[259];

main(argc, argv)
char *argv[];
{
	int i;
	register u, c;
	register char *p;
	char *tmp,kludge[96];
	int showps();

	showps(0);
	if(argc != 3) {
		printf("Usage: password old-password new-password\n");
		goto bex;
	}
	signal(1, 1);
	signal(2, 1);
	signal(3, 1);

	if(stat(tfile, tbuf+20) >= 0) {
		write(2, "Temporary file busy -- try again\n", 33);
		goto bex;
	}
	tbuf[0] = creat(tfile, 0600);
	if(tbuf[0] < 0) {
		write(2, "Cannot create temporary file\n", 29);
		goto bex;
	}
	pbuf[0] = open(pfile, 0);
	if(pbuf[0] < 0) {
		write(2, "Cannot open /etc/passwd\n", 24);
		goto out;
	}
	getpw(getuid(),kludge);
	tmp=kludge;
	while(*tmp != ':') 
		tmp++;
	*tmp++=0;
	tmp=kludge;
	goto l1;

/*
 * skip to beginning of next line
 */

skip:
	while(c != '\n') {
		if(c < 0)
			goto ill;
		c = getc(pbuf);
		putc(c, tbuf);
	}

/*
 * compare user names
 */

l1:
	c = getc(pbuf);
	putc(c, tbuf);
	if(c < 0) {
		write(2, "User name not found in password file\n", 37);
		goto out;
	}
	p = tmp;
	while(c != ':') {
		if(*p++ != c)
			goto skip;
		c = getc(pbuf);
		putc(c, tbuf);
	}
	if(*p)
		goto skip;
/*
 * check old password for validity
 */
	if((c=getc(pbuf)) == ':')
		goto new;
	p = argv[1];	/* get old password, truncate to 8 chars */
	while(*p != '\0') {
		if(p - argv[1] > 7)  {
			*p = '\0';
			break;
		}
		*p++;
	}
	tmp=crypt(argv[1]);
	for(u=0;u < 8;u++)
		if(*tmp++ != c){
			printf("Permission denied\n");
			goto out;
		}
		else
			c=getc(pbuf);

/*
 * copy in new password
 */
new:	p = argv[2];
	for(c=0; c<9; c++)
		if(*p++ == 0)
			break;
	*--p = 0;
	if(p != argv[2])
		p = crypt(argv[2]);
	while(*p)
		putc(*p++, tbuf);
	putc(':', tbuf);

/*
 * validate uid
 */

	u = 0;
	do {
		c = getc(pbuf);
		putc(c, tbuf);
		if(c >= '0' && c <= '9')
			u = u*10 + c-'0';
		if(c < 0)
			goto ill;
	} while(c != ':');
	c = getuid();
	if(c != 0 && (c == GUEST_UID | c != u)) {
		write(2, "Permission denied\n", 18);
		goto out;
	}

/*
 * copy out and back
 */

	for(;;) {
		c = getc(pbuf);
		if(c < 0) {
			fflush(tbuf);
			close(pbuf[0]);
			close(tbuf[0]);
			tbuf[0] = open(tfile, 0);
			if(tbuf[0] < 0) {
				write(2, "Urk\n", 4);
				goto out;
			}
			pbuf[0] = creat(pfile, 0644);
			if(pbuf[0] < 0) {
				write(2, "Cannot create /etc/passwd\n", 26);
				goto out;
			}
			while((c = read(tbuf[0], tbuf+1, 512)) > 0)
				write(pbuf[0], tbuf+1, c);
			unlink(tfile);
			exit(0);
		}
		putc(c, tbuf);
	}

ill:
	write(2, "Password file illformed\n", 24);

out:
	unlink(tfile);

bex:
	exit(1);
}
