#ifndef lint
static char rcsid[] = "$Header: search.c,v 2.4 85/08/06 19:20:08 matt Exp $";
#endif
/*
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original          by Dave Pare	1983
 * Ported & improved by Matt Crawford	1985
 * Ported            by Lars Pensjo	1985
 * Ported & improved by Tore Fahlstrom	1985
 *
 * The player process.  Search tries to connect to the daemon via a message
 * queue. It gathers data about the player and sends it along to the daemon.
 * Termcap routines taken from the original SVR2 source.
 *
 * Copyright (c) 1983
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pwd.h>

#include "defines.h"
#include "structs.h"

#define XQZERO	0		/* flag value: Nothing dangerous done */
#define XQINIT	1		/* flag value: done savetty */
#define XQCONN	2		/* flag value: deamon servs this player */
#define XQDISC	3		/* flag value: deamon ignore this player */
int	xqstat = XQZERO;	/* Make it possible to close down all shit */

struct message omesg;		/* buffer for outgoing messages */
struct message imesg;		/* buffer for incoming messages */
struct message amesg;		/* buffer for ack messages */

int	fd;			/* the terminal - opened from "/dev/tty" */
t_file	player;			/* who are we, what does our term look like */
int	sock;			/* socket to daemon */
int	uid;			/* user id -- well, not the real one */
int	owner = 0;		/* True if owner is playing */


main(argc, argv)
int argc;
char *argv[];
{
	extern	catchsignal ();
	char	*strcpy(), *strncpy();
	int pid;
	int apa;

	register int n;			/* read count register */
		 int sendok = TRUE;	/* did we get away the last message */
		 int acknok = TRUE;	/* did a rcve before send */

	if (geteuid() == OWNERID) {
	    owner = 1;
	    setuid(OWNERID);
	    goto skip;	/* Skip boring things */
	}
	/*
	 * Start up daemon if not running
	 */
	if (!excl("/tmp/SEARCH")) {
	    printf("Starting daemon...\n");
	    system("/usr/games/lib/search/searchd");
	    printf("Done\n");
	}

	if (!isatty(0) || !isatty(1))
		fatal("You must have a terminal to play search!\n");
	
skip:
	/*
	 * Set all the signals
	 */
	if (owner) {
	    (void) signal(SIGINT,  SIG_IGN);
	    (void) signal(SIGTERM, SIG_IGN);
	    (void) signal(SIGHUP,  SIG_IGN);
	    (void) signal(SIGPIPE, SIG_IGN);
	} else {
	    (void) signal(SIGINT,  catchsignal);
	    (void) signal(SIGTERM, SIG_IGN);
	    (void) signal(SIGHUP,  catchsignal);
	    (void) signal(SIGPIPE, catchsignal);
	}

	/*
	 *  Compute a search-user-id. It must be larger than 1 because 0
	 *  and 1 are special. 0 is notused. 1 is used for new players and
	 *  therefore used only once in the game. 
	 *  Uid is also a multiple of 3, because the messages sent to the
	 *  deamon have an even type (uid) and the messages recieved have an
	 *  odd type (uid+1).
	 */
	uid = 3 * getpid() + 3;

	/*
	 *  Try to open a channel to the deamon (if it is up and running)
	 */
	for (apa = 0; (sock = getid()) < 0; apa++) {
	    if (apa == 5) {
		if (!owner) printf("Can't start daemon\n");
		exit(1);
	    }
	    sleep(3);
	}

	/*
	 *  Call initscr() to make use of handy functions.  Save the old term
	 *  characteristics.  It is also advisable to do all terminal dependent
	 *  operations here in search and not in the deamon.
	 *  Set up terminal modes for game
	 */
	savetty ();
	xqstat = XQINIT;
	cbreak();
	noecho();


	/*
	 *  Open our own tty for both reading & writing - makes it
	 *  much easier to do ioctls since we only have to do them on
	 *  the one fd now.
	 */
	if (!owner)
	    if ((fd=open("/dev/tty", O_RDWR , 0)) < 0)
		fatal("Cannot open your tty\n");

	/*
	 *  Now set up the player structure for transmission to the daemon.
	 */
	if (owner) (void) strcpy(player.p_name, "search");
	else (void) strcpy(player.p_name, (getpwuid (getuid ()))->pw_name);
	player.uid = uid;
	if (termcap(&player))
		fatal("Terminal unsuitable for play!\n");

	/*
	 * Transmit all the info we've gathered to the daemon
	 */
	omesg.mtype = 1;	/* special type used only here */
	omesg.ident = uid;	/* to tell the deamon who I am */
	if (sizeof(player) > (sizeof(omesg) - 8))
		fatal("Prog. error: player struct > sizeof omesg.\n");
	bcopy(omesg.text, &player, sizeof(player));
	if (msgsnd(sock, &omesg, (sizeof(player) + 8), 0) == -1)
		fatal("Startup request failed!\n");
	xqstat = XQCONN;

	omesg.mtype = uid;
	imesg.mtype = uid + 1;
	amesg.mtype = uid + 2;
	if (!owner) pid = fork();
	/* When owner is playing, don't read from keyboard */
	if(!owner && !pid){
	    /* child */
	    (void) signal(SIGINT,  SIG_DFL);
	    (void) signal(SIGTERM, SIG_IGN);
	    (void) signal(SIGHUP,  SIG_DFL);
	    (void) signal(SIGPIPE, SIG_DFL);
	    /*
	    ** just read from the keyboard, send to deamon and wait for ack.
	    */
	    for (;;) {

		/*  No use of reading more than QSIZE characters because
		 *  the deamon input queue cannot take more than that.
		 */
		if ((n=read(fd, omesg.text, QSIZE)) == -1)
		    fatal("Terminal read failed!\n");
		omesg.text[n] = '\0'; /* remove in the future */
		if (msgsnd(sock,&omesg,8+n,0) == -1) {
		    fatal("msgsnd failed!\n");
		}
		/*
		 *  Wait for ack.
		 */
		if (msgrcv(sock,&amesg,sizeof amesg,uid+2,0)!= -1) {
			switch (amesg.text[0]) {
			default:
				fatal("ack gets funny!\n");
				break;
			case 1:
				/* Acknowledge -- ok to send next message */
				break;
			}
		} else
			fatal("msgrcv ack failed!\n");
	    }
	} else {
	    /* parent */	
	    /*
	    ** read from deamon and write it on the screen
	    */
	    for(;;){
		if (msgrcv(sock,&imesg,sizeof imesg,uid+1,0)!= -1) {
			switch (imesg.text[0]) {
			default:
				/* Output for your screen */
				if (owner) update(imesg.text);
				else write(fd, imesg.text, strlen(imesg.text));
				break;
			case 0:
				/* The deamon is finished with you */
				xqstat = XQDISC;
				if (owner) fatal("");
				/* Kill child */
				kill(pid,SIGINT);
				fatal("Welcome back to search!\n");
			}
		} else
			fatal("msgrcv failed!\n");
	    }
	}
}

