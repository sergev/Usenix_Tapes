#include <signal.h>
#include <sgtty.h>

#define equal(a,b) ( !strcmp(a,b) )

/* #"# Modified 1 Feb 1978 by Henry Spencer to fix a bug:  set */
/*  terminal modes (very) wrong. */

/* #"# Modified July 1979 by Dave Sherman to create a lock file.
 *	The lock file contains one word with the UID in it. If
 *	a different user tries to get on he must break the line
 *	open with the -f (force) flag. xfr(I) should look at the same
 *	lock file, perhaps.
 *	The file is /tmp/cu.lock (or /tmp/cu.lock.x for ttyx).
 *
 *	Also modified by Dave Sherman to allow ~!command to be a
 *	command to a local shell.
 *
 *	Modified by Dave Sherman October 1979 to catch signals
 *	so the lock file will get remove (it practice this will
 *	affect hangups only since terminal is in raw mode.
 */

/* 
 * Fake terminal for logging onto other systems.
 * Usage:	cu [-h] [-l line] [-s speed]
 *
 * -h	indicates half duplex
 * -l	is used to override the default line
 * -s	is used to override the default line speed (of 300 baud).
 * -b	causes the program to transmit breaks on the output line.
 * -f	forces the lock open on the output line.
 *
 * This program is intended to be a subset of the PWB/UNIX "cu"
 * program.  It does NOT support most of the cu options.  It does
 * support a few undocumented options, for compatibility with cu
 * and with the old "tcom" program.
 *
 * -lx	sets the output line to /dev/ttyx
 * -t	is currently a synonym for -h
 *
 */
char culine[] "/dev/cul0";

/* DS July 1979 */
char lockfile[20] "/tmp/cu.lock";
int forceflag	0;
int globalu;
struct sgttyb localstuff;

int CULINEBREAKABLE	1;	/* system dependant.  If you don't know, set this
				 * to zero.  then a null will be transmitted.  most
				 * programs that look for break do so by looking
				 * for null.
				 */
int breakable	= 0;
char *line culine;
char local[] "local:\r\n";
char remote[] "remote:\r\n";
#define MODE	(RAW|ODDP|EVENP)
#define EOT	004
struct sgttyb tty, stuff;
int terminal;
#define DEFSPEED	B1200
#define SIZSPEEDS 14
struct speeds {
	char *speed;
	int breakchars;
} speeds[SIZSPEEDS] {
	0,0,
	"50", 2,
	"75", 2,
	"110", 3,
	"135.5", 4,
	"150", 4,
	"200", 5,
	"300", 7,
	"600", 14,
	"1200", 28,
	"1800", 42,
	"2400", 56,
	"4800", 112,
	"9600", 224
};


interrupt() { quit(2); }


