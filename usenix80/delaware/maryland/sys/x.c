#       /* all new communications program for LPA UNIX  Dec 8, 1978 */
				/* last modification: April 2, 1980 */

			/* constant definitions */

/* mode bits used by gtty & stty */
#define EVEN    0200
#define ODD     0100
#define RAW     0040
#define CRLF    0020
#define ECHO    0010
#define HUP     0001

/* and some other stuff */
#define PRIORITY        -10     /* priority for process to run at */
#define ESCAPE          '\033'
#define BELL            '\007'
#define BLANK           ' '
#define SIGINT          2
#define FOREGROUND      1
#define BACKGROUND      2
#define READPIPE        3
#define FOREVER         for(;;)
#define BUFFERSIZE      100	/* size of buffer for file name or command */

/****************************************************************************/

			/* global data declarations */

struct {        /* this is the format of the lock file */
	int  usr_id;
	int  parent_id;
	int  procA_id;
	int  procB_id;
	char tty_id;
	}       lock_buf;

struct {        /* format of entries in "/etc/ttys" */
	char active;
	char id;
	char opt;
	char nl;
	}       ttys_buf;

struct {        /* structure used by stty & gtty */
	char ispeed, ospeed;
	char erase, kill;
	int  mode;
	} save_mode, comm_mode, cmnd_mode, link_mode;

struct {        /* structure used by buffered i/o routines */
	int  fildes;
	int  nunused;
	char *xfree;
	char buff[512];
	} iobuf[1];

int speedtable[] {0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400,
			4800, 9600};

int inline 0, state, state_change 0, comm_dev, lock_file, ttys_file,
	baud, i, copyfile 0, pipe_file[2], badcnt 0;

char pwbuf[100],                                /* used by getpw() */
	t, c, *p, *name, *command,
	comttyno,                               /* last char of tty name used for link */
	comdevname[] "/dev/ttyx",               /* full tty name */
	lockfilname[] "/tmp/comm.lock.x";       /* insures exclusive use */

int cantopen(); /* called when user interrupts `open' call on communications device */

/****************************************************************************/

main(argc, argv)
int argc;
char *argv[];
{

		/* initialize names of link & lock files */
if (argc > 1)
	comttyno = argv[1][0];
else {
	ttys_file = open("/etc/ttys", 0);
	FOREVER {
		if (read(ttys_file, &ttys_buf, 4) < 4) {
			printf("Link file entry not found in /etc/ttys\n");
			exit(1);
			}
		if (ttys_buf.opt == 'X') {
			comttyno = ttys_buf.id;
			break;
			}
		}
	close(ttys_file);
	}

lockfilname[sizeof(lockfilname)-2] = comdevname[sizeof(comdevname)-2] = comttyno;

if ((lock_file = open(lockfilname, 0)) >= 0) {     /* there is a process active, but whose is it? */
	if (read(lock_file, &lock_buf, sizeof(lock_buf)) < sizeof(lock_buf)) {
		printf("Lock file is in bad format.\n");
		exit(1);
		}
	if (lock_buf.usr_id != (getuid() & 0177)) {     /* someone else is using this program */
		if (getpw(lock_buf.usr_id, pwbuf)) {    /* can't find out who */
			printf("Someone else is using the link!\n");
			exit(1);
			}
		else {
			for (p = pwbuf; *p && (*p != ':'); p++);
			*p = 0;
			if (*pwbuf >= 'a' && *pwbuf <= 'z')
				*pwbuf =- 040;  /* capitalize */
			printf("%s is using the link.\n", pwbuf);
			exit(1);
			}
		}
	else {  /* it is mine! */
		if (lock_buf.tty_id == ttyn(2)) {
			close(lock_file);
			setuid(getuid() & 0177);
			lock_file = open(lockfilname, 1);  /* re-open for writing */
			lock_buf.parent_id = getpid();  /* so other processes know who to signal */
			write(lock_file, &lock_buf, sizeof(lock_buf));
			kill(lock_buf.procA_id, FOREGROUND);
			kill(lock_buf.procB_id, FOREGROUND);
			hang();         /* wait until this process is killed */
			}
		else {  /* but wrong tty */
			printf("You already have a communication process on tty%c.\n", lock_buf.tty_id);
			exit(1);
			}
		}
	}

		/* no process active */

if ((t = ttyn(0)) == 'x' || t != ttyn(1) || t != ttyn(2)) {
	printf("Standard input and output must be to a terminal.\n");
	exit(1);
	}

ttys_file = open("/etc/ttys", 0);
FOREVER {
	if (read(ttys_file, &ttys_buf, 4) < 4) {
		printf("Tty entry not found in /etc/ttys\n");
		exit(1);
		}
	if (ttys_buf.id == comttyno)
		if (ttys_buf.active == '1') {
			printf("The link is in use as a teletype line.\n");
			exit(1);
			}
		else
			break;
	}
close(ttys_file);

chown(comdevname, (getuid() & 0177));
chmod(comdevname, 0600);
nice(PRIORITY);
setuid(getuid() & 0177);

if ((lock_file = creat(lockfilname, 0644)) < 0) {
	printf("Can't place lock.\n");
	exit(1);
	}

if (pipe(pipe_file) < 0) {
	printf("Can't create pipe.\n");
	unlink(lockfilname);
	exit(1);
	}

signal(SIGINT, cantopen);       /* in case `open' call blocks */

printf("\n\nOne moment please . . .  ");

if ((comm_dev = open(comdevname, 2)) < 0) {
	printf("Can't open link file.\n");
	unlink(lockfilname);
	exit(1);
	}

signal(SIGINT, 0);

gtty(0, &save_mode);
gtty(0, &comm_mode);
gtty(0, &cmnd_mode);
gtty(comm_dev, &link_mode);

/* mode of terminal when communication link is open */
comm_mode.mode =|   RAW;
comm_mode.mode =& ~CRLF;

/* mode of terminal when reading escape commands */
cmnd_mode.mode = comm_mode.mode & ~ECHO;

		/* set mode of link device */
link_mode.ispeed = 9;
link_mode.ospeed = 9;
link_mode.mode = EVEN | ODD | RAW;
stty(comm_dev, &link_mode);

lock_buf.usr_id    = getuid() & 0177;
lock_buf.parent_id = getpid();

if ((lock_buf.procA_id = fork()) == 0)
	procA();

if ((lock_buf.procB_id = fork()) == 0)
	procB();

lock_buf.tty_id = ttyn(0);

write(lock_file, &lock_buf, sizeof(lock_buf));

hang();         /* wait to be killed */
}