catchsignal ()
{
	if (getuid() == OWNERID || geteuid() == OWNERID) {
	    printf("PID %d, uid %d, euid %d, owner %d\n",
		getpid(), getuid(), geteuid(), owner);
	    abort();
	}
	fatal("Signal caught -- execution terminated.\n");
}

fatal (godbye)
	char *godbye;
{
	/*
	 *  Do the tty reset right away, no need to wait...
	 */
	if (xqstat != XQZERO)
		resetty ();
	if (xqstat == XQCONN || xqstat == XQDISC) {
		/*
		 *  Read back all output not yet read by deamon
		 *  Got to fix in the future so I know if I have sent any
		 *  messages that need to get acknowledge, or shall that
		 *  be done in the deamon. Maybe the deamon should read
		 *  back both channels.
		 */
		while(msgrcv(sock,&imesg,sizeof imesg,uid+2,IPC_NOWAIT) != -1);
		while(msgrcv(sock,&imesg,sizeof imesg,uid+1,IPC_NOWAIT) != -1);
		while(msgrcv(sock,&imesg,sizeof imesg,uid,IPC_NOWAIT) != -1);
	}
	if (!owner) write (fd, godbye, strlen (godbye));
	if (xqstat == XQCONN) {
		/* Need a nl -- the game was running */
		if (!owner) write (fd, "\n", 1);
		/*
		 *  Tell deamon that we are leaving now
		 */
		omesg.text[0] = '\0';	/* remove in the future */
		if (msgsnd (sock, &omesg, 8+1, 0) == -1) {
			char *tellem =
				"Failed to tell deamon that I'm leaving!\n";
			if (!owner) write (fd, tellem, sizeof (tellem));
			exit(xqstat == XQDISC ? 0 : 10 + xqstat);
		}
		for(;;)
		    if (msgrcv(sock,&imesg,sizeof imesg,uid+1,0)!= -1) {
			switch (imesg.text[0]) {
			default:
			    /* Output for your screen */
			    if (!owner)
			    write(fd, imesg.text, strlen(imesg.text));
			    break;
			case 0:
			    exit(xqstat == XQDISC ? 0 : 10 + xqstat);
			}
		    } else break;
	}
	exit(xqstat == XQDISC ? 0 : 10 + xqstat);
}


