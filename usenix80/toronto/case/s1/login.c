#
/*
 * login [ name ]
 */

#include "login.h"
#ifdef RESTRICT
#include "access.h"
#endif
/*
 * Case Dependencies
 */
#define THREE_DAYS	259200L		/* 3*24*60*60 */
#define NOBODY		99
#define NEWS		"/case/usr/news/.mail"
#define DEFSHELL	"/bin/sh"
char id[] "~|^` login:  2.0     31-Jan-79";

/*
 *	University of Illinos login program modified to run on
 *	Case Ecmp Unix system; below are the comments as found
 *	in the original UI login.c - many don't apply due to
 *	differences in system:
 *
 *      1.2 14-Sep-76  Dennis L. Mumaugh
 *      Test to see if there are any mail messages since the last
 *      time the user read his mail. Checks for a file .msgtime
 *      in login directory. If the modtime of the .mail file is
 *      later than the modtime of the .msgtime file the there
 *      is new mail (almost always).
 *
 *      1.3 14-Jan-77  Dennis L. Mumaugh
 *      Test file .llog to see if /etc/motd has changed since
 *      last login -- if not don't bother to print it.
 *      Prints time for previous login.
 *
 *      1.4 15-Feb-77  Dennis L. Mumaugh
 *      If /shutsw exist and not a local terminal refuse login.
 *
 *      1.5 4-May-77   Dennis L. Mumaugh
 *      Ask for password even if the login name is bad.
 *
 *      1.6 24-June-77  Dennis L. Mumaugh
 *      Restrict logins by user vice terminal
 *      Shut off logins to remote terminals when /shutsw exists
 *      Default .profile is run if exists
 *
 *      1.7 09-Nov-77  Dennis L. Mumaugh
 *      Use information provided by ksys if possible
 *      and give warning of impending shutdown (doom)
 *
 *      1.8 10-Mar-78  Dennis L. Mumaugh
 *      Fix path name of today
 *
 *	2.0 31-Jan-79 Sam Leffler
 *	Modify to work on Case Ecmp Unix.
 *	Changes involve Nobody shell needs,
 *	no .msgtime file,
 *	no today program,
 *	no need for profile shell as case shell does this
 *	 automatically,
 *	handle group restricted hours,
 *	no default character erase/kill chars (see getty),
 *	change modtime setup to longs in utmp and ibuf structs,
 *	change .llog file creation to be done only
 *	 if user starts up the process (i.e. no .llog files
 *	 will be created unless one already exists).
 */

/*
 *      for the restricted login feature to be implemented one must define
 *      a define of RESTRICT
 */

long tvec;

struct {
	char	name[8];
	char	tty;
	char	ifill;
	long	time;
	int	ufill;
} 
utmp;

struct {
	int	speeds;
	char	erase, kill;
	int	tflags;
} 
ttyb;


struct  inode        /* RAND */
{
	char	minor;
	char	major;
	int	inumber;
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size1;
	int	addr[8];
	unsigned actime[2];
	long	modtime;
};
char	*ttyx;

struct ksys ksys;
char tsmsg1[] "Time sharing will cease in %d minutes\n";
#define	ECHO	010

