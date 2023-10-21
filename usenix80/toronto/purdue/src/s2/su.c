#
#define	SIGHUP	1
#define	SIGINT	2
#define	SIGQIT	3
#define	ECHO	010
#define LOPRI		/* allow 'su e' to lower priority fast */
#define COVER		/* enable coverup of typing mistakes */

/* su -- become super-user */


/* su changes:

9/18/78 Mistyped passwords by known su people are not printed
	on the console instead 'xxxxxx' is printed.
	For all other users incorrect passwords are printed
	on console.
	Added 'su e' feature to immediately lower priority
	for su login and shell.  This to be used when
	su needs to get into system fast.
*/

#ifdef COVER

/* include staff.h
If the coverup feature is not wanted, remove the define
COVER statement at the top of this file.
This will cause the coverup code to be skipped at
compilation time.

staff.h declares array puids. puids is a list
of priviledged (su) uids.  This file must
contain all priviledged (su) users on the system.
*/

#include <staff.h>
char    xxxpass[] "xxxxxx";	/* cover up for su attempt by known su */
#endif


int	ttysav;
int	ttybuf[3];
char	password[100];
char	pwbuf[100];


main(argc,argv)
char **argv;
{
	register char *p, *q;
	int tvec[2], usrid;
	char *np, *tim, ttyno;
	extern fin, fout, sig();

#ifdef LOPRI
	/* if command was 'su e' give this a lower priority */
	if (argc > 0 && *argv[1] == 'e')
		nice(-127);
#endif

	if ((gtty(0,ttybuf)<0) || ((ttyno = ttyn(0)) == 'x')){
		write(2, "Not a tty!\n", 11);
		exit(1);
	}
	if (!(usrid = getuid()))
		goto root;
	if (getpw(0, pwbuf))
		goto badpw;
	(&fin)[1] = 0;
	p = pwbuf;
	while (*p != ':')
		if (*p++ == '\0')
			goto badpw;
	if (*++p == ':')
		goto ok;
	ttysav = ttybuf[2];
	ttybuf[2] =& ~ECHO;
	signal(SIGINT, sig);
	signal(SIGQIT, sig);
	stty(0, ttybuf);
	printf("password: ");
	q = password;
	while ((*q = getchar()) != '\n')
		if (*q++ == '\0')
			sig();
	signal(SIGINT,1);
	signal(SIGQIT,1);
	*q = '\0';
	password[8] = '\0';
	ttybuf[2] = ttysav;
	stty(0, ttybuf);
	printf("\n");
	q = crypt(password);
	while (*q++ == *p++);
	if (*--q == '\0' && *--p == ':')
		goto ok;
	goto error;

badpw:
	printf("bad password file\n");
	exit(1);
ok:
	if (fork() == 0) {
		if((getpw(usrid,pwbuf))== 0){
			np = pwbuf;
			while (*np != ':')
				np++;
			*np = 0;
			np = pwbuf;
		}else
			np = "{Unassigned UID}";
		time(tvec);
		tim = clock(tvec);
		close(1);
		if (open("/dev/tty8", 1) > 0){
			printf("%s: %s (%d) logged in as super-user at tty%c\n",
			    tim, np, usrid,  ttyno);
		}
		exit();
	}
	setuid(0);
root:
	signal(SIGINT,0);
	signal(SIGQIT,0);
	execl("/bin/sh", "-", 0);
	printf("cannot execute shell\n");
	exit();
error:
	if (fork() == 0) {
		if((getpw(usrid,pwbuf))== 0){
			np = pwbuf;
			while (*np != ':')
				np++;
			*np = 0;
			np = pwbuf;
		}else
			np = "{Unassigned UID}";
		time(tvec);
		tim = clock(tvec);
		close(1);
		if (open("/dev/tty8", 1) > 0){
			if (*password == '\0')
				exit();

#ifdef COVER
	/* if user is on list, print xxx instead of password */
			if (checkuid()) {
				q = xxxpass;    /*copy xxxpass to password*/
				p = password;
				while(*p++ = *q++);
			}
#endif

			printf("%s: SU %s (%d) attempted \"%s\" at tty%c\n",
			    tim, np, usrid, password, ttyno);
		}
		exit();
	}
	printf("sorry\n");
}

getpw(uid, buf)
int uid;
char buf[];
{
	auto pbuf[259];
	static pwf;
	register n, c;
	register char *bp;

	if (pwf == 0)
		pwf = open("/etc/6passwd", 0);
	if (pwf < 0)
		return(1);
	seek(pwf, 0, 0);
	pbuf[0] = pwf;
	pbuf[1] = 0;
	pbuf[2] = 0;

	for (;;) {
		bp = buf;
		while ((c = getc(pbuf)) != '\n') {
			if (c <= 0)
				return(1);
			*bp++ = c;
		}
		*bp++ = '\0';
		bp = buf;
		n = 3;
		while (--n)
		while ((c = *bp++) != ':')
			if (c == '\n')
				return(1);
		while ((c = *bp++) != ':') {
			if (c < '0' || c > '9')
				continue;
			n = n * 10 + c - '0';
		}
		if (n == uid)
			return(0);
	}
	return(1);
}

clock(atvec)
int *atvec;
{
	register *p;
	register char *c;
	static char tbuf[12];

	p = localtime(atvec);
	c = tbuf;

	*c++ = ++p[4] / 10 + '0';
	*c++ = p[4] % 10 + '0';
	*c++ = '/';
	*c++ = p[3] / 10 + '0';
	*c++ = p[3] % 10 + '0';
	*c++ = ' ';
	*c++ = p[2] / 10 + '0';
	*c++ = p[2] % 10 + '0';
	*c++ = ':';
	*c++ = p[1] / 10 + '0';
	*c++ = p[1] % 10 + '0';
	*c = '\0';
	return(tbuf);
}

sig() {
	ttybuf[2] = ttysav;
	stty(0, ttybuf);
	putchar('\n');
	exit();
}

#ifdef COVER

checkuid()
	/*
	* returns 1 if user is on list, else 0.
	* root is always on list.
	* puids array is an integer array of uids which
	* are to return 1. all others return 0.
	* this type of routine is typically used in
	* 'god' programs such as su and wall.
	*/
{
	register uid, i;

	uid = getuid();
	if(uid == 0)
		return(1);
	for(i = 0; i < sizeof puids / 2; i++)
		if(uid == puids[i])
			return(1);
	return(0);
}
#endif