static	char	buf[1024], *bp = buf;
struct	init {
	char	*i_name;	/* termcap name */
	char	*i_dest;	/* where to put the result */
	short	i_size;		/* sizeof associated buffer */
	short	i_pad;		/* padding flag: 0=no, 1=P, 2=P* */
} list[] = {
	{ "bc", player.p_BC, sizeof(player.p_BC), 0 },
	{ "up", player.p_UP, sizeof(player.p_UP), 0 },
	{ "cm", player.p_CM, sizeof(player.p_CM), 1 },
	{ "ce", player.p_CE, sizeof(player.p_CE), 1 },
	{ "cl", player.p_CL, sizeof(player.p_CL), 2 },
	{ NULLCH, NULLCH, 0, 0 }
};

/*
 * Build up the termcap description needed by searchd
 */
int termcap(p)
register t_file *p;
{
	register char	*cp;
	register int	lines,	cols;
	register struct init *pi = list;
	char	*term,	tbuf[BUFSIZ];
	extern	char	*getenv(),	*tgetstr();
	int getstring();

	p->p_speed = 9600;
	if ((term = getenv("TERM")) != NULL && tgetent(tbuf, term) > 0) {
		if (cp = tgetstr("pc", &bp))
			p->p_PC = *cp;
		cols = tgetnum("co", &bp);
		lines = tgetnum("li", &bp);
		while (pi->i_size != 0) {
			getstring(pi);
			pi++;
		}
		return(*p->p_CM == '\0' || *p->p_CL == '\0' || cols < 80 ||
		       lines < 24);
	}
	return(1);
}

/*
 * Termcap support routine
 */
getstring(pi)
register struct	init	*pi;
{
	register char	*cp = tgetstr(pi->i_name, &bp);

	if (cp == NULL)
		return;
	if ( pi->i_pad ) {
		while ( strchr( "0123456789.", *cp ) )
			cp++;
		if ( pi->i_pad == 2 && *cp == '*' )
			cp++;
	}
	if ( strlen(cp) >= pi->i_size )
		fatal("Prog. error: Long termcap entry found!\n");
	(void) strncpy(pi->i_dest, cp , pi->i_size);
}

getid() {
    int key;

    if ((key = ftok("/tmp/SEARCH", 0)) == -1) {
	return -1;
    }
    return msgget(key, 0);
}

bcopy(to, from, n)
register char *to, *from;
{
    while (n--) *to++ = *from++;
}
#include <termio.h>
typedef struct termio SGTTY;
#include <term.h>
#include <stdio.h>

/*
------------------------------------------------------------------------------
 *	There are a few problems in conection with the use of curses,
 *	I don't know why, but the routines below does exactly what you
 *	want them do. The same routines in curses does not.
------------------------------------------------------------------------------
 */

static struct termio oldti, newti;
cbreak () { newti.c_lflag    &= ~ICANON; newti.c_cc[VMIN]  = 1;
	newti.c_cc[VTIME] = 1; ioctl (1, TCSETA, &newti); }
nocbreak () { newti.c_lflag |= ICANON; newti.c_cc[VEOF] = oldti.c_cc[VEOF];
	newti.c_cc[VEOL] = oldti.c_cc[VEOL]; ioctl (1, TCSETA, &newti); }
echo () { newti. c_lflag |= ECHO; ioctl (1, TCSETA, &newti); }
noecho () { newti.c_lflag &= ~ECHO; ioctl (1, TCSETA, &newti); }
resetty () { newti = oldti; ioctl (1, TCSETA, &newti); }
savetty () { ioctl (1, TCGETA, &oldti); newti = oldti; }

update(str)
char *str;
{
}