/****************************************************************************/

cantopen() {    /* `open' call must have blocked so user hit `rubout' */

unlink(lockfilname);
printf("\n\nOh well . . .   they probably didn't want to talk to you anyway.\n");
exit(1);
}

/****************************************************************************/

block() {

while (sleep(300) == 0);        /* infinite loop */
}

/****************************************************************************/

foregnd() {

state_change++;
state = FOREGROUND;
signal(FOREGROUND, foregnd);
}

/****************************************************************************/

backgnd() {

state_change++;
state = BACKGROUND;
signal(BACKGROUND, backgnd);
}

/****************************************************************************/

readpipe() {
char buf[BUFFERSIZE+2];
register char *char_p;
int status;

state_change++;

for (char_p = buf; (read(pipe_file[0], char_p, 1) == 1) && *char_p; char_p++);

switch (buf[0]) {

	case 'C':
		if (copyfile > 0) {
			fflush(iobuf);
			close(iobuf->fildes);
			}
		fcreat("/dev/null", iobuf);     /* to initialize iobuf */
		close(iobuf->fildes);
		if ((iobuf->fildes = shell()) < 0) {
			printf("\r\n\007Can't fork shell process!\r\n");
			copyfile = 0;
			}
		else {
			for (char_p = &buf[1]; *char_p; char_p++)
				putc(*char_p, iobuf);
			putc('\n', iobuf);
			copyfile++;
			}
		break;

	case 'T':
		if (copyfile > 0) {
			fflush(iobuf);
			close(iobuf->fildes);
			}
		if (fcreat(&buf[1], iobuf) < 0) {
			printf("\r\n\007Can't open '%s' for writing.\r\n", &buf[1]);
			perror("");
			putchar('\r');
			copyfile = 0;
			}
		else
			copyfile++;
		break;

	case 'U':
		if (copyfile > 0) {
			fflush(iobuf);
			close(iobuf->fildes);
			}
		copyfile = 0;
		break;

	case 'X':
		printf("\r\n=== running ===\r\n");
		if (fork() == 0) {
			close(0);
			close(1);
			dup(comm_dev);
			dup(comm_dev);
			close(comm_dev);
			execl("/bin/sh", "sh", "-c", &buf[1], 0);
			perror("Can't exec shell");
			exit(1);
			}
		while (wait(&status) < 0);
		printf("\r\n=== completed ===\r\n");
		kill(lock_buf.procA_id, FOREGROUND);
		foregnd();
		break;

	default:
		printf("\r\n\007Internal command error!\r\n");

	}
signal(READPIPE, readpipe);
}

