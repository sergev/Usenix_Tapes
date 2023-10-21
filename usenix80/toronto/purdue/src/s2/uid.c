/*
 *	UID program -- Determines user with specified id or name
 *	or reports name and user-id of current user.
 * usage: uid 0123..........	octal uid specified
 *	  uid 123..........	decimal uid specified
 *	  uid name...........	user name specified
 * ie:	  uid reevesr ghg 014 032 408
 * will give the user names for each uid
 */ 

char buf[80];
int uid, euid;

main(argc, argv)
	char **argv;
{
	register j,k;
	char *ptr, *ptr2;
	extern fout;
	fout = dup(1);	/* warp drive */
	if(argc < 2) {
		uid = getuid();
		euid = geteuid();
		getpw(uid, buf);
		strip();
		print(buf);
		flush();
		exit();

	}
	while (--argc){
		argv++;
		if (**argv >= '0' && **argv <= '9'){
			uid = euid = aton(*argv);
			if (getpw(uid,buf) != 0)
				printf("UID %d: not assigned\n",uid);
			else{
				strip();print(buf);
			}
		}else{
			for (j=0;(*argv)[j] != '\0';j++)
				if (j >= 8){
					(*argv)[j] = '\0';
					break;
				}
			if (getpwn(*argv,buf) == -1){
		printf("%s: not found in passwd file\n",*argv);
				continue;
			}else
				ptr2 = buf;
			for (k=0;k < 2;k++)
				while (*ptr2++ != ':');
			j=0;
			while (*ptr2 != ':')
				j = j*10 + *ptr2++ -'0';
			uid = euid = j;
			print(*argv);
		}
	}
	flush();
}
strip() {
	register char *i;
	for(i = buf; i <= buf+80; i++) {
		if(*i == ':') {
			*i = '\0';
			return;
		}
	}
}
print(s) 
char *s;
{
	printf("UID = %d: %s\n", uid, s);
	if(euid != uid)
		printf("EID = %d\n", euid);
}
aton(arg)
char *arg;
{
	/*
	* convert a string to a number
	* if string started with 0 assume octal,
	* else it's decimal
	*/
	register char *c;
	register n, base;
	n=0;
	c = arg;
	base= *c =='0'?8:10;
	do
	n = (n * base) + (*c - '0');
	while('0' <= *(++c) && *c <= (base+'0'-1));
	return(n);
}
#
/*
 * getpwn(name, buf)
 *
 * Copies the password file entry for the given user name into buf.
 * On return buf is null-terminated and contains no newline character.
 *
 * Returns 0 if successful, -1 if not.
 *
 * Written by Ron Gomes as a modification of getpw -- April 1978.
 * modified for Purdue Unix, Nov 78, R. Reeves
 * fixed bug -kt 79
 *	
 * this routine uses the standard unix i/o system
 * rather than the new i/o package.
*/

#define ERROR	-1
#define OK	0

/* change def of PASSWORD to get a version of getpwn
 * that uses /etc/passwd or /etc/6passwd
 * then edit the program some.
 * if the seek file doesn't exist, getpwn
 * will try to use the standard password file (slowly)
 */
#define PASSWORD "/etc/passwd"
#define USEEK "/etc/u-seek"

getpwn(name, buf)
char *name, *buf;
{
	int pbuf[259];
	int pwf;
	int use;
	int nread;	/* number chars read when using u-seek */
	int uflg;	/* on if using useek file */
	register char *bp, *np;
	register char c;

	uflg++;
	if ((pwf = open(USEEK,0)) < 0) {
		if ((pwf = open(PASSWORD,0)) < 0)
			return(ERROR);
		else uflg = 0;
	}

	pbuf[0] = pwf;
	pbuf[1] = 0;
	pbuf[2] = 0;

	for (;;) {

		if (uflg) {

			/* use USEEK file */
			nread = read(pwf,buf,64);
			buf[63] = '\0';         /* make sure ended */
			if(nread != 64 || *buf == 0) {
				pwndone(pbuf,pwf,buf,ERROR);
				return(ERROR);
			}

		} else {

			/* use PASSWORD file */
			/* read in one line from password file */
			bp = buf;
			while((c=getc(pbuf)) != '\n') {
				if(c <= 0)
					{
					pwndone(pbuf,pwf,buf,ERROR);
					return(ERROR);
					}
				*bp++ = c;
			}
			*bp = '\0';

		}       /* end PASSWORD code */

		/* compare the name in the password line with
		* the desired user name 
		*/

		np = name;
		bp = buf;	/* point to password line buffer */
			use = 0;
		while (*bp != ':'){
			if (*bp++ != *np++){
				use++;
				break;
			}
		}
		if (use)
			continue;
		if(*np == '\0') {
			/* found */
			pwndone(pbuf,pwf,buf,OK);
			return(OK);
		}

		/* not found yet, if EOF then not there */

		if(*bp == '\0') {
			pwndone(pbuf,pwf,buf,ERROR);
			return(ERROR);
		}
	}
}
pwndone(pbuf,pwf,buf,err)
char *pbuf;     /* pointers to password file */
int pwf;        /* fd of password file */
char *buf;      /* buffer for line from password file */
int err;        /* -1 if error detected */
{
	register char *i;

/* clean up before exiting from getpwn */
	close(pwf);
	*pbuf++ = 0;
	*pbuf++ = 0;
	*pbuf = 0; 	/* clear out pointers */

/* if error, then clear out the buf field */
	i = buf;
	if (err == ERROR)
		while (*i != '\0')
			*i++ = '\0';

}
