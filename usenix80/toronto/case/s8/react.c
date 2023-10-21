#
/*
 *      react -- program for maintenance of accounting files, etc.
 *      the name react is derived from the REACT program on TOPS-10
 *      which performs similar function.
 *
 *      current usage is to update and maintain two files for use
 *      by login in support of the restricted login feature.
 *
 *      future plans are to include the ability to update the
 *      password file by adding, deleting and changing entries,
 *      and to make the neccessary directories, etc required to
 *      install a new user.  Thus eliminating all the nasty business
 *      presently required and allow mere mortals to install users.
 *
 *      Author:  Dennis L. Mumaugh
 *      24 June 1977
 *
 *      React knows about two files.  One file is organized as
 *      one word (16 bits) per user. (512 bytes).  This file is
 *      used to specify user access priviliges. The bits and the
 *      meanings are specified in the defines below.
 *
 *      The other file consists of 4 bytes per terminal:
 *              one byte for terminal name
 *              one word for terminal class (uses same bits as above)
 *              one byte for padding
 *      This specifies the class of the terminal e.g. local.
 *
 *      The general behaviour of react is that it prompts for an option
 *      this is a single character:
 *           i[nitialize] file -- initialize non-zero entries to default
 *           p[rint] user information -- for a given user
 *           d[ump] file  -- all users as above
 *           t[erm] file  -- change terminal class
 *           e[nter] user -- change user access information
 *              options
 *                   a[dd] new access modes
 *                   d[elete] access modes
 *                   c[hange] access modes (d + a)
 *                   s[tandard] (set to default modes)
 *           n[ew user]  -- not implemented yet
 *           r[emove user] -- not implemented yet
 *           q[uit] -- exit
 *      Each option may give more options.
 *
 *      If a user id is required one can enter the decimal value or
 *      a user login name.
 *
 *      If a mode is required one can enter an octal number or the
 *      name of the mode, if another entry is possible it will
 *      prompt
 *              more:
 *
 *      At any point an illegal response of '?' will give listing of
 *      available options.
 *
 */

char react_id[] "~|^`react:\t\t1.3\t17 Oct 1977\n";

/*
 *      access rights definitions
 */
#define CONSOLE   01
#define LOCAL     02
#define SYSTEM    04
#define REMOTE   010
#define NETWORK  020
#define SPCL1    040
#define SPCL2   0100
#define SPCL3   0200
#define SPCL4   0300

#define DEFAULT (CONSOLE|LOCAL)

struct alist{
	int bits;
	char *type;};

struct alist list[] {
	CONSOLE,     "console",
	LOCAL,       "local",
	SYSTEM,      "system",
	REMOTE,      "remote",
	NETWORK,     "network",
	SPCL1,       "spcl1",
	SPCL2,       "spcl2",
	SPCL3,       "spcl3",
	SPCL4,       "spcl4",
	0,};


#define PASSWORD "/etc/passwd"
#define aclist	"/etc/usraccess"

int buff[256];
int fd;
char pbuf[128];
int uid, mode;
int oldmode;

main(argc,argv)
char **argv;
{
	char *sp;
	struct alist *lp;
	int fd1;
	int tfout;
	extern int fout;

	if( (fd = open(aclist,2)) <0 ) {perror(aclist); exit();}
loop:
	printf("Option: ");
	sp = gets();
	switch(*sp){
	case 0: /* end of file */
	case 'q':  /* quit */
		printf("done\n");
		exit();
	case 'p': /* print user information */
		uid = guser();
		gread();
		pruser();
		goto loop;
	case 'd': /* dump file information */
		gread();
		printf("Enter listing file name: ");
		sp = gets();
		fd1 = creat(sp,0644);
		tfout = fout;
		fout = fd1;
		for(uid=0; uid <256; uid++) {
			pruser();
			putchar('\n');
		}
		flush();
		close(fout);
		fout = tfout;
		flush();
		goto loop;
	case 'e':
		uid = guser();
		break;
	case 'i':
		gread();
		for(uid=0; uid <256; uid++) {
			if (buff[uid] == 0) buff[uid] = DEFAULT;
		}
		gwrite();
		goto loop;
	case 't':
		settty();
		goto loop;
	case 'r': /* remove user*/
		/* insert code to remove files, directories, etc of
		   specified user and to remove line from password file
		 */
		goto loop;
	case 'n': /* new user*/
		/* insert code to install files, directories, etc of
		   specified user and to install line into password file
		 */
		goto loop;
	default:
		printf("Options are: i[nitialize], p[rint], d[ump], t[erm], e[nter]\n");
		goto loop;
	}
/*
 *      for the enter option
 *      allows one to change a user's priviliges
 *
 */
	gread();
	oldmode = buff[uid];
loop1:
	pruser();
	printf("Action: ");
	sp = gets();
	switch(*sp){
	default:
		printf("Choice: a[dd], d[elete], c[hange], s[tandard]\n");
		goto loop1;
	case 'a':
		oldmode =| getmode();
		break;
	case 'd':
		oldmode =& ~getmode();
		break;
	case 'c':
		oldmode = getmode();
		break;
	case 's':
		oldmode = DEFAULT;
		break;
	}
	buff[uid] = oldmode;
	gwrite();
	goto loop;
}