/****************************************************************************/

procA() {    /* this routine copies characters from the user to the link */

sleep(1);       /* kludge to let parent process initialize the lock file */

backgnd();      /* initialize interrupt routines */
foregnd();

close(lock_file);
lock_file = open(lockfilname, 0);
read(lock_file, &lock_buf, sizeof(lock_buf));

rawtty();
printf("\r\n\007=== open ===\r\n");

FOREVER {
	read(0, &c, 1);
	if (c != ESCAPE)
		write(comm_dev, &c, 1);
	else {  /* do some fancy stuff */
		cmndtty();
		write(1, "\007", 1);    /* ding-a-ling */
		read(0, &c, 1);
		switch (c) {
			case ESCAPE:            /* transmit an ESC */
				write(comm_dev, &c, 1);
				break;

			case 'P': case 'p':     /* pause */
				kill(lock_buf.procB_id, BACKGROUND);
				cookedtty();
				printf("\n=== pause ===\n");
				seek(lock_file, 0, 0);
				read(lock_file, &lock_buf, sizeof(lock_buf));
				kill(lock_buf.parent_id, 4);
				for (state = BACKGROUND; state == BACKGROUND; )
					block();        /* don't read from terminal */
				printf("=== reopened ===\n");
				break;

			case 'Q': case 'q':     /* quit */
				cookedtty();
				write(pipe_file[1], "U\0", 2);
				kill(lock_buf.procB_id, READPIPE);
				seek(lock_file, 0, 0);
				read(lock_file, &lock_buf, sizeof(lock_buf));
				unlink(lockfilname);
				sleep(1);       /* give process b time to flush any buffered output */
				printf("\007\n=== closed ===\n");
				kill(lock_buf.procB_id, 9);
				kill(lock_buf.parent_id, 4);
				exit(0);

			case 'S': case 's':     /* change line speed */
				cookedtty();
				printf("\nLine speed? ");
				baud = atoi(reads());
				for (i = 1; speedtable[i] != baud && i < sizeof(speedtable); i++);
				if (i < sizeof(speedtable)) {
					link_mode.ispeed = link_mode.ospeed = i;
					stty(comm_dev, &link_mode);
					}
				else
					printf("Foo\n");
				break;

			case 'E': case 'e':     /* turn on echoing */
				comm_mode.mode =| ECHO;
				break;

			case 'N': case 'n':     /* turn off echoing */
				comm_mode.mode =& ~ECHO;
				break;

			case 'C': case 'c':	/* shell command mode */
				cookedtty();
				printf("\nCommand? ");
				command = reads();
				write(pipe_file[1], "C", 1);
				write(pipe_file[1], command, length(command));
				write(pipe_file[1], "\0", 1);
				kill(lock_buf.procB_id, READPIPE);
				break;

			case 'T': case 't':	/* transcript mode */
				cookedtty();
				printf("\nFile name? ");
				name = reads();
				write(pipe_file[1], "T", 1);
				write(pipe_file[1], name, length(name));
				write(pipe_file[1], "\0", 1);
				kill(lock_buf.procB_id, READPIPE);
				break;

			case 'U': case 'u':	/* weird modes off */
				write(pipe_file[1], "U\0", 2);
				kill(lock_buf.procB_id, READPIPE);
				break;

			case 'H': case 'h':     /* help */
				printf("\n\rCommands:\n\r\tP(ause)\n\r\tQ(uit)\n\r\tS(peed change)");
				printf("\n\r\tE(cho)\n\r\tN(o echo)\n\r\tC(ommand mode)");
				printf("\n\r\tT(ranscript mode)\n\r\tU(nknown) - turns off C or T");
				printf("\n\r\tB(reak) - sends break to remote host");
				printf("\n\r\tX(ecute) - temporarily run anoter program in place of \"x\"");
				printf("\n\r\tH(elp) - but you must know that already!\n\r");
				break;

			case 'B': case 'b':     /* break */
				write(comm_dev, "", 0);
				break;

			case 'X': case 'x':
				cookedtty();
				printf("\nCommand? ");
				command = reads();
				write(pipe_file[1], "X", 1);
				write(pipe_file[1], command, length(command));
				write(pipe_file[1], "\0", 1);
				kill(lock_buf.procB_id, READPIPE);
				for (state = BACKGROUND; state == BACKGROUND; )
					block();
				break;

			default:                /* undefined command */
				if (c < BLANK)
					printf("\007\007'^%c'???\007\007", c + 0100);
				else
					printf("\007\007'%c'???\007\007", c);

				if (badcnt++ >= 3) {
					badcnt = 0;
					printf("\n\rDo: ESC-`H' for help.\n\r");
					}
			}
			rawtty();
		}
	}
}