char logbuf[1]; /* RAND--its mod time is last login time */
main(argc, argv)
char **argv;
{
	int cty;
	char mbuf[32];  /* RAND -- buffer for /etc/motd */
	char pbuf[128];
	register char *namep, *np;
	char pwbuf[9];
	int t, sflags, f, c, uid, gid;
	int count;
	struct inode ibuf1;  /* RAND */
	struct inode ibuf2;  /* RAND */
	int ttime; /* time until ksys occurs; used by ksys code section */

	signal(2, 1);
	nice(2);
	ttyx = "/dev/ttyx";
	if ((utmp.tty=ttyn(0)) == 'x') {
		write(1, "Sorry.\n", 7);
		exit(1);
	}
	count = 0;
	ttyx[8] = utmp.tty;
	gtty(0, &ttyb);
loop:
	namep = utmp.name;
	if (argc>1) {
		np = argv[1];
		while (namep<utmp.name+8 && *np && (*np != ' ') )
			*namep++ = *np++;
		argc = 0;
	} 
	else {
		write(1, "Name: ", 7);
		while ((c = getchar()) != '\n') {
			if (c <= 0)
				exit(1);
			if (namep < utmp.name+8 && c != ' ' )
				*namep++ = c;
		}
	}

#ifdef  RESTRICT
/*
 *      local installation definition of "local" terminals
 */
       /*  rlg mods to refuse logins during shutdown */
	if(ncompar(utmp.name,"Nobody",8))
	       if ( (f = open(SHUTSW,0) ) >= 0) {
	                close(f);
			if( ttybits((utmp.tty))  & (CONSOLE|SYSTEM))
				goto contin;
			write( 1, "No remote users allowed!\n",25);
			sleep(5);
			exit(1);
		contin: ;
	       }
#endif
	while (namep < utmp.name+8)
		*namep++ = ' ';
	if (getpwentry(utmp.name, pbuf))
		goto bad;		// SJL
	np = colon(pbuf);
	if (*np!=':') {
contin1:
		sflags = ttyb.tflags;
		ttyb.tflags =& ~ ECHO;
		stty(0, &ttyb);
		write(1, "Password: ", 10);
		namep = pwbuf;
		while ((c=getchar()) != '\n') {
			if (c <= 0)
				exit(0);	// SJL
			if (namep<pwbuf+8)
				*namep++ = c;
		}
		*namep++ = '\0';
		ttyb.tflags = sflags;
		stty(0, &ttyb);
		write(1, "\n", 1);
		namep = crypt(pwbuf);
		while (*namep++ == *np++);
		if (*--namep!='\0' || *--np!=':')
			goto bad;
	}
	np = colon(np);
	uid = 0;
	while (*np != ':')
		uid = uid*10 + *np++ - '0';
	np++;
	gid = 0;
	while (*np != ':')
		gid = gid*10 + *np++ - '0';
	np++;
	np = colon(np);
	namep = np;
	np = colon(np);

#ifdef RESTRICT
/*
 *      restricted logins
 */
	if( !(ttybits(utmp.tty) & userbits(uid)) ) {
		write(1, "Permission Denied.\n",19);
		sleep(5);
		exit(1);
	}
#endif
	/*
	 * group numbers >= BADGROUP not allowed
	 * on system during time specified in
	 * grouptime string
	 */
	if(gid >= BADGROUP){
		if(ifdate(grouptime)){
			write(1,"Restricted hours.\n",18);
			exit(1);
		}
	}
	if (chdir(namep)<0) {
		write(1, "No directory\n", 13);
		exit(1);
	}
	time(&utmp.time);
	if ((f = open("/etc/utmp", 1)) >= 0) {
		t = utmp.tty;
		if (t>='a')
			t =- 'a' - (10+'0');
		seek(f, (t-'0')*16, 0);
		write(f, &utmp, 16);
		close(f);
	}
	/*
	 * check for Nobody shell - SJL
	 */
	if(uid != NOBODY){
		if((f = open("/usr/adm/wtmp",1)) >= 0){
			seek(f, 0, 2);
			write(f, &utmp, 16);
			close(f);
		}
		if((f = open("/etc/motd",0)) >= 0){
			if ((stat (".llog", &ibuf2)) != -1)
			{
				 /* print out last login time */
				write(1, "Last login: ", 12);
				write(1, ctime(&ibuf2.modtime), 25);
				fstat(f,&ibuf1);
				if(ibuf1.modtime > ibuf2.modtime){
					while ((t=read(f, mbuf, 32)) > 0)
						write(1, mbuf, t);
				}
				/*
				 * creat and close .llog file to
				 * record login time
				 */
				close(creat(".llog",0600));
			}
			else
			{       /* print out message */
				while ((t=read(f, mbuf, 32)) > 0)
					write(1, mbuf, t);
			}
			close(f);
		}
	/*
	 *      check to see if a ksys is pending and announce to
	 *      potential users this fact
	 */
		time(&tvec);
		f = open(KSYST,0);
		if( f > 0) {
			read(f,&ksys, sizeof ksys);
			close(f);
			ttime = ksys.dtime - tvec;
			printf(tsmsg1,ttime/60);
		}
		/*
		 * check for news - SJL
		 */
		if(stat(NEWS,&ibuf1) >= 0){
			if(ibuf1.size1 && (utmp.time - ibuf1.modtime < THREE_DAYS))
				write(1,"There is news.\n",15);
		}
		if(stat(".mail", &ibuf1) >= 0 && ibuf1.size1 ) {
			write(1, "You have mail.\n", 15); 
		}
	}else
		if((f = open("/etc/sysmesg",0)) >= 0){
			while((t = read(f,mbuf,32)) > 0)
				write(1,mbuf,t);
			close(f);
		}
	chown(ttyx, uid);
	setgid(gid);
	setuid(uid);
	if (*np == '\0')
		np = DEFSHELL;
	execl(np, "-", 0);
	write(1, "No shell.\n", 9);
	exit(1);
bad:
	write(1, "Login incorrect.\n", 17);
	/*
	 * must exit for Nobody shell
	 */
	exit(1);
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
			exit(1);
		}
	}
	*rp++ = '\0';
	return(rp);
}

ncompar(s1,s2,n)
char *s1, *s2;
int n;
{
register c1, c2, i;

	i = 0;
	while((c1 = *s1++) == (c2 = *s2++) && i++ < n)
		if(c1 == '\0')
			return(0);
	if(i == n)
		return(0);
	else
		return(c2-c1);
}

#ifdef RESTRICT
#include "access.c"
#endif