getmode(){
	struct alist *lp;
	char *sp;

	gread();
	printf("Mode: ");
	mode = 0;
	sp = gets();
	if ( *sp >= '0' && *sp <= '9' ) mode = octal(sp);
	else {
loop:           ;
		for(lp = list; lp->bits; lp++) {
			if( equal(sp,lp->type) ) { 
				mode =| lp->bits; 
				goto loop1;
			}
		}
		printf("Modes are:\n");
		pmode(0177777);
		printf("\nMode: ");
		sp = gets();
		goto loop;
loop1:
		printf("more: ");
		sp = gets();
		if ( *sp == '\000') goto done;
		goto loop;
	}
done:
	return(mode);
}

guser(){
	char  *np, *cp, *sp;
	char pname[9];

loop:   
	printf("User: ");
	uid = 0;
	sp = gets();
	cp = sp;
	if ( '0' <= *sp && *sp <= '9' ) uid = atoi(sp);
	else {
		for(np = pname; np < pname +8 && *sp; np++) *np = *sp++;
		while (np < pname+8)
			*np++ = ' ';
		if (getpwentry(pname, pbuf))
		{
			printf("%s not a valid user name\n", cp);
			goto loop;
		}
		np = colon(pbuf);
		np = colon(np);
		while (*np != ':')
			uid = uid*10 + *np++ - '0';
	}
	return(uid);
}

puser(tuid)
int tuid;
{
	char *cp, *np;
	static char line[80];
	if(getpw(tuid,pbuf)){
		np = "unknown";
	}
	else{
		np = line;
		for(cp=pbuf;*cp!=':';)*np++ = *cp++; /* name */
		for(++cp;*cp!=':';++cp);            /* passwd */
		for(++cp;*cp !=':';++cp);       /* uid */
		for(++cp;*cp !=':';++cp);       /* gid */
		for(;np < line + 10; *np++ = ' ');
		cp++;
		while(*cp !=';' && *cp !=':'){
			*np++ = *cp++;
		}
		*np = '\0';
		np = line;
	}
	return(np);
}

getpwentry(name, buf)
char *name, *buf;
{
	extern fin;
	int fi, r, c;
	register char *gnp, *rnp;

	fi = fin;
	r = 1;
	if((fin = open(PASSWORD, 0)) < 0)
		goto ret;
loop:
	gnp = name;
	rnp = buf;
	while((c=getchar()) != '\n') {
		if(c <= 0)
			goto ret;
		*rnp++ = c;
	}
	*rnp++ = '\0';
	rnp = buf;
	while (*gnp++ == *rnp++);
	if ((*--gnp!=' ' && gnp<name+8) || *--rnp!=':')
		goto loop;
	r = 0;
ret:
	close(fin);
	fin = 0;
	(&fin)[1] = 0;
	(&fin)[2] = 0;
	return(r);
}

colon(p)
char *p;
{
	register char *rp;

	rp = p;
	while (*rp != ':') {
		if (*rp++ == '\0') {
			write(1, "Bad file format\n", 16);
			exit();
		}
	}
	*rp++ = '\0';
	return(rp);
}


gwrite(){
	seek(fd,0,0);
	write(fd,&buff[0],512);
}

gread(){
	seek(fd,0,0);
	read(fd,&buff[0],512);
}

octal(ap)
char *ap;
{
	register n;
	register char *p;

	p = ap;
	n = 0;
	while(*p == ' ' || *p == '\t')
		p++;
	while(*p >= '0' && *p <= '7')
		n = (n<<3) | (*p++ - '0');
	return(n);
}

#define ttyfile "/etc/ttyaccess"
settty(){
	int fd1;
	struct tlist{
		int bits;
		char ttyname;
		char nl;} ttylst;
	char *sp;

loop:   ;
	printf("Action: ");
	sp = gets();
	switch(*sp){
	case 'c':
	fd1 = open(ttyfile,2);
	if( fd1 < 0 ) {
		fd1 = creat(ttyfile,0644);
		close(fd1);
		fd1 = open(ttyfile,2);
	}
	printf("terminal letter: ");
	sp = gets();
	while( read(fd1,&ttylst, sizeof ttylst) > 0)
	if (ttylst.ttyname == *sp ) {
		seek(fd1,-(sizeof ttylst),1);
		printf("tty%c: ",ttylst.ttyname);
		pmode(ttylst.bits);
		putchar('\n');
		goto out;
	};
	ttylst.ttyname = *sp;
out:    ;
	ttylst.bits = getmode();
	ttylst.nl = '\n';
	write(fd1, &ttylst, sizeof ttylst);
	close(fd1);
	return(0);
	break;
	case 'p':
	fd1 = open(ttyfile,2);
	if( fd1 < 0 ) {
		perror(ttyfile);
		return(0);
	}
	while( read(fd1,&ttylst, sizeof ttylst) > 0) {
		printf("tty%c: ",ttylst.ttyname);
		pmode(ttylst.bits);
		putchar('\n');
	}
	return(0);
	default:
	printf("Options: c[hange] or p[rint]\n");
	goto loop;
	}
}

pmode(tmode){
	struct alist *lp;

		for(lp = list; lp->bits; lp++) {
			if( tmode&lp->bits ) {
				printf("%s ",lp->type);
			}
		}
}

pruser(){
		printf("%6.6d %s\n",uid,puser(uid));
		mode = buff[uid];
		printf( "mode: %6.6o\n",mode);
		pmode(mode);
		putchar('\n');
}
