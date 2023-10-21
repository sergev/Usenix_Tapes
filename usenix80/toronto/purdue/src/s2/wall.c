#
char namebuf[64];

#include <staff.h>    /* declare array puids, list of uids
			* which can use wall */
#include <signals.h>

char	mesg[3000];
int	msize;
struct
{
	char	name[8];
	char	tty[2];
	int	time[2];
	int	junk;
} utmp[200];

main(argc, argv)
char *argv[];
{
	register i, *p;
	char *pnam;
	int f;
	extern sig();
	signal(SIGCLK,sig);     /* set up can't fork alarm */

	checkuid();
	f = open("/etc/utmp", 0);
	if(f < 0) {
		printf("wall: Can't open /etc/utmp\n");
		exit();
	}
	read(f, utmp, sizeof utmp);
	close(f);
	f = 0;
	if(argc >= 2) {
		f = open(argv[1], 0);
		if(f < 0) {
			printf("wall: Can't read %s\n", argv[1]);
			exit();
		}
	}
	while((i = read(f, &mesg[msize], sizeof mesg - msize)) > 0)
		msize =+ i;

	/* if amped off, go ahead and wall without releasing tty */
	if ((signal(SIGINT,1) & 01) == 0) {
		signal(SIGQIT,1);
		/* try to fork,
		 * if successful, release tty and then wall
		 * else go ahead and wall waiting at tty til done
		 */
		if (fork() > 0)
			exit();
	}

	close(f);
	getpw(getuid(), namebuf);
	pnam = namebuf;
	while(*pnam != ':')
		pnam++;
	*pnam = 0;
	for(i=0; i<sizeof utmp/sizeof utmp[0]; i++) {
		p = &utmp[i];
		if(p->tty[0] == 0)
			continue;
		sleep(1);
		sendmes(p->tty[0]);
	}
}

sendmes(tty)
char tty;
{
	int fd, pid;
	register char *s;
#ifdef NTTY
	register char *kludge, *gross;
#endif

#ifdef DEBUG
printf("wall to tty%c\n",tty);
return;
#endif

	while((pid = fork()) == -1) {	/* wait til can fork */
		alarm(2);
		while(wait() != -1);
		alarm(0);
	}
	if(pid)
		return;
#ifndef NTTY
	s = "/dev/ttyx";
	s[8] = tty;
#endif
#ifdef NTTY		/* if numeric tty numbers */
	gross = "/dev/ttyxxx";
	s = gross + 8;
	kludge = itoa(tty & 0377);
	while(*s++ = *kludge++)
		;
	s = gross;
#endif
	fd = open(s, 1);
	if(fd < 0) {
		printf("wall: cannot open tty%c\n", tty);
		exit();
	}
	close(1);
	dup(fd);
	printf("Broadcast Message from %s\n\n", namebuf);
	flush();
	write(1, mesg, msize);
	exit();
}
checkuid()
{
	register uid, i;

	uid = getuid();
	if(uid == 0 || ttyn(2) == '8')
		return;
	for(i = 0; i < sizeof puids / 2; i++)
		if(uid == puids[i])
			return;
	printf("wall: Permission denied\n");
	exit();
}

sig() {
	signal(SIGCLK,sig);
}