main(argc, argv)
	char *argv[];
{
	register i, u;
	int him, child, full;
	int charstobreak, newspeed, thespeed;
	int whohim, whome, fd, loc;	/* DS July 1979 */
	char command[256];		/* DS July 1979 */
	char c, lastc, ttyn();

	full = 1;
	newspeed = DEFSPEED;
	argc--;
	argv++;
	while(argc>0 && argv[0][0]=='-') {
		switch(argv[0][1]) {
		/* DS July 1979 */
		case 'f':
			forceflag++;
			break;
		/* drg Dec 1980 */
		case 'b':
			breakable++;
			break;

		case 'h':
		case 't':
			full = 0;
			break;
		case 'l':
			if(argv[0][2]=='\0') {
				argc--;
				argv++;
				if(argc<1)
					usage();
				line = argv[0];
			}
			else {
				line = "/dev/ttyx";
				line[strlen(line)-1] = argv[0][2];
			}

			/* DS July 1979: "/tmp/cu.lock.x" */
			lockfile[12] = '.';
			lockfile[13] = line[strlen(line) - 1];
			lockfile[14] = '\0';
			break;
		case 's':
			argc--;
			argv++;
			if(argc<1)
				usage();
			for (i=1; i<SIZSPEEDS; i++)
				if (equal(argv[0],speeds[i].speed)) break;
			if (i == SIZSPEEDS) error("unknown speed");
			else newspeed = i;
			break;
		default:
			usage();
		}
		argc--;
		argv++;
	}
	if(equal(line, culine))
		breakable = CULINEBREAKABLE;
	him=getpid();
	whome = getuid();

	/* DS July 1979 */
	if((fd=open(lockfile, 0)) > 0)
	{
		if(forceflag)
		{
			write(2, "forcing line open\n", 18);
			unlink(lockfile);
		}
		else
		{
/* if you can log out of cu and still keep the line for your own use
   (i.e., xfr'ing stuff), then only you should be able to get back into
   the line. Xfr should use the same convention.
(This is not implemented, though.)
*/
			read(fd, &whohim, sizeof whohim);
			write(2,"Someone else is using the line: ",32);
			if(getpw(whohim, lockfile) == 0)
				write(2, lockfile, namefrom(lockfile));
			else
				write(2, "unknown user!!!", 15);
			write(2, "\n", 1);
			exit(1);
		}
	}
	/* end DS */

	if((u=open(line, 2))<0)
		error("can't open communications line");
	globalu = u;	/* DS July 1979 */
	terminal = isatty(0);
	if (terminal) {
		gtty(0, &tty);
		copyb(&tty, &localstuff, sizeof tty);
		localstuff.sg_flags =| RAW;
		localstuff.sg_flags &= ~CRMOD;
		if(full)
			localstuff.sg_flags =& ~ECHO;
		else
			localstuff.sg_flags =| ECHO;
		stty(0, &localstuff);
		write(2,remote,strlen(remote));
	}

	/* catch signals -- DS October 1979 */
	signal(SIGHUP, interrupt);	/* hangup */
	signal(SIGINT, interrupt);	/* "rubout" but we're in raw mode */
	signal(SIGQUIT, interrupt);	/* "quit" but we're in raw mode */

	/* DS July 1979. If we ge this far, create the lockfile */
	fd = creat(lockfile, 0666);	/* if it doesn't work, who cares? */
	write(fd, &whome, sizeof whome);
	close(fd);

	gtty(u, &stuff);
	if (newspeed)
		{
		stuff.sg_ispeed = newspeed;
		stuff.sg_ospeed = newspeed;
		}
	thespeed = stuff.sg_ospeed;
	charstobreak = speeds[thespeed & 017].breakchars;
	stuff.sg_flags = MODE;
	stty(u, &stuff);
	if(child=fork()){
		/* send process */
		lastc = '\n';
		for(;;){
			if(rdch(&c)<=0) {
			Rexit:
				kill(child, SIGHUP);
				if(terminal)
					write(2,local,strlen(local));
				quit(0);
			}
#ifdef TIOCBREAK
			if (c==0 && breakable) {
				/* send a break */
				stty(u, &stuff);/* force buffer cleared */
				ioctl(u, TIOCBREAK, stuff);
				/* time the break by writing correct number
				 * of chars.  since break bit is on, the chars
				 * won't actually be sent.
				 */
				for(i=0; i<charstobreak; i++)
					write(u, "X", 1);
				/* use stty to delay until break timing done */
				stty(u, &stuff);
				/* end the break */
				ioctl(u, TIOCNOBREAK, stuff);
				sleep(1);/* till line settles down */
			}
			else
#endif
				if(c=='~' && newline(lastc)) {
				/* a cu command line */
				if(terminal)
					write(2, &c, 1);
				if(rdch(&c)<=0)
					goto Rexit;
				switch(c) {
				case EOT:
					goto Rexit;
				case '.':
					write(2, &c, 1);
					while(rdch(&c)==1 && !newline(c))
						write(2, &c, 1);
					write(2, "\r\n", 2);
					goto Rexit;
				case '~':
					write(u, &c, 1);
					break;
				/* DS July 1979 */
				case '!':
					stty(0, &tty);
					loc = 0;
					write(2, "!", 1);
					while(newline(c) == 0)
					{
						rdch(&c);
						command[loc++] = c;
					}
					command[loc] = '\0';
					system(command);
					stty(0, &localstuff);
					write(2, "!\n", 2);
					break;
				default:
					write(2, "unimplemented command\r\n", 23);
					c = '\n';
				}
			}
			else write(u,&c,1);
			lastc = c;
		}
	}
	else{
		/* receive process */
		for(;;){
			if(read(u, &c, 1)<=0){
				if(terminal)
					write(2, local, strlen(local)); 
				kill(him, SIGHUP);
				if (terminal) stty(0, &tty);
				error("line dropped");
			}
			write(1, &c, 1);
		}
	}
}
quit(x){
	if (terminal) stty(0, &tty);
	unlink(lockfile);	/* DS July 1979 */
	exit(x);
}
usage() {
	error("usage: cu [-h] [-t] [-l line] [-s speed]");
}

error(s) {
	write(2,"cu: ",4);
	write(2,s,strlen(s));
	write(2,"\n",1);
	quit(1);
}

newline(c)
	register c;
{
	return(c=='\n' || c=='\r' || c==EOT || c==0177);
}

/* DS July/80 */
namefrom(str)
	register char *str;
{
	register char *p;
	register char colon;
	colon = ':';
	p = str;
	while(*p != colon && *p++);
	return(p-str);
}



copyb(from, to, nbytes)
	register char *from, *to;
	register unsigned nbytes;
	{
	while(nbytes--)
		*to++ = *from++;
	}
rdch(cp)
	char *cp;
{
	int n;

	n = read(0, cp, 1);
	*cp &= 0177;
	return (n);
}
