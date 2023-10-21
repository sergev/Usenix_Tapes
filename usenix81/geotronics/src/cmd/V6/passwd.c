/*
 * passwd.c - enter a password in the password file
 *
 *	modified 01-Nov-1980 by D A Gwyn:
 *	1) V7 `crypt' now used.
 *
 * this program should be suid
 * with an owner with write permission on /etc/passwd
 */
char	*tfile	{ "/tmp/ptmp" };
char	*pfile	{ "/etc/passwd" };
int	tbuf[259];
int	pbuf[259];

char	saltab[64] =	{
	'a','A','b','B','c','C','d','D','e','E','f','F','g','G','h','H',
	'i','I','j','J','k','K','l','L','m','M','n','N','o','O','p','P',
	'q','Q','r','R','s','S','t','T','u','U','v','V','w','W','x','X',
	'y','Y','z','Z','0','1','2','3','4','5','6','7','8','9','.','/'
	};

main(argc, argv)
char *argv[];
{
	register u, c;
	register char *p;
	int	index[2];		/* used to index salt table */
	char	salt[2];

	if(argc != 3) {
		write(2, "Usage: passwd user password\n", 28);
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
	p = argv[1];
	while(c != ':') {
		if(*p++ != c)
			goto skip;
		c = getc(pbuf);
		putc(c, tbuf);
	}
	if(*p)
		goto skip;
/*
 * skip old password
 */
	do {
		c = getc(pbuf);
		if(c < 0)
			goto ill;
	} while(c != ':');

/*
 * copy in new password
 */
	p = argv[2];
	for(c=0; c<9; c++)
		if(*p++ == 0)
			break;
	*--p = 0;
	if(p != argv[2])
		{
		time( index );		/* sort of random */
		salt[0] = saltab[index[1] & 077];
		salt[1] = saltab[index[1]>>6 & 077];
		p = crypt( argv[2] , salt );
		}
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
	c = getuid() & 0377;
	if(c != 0 && c != u) {
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