/****************************************************************************/

procB() {    /* this process copies characters from the link to the terminal */

sleep(1);
signal(READPIPE, readpipe);
backgnd();
foregnd();

close(lock_file);
lock_file = open(lockfilname, 0);
read(lock_file, &lock_buf, sizeof(lock_buf));

FOREVER {
	if (read(comm_dev, &c, 1) < 1) {
		if (state_change) {
			state_change = 0;
			continue;
			}
		else {          /* error on link file */
			cookedtty();
			printf("\007\nLost Connection!\n\007");
			kill(lock_buf.procA_id, 9);
			seek(lock_file, 0, 0);
			read(lock_file, &lock_buf, sizeof(lock_buf));
			kill(lock_buf.parent_id, 4);
			unlink(lockfilname);
			exit(1);
			}
		}
	else {  /* a character was read */
		if (copyfile > 0)	/* also divert a copy */
			if (putc(c, iobuf) < 0) {
				printf("\r\n\007Error in writing to transcript file.\r\n");
				perror("");
				putchar('\r');
				fflush(iobuf);
				close(iobuf->fildes);
				copyfile = 0;
				}

		if (state == FOREGROUND)
			write(1, &c, 1);
		else {                  /* <<surround the line by brackets>> */
			if (!inline && (c >= BLANK) && (c != 0177)) {
				printf("\t<<");
				inline++;
				}
			if (c == '\r') {
				inline = 0;
				printf(">>\r");
				}
			else
				write(1, &c, 1);
			}
		}
	}
}

/****************************************************************************/

reads() {
static char buf[BUFFERSIZE];
register char *char_p;

for (char_p = buf; char_p < &buf[sizeof(buf)-1]; char_p++) {
	read(0, char_p, 1);
	if (*char_p < BLANK) /* control character */
		break;
	}

while(*char_p <= BLANK && char_p >= buf)
	/* trim trailing blanks */
	*char_p-- = 0;

for(char_p = buf; *char_p == BLANK; char_p++);  /* trim leading blanks */

return(char_p);
}

/****************************************************************************/

rawtty() {

stty(0, &comm_mode);
}

/****************************************************************************/

cookedtty() {

stty(0, &save_mode);
}

/****************************************************************************/

cmndtty() {

stty(0, &cmnd_mode);
}

/****************************************************************************/

length(string)
char *string;
{
int len;

for (len = 0; string[len]; len++);

return(len);
}

/****************************************************************************/

shell() {       /* returns a descriptor for the std input of a shell process */
int s_pipe[2], pid, status;

pipe(s_pipe);

if((pid = fork()) > 0) {        /* main ``x'' process */
	wait(status);           /* to avoid creating a defunct process */
	close(s_pipe[0]);
	return(s_pipe[1]);
	}

if (pid < 0) {
	perror("Can't fork intermediate process");
	putchar('\r');
	return(-1);
	}

if ((pid = fork()) > 0)         /* intermediate process */
	exit(0);

if (pid < 0) {
	perror("Can't fork shell process");
	putchar('\r');
	exit(1);
	}

close(0);       /* redirect std input to pipe */
dup(s_pipe[0]);

close(1);       /* redirect std output to bit bucket */
open("/dev/null", 1);

execl("/bin/sh", "sh", 0);
execl("/usr/bin/sh", "sh", 0);

perror("Can't exec shell");
putchar('\r');
exit(1);
}

/****************************************************************************/

die() { /* an elegant way to terminate the main process without causing
	   the shell to print distracting error messages */

exit(0);
}

/****************************************************************************/

hang() {

signal(1, 1);
signal(2, 1);
signal(3, 1);
signal(4, die);

FOREVER /* wait here until we get a signal #4 */
	block();
}

/****************************************************************************/
