/*
 *	mailwatch - New mail watcher which manipulates SUN tty windows
 *		Changes icons when new mail arrives and optionally
 *		opens SUN window and starts up mail reading program
 *
 *	Usage: mailwatch -a (Ask before starting up mail reader)
 *			 -c (Clear screen before reading mail)
 *			 -e (Bring window to top when mail arrives)
 *			 -o (Automatically open window for new mail)
 *			 -i (Icon to display when mailbox empty)
 *			 -I (Icon to display when mailbox full)
 *			 -l (Label for empty mailbox icon)
 *			 -L (Label for full mailbox icon)
 *			 -m (Mail reading program)
 *			 -t (New mail check time)
 *
 *	Compliation:	cc -o mailwatch -O mailwatch.c
 *	Ownership:	any/any
 *	Mode:		0755
 *	Binary:		anywhere
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>

int	CheckTime = 120;	/* Time between mail checks */
char	*MailBox;		/* Mailbox file */

char	*EmptyMboxIcon = "/usr/include/images/emptymbox", /* CONFIG */
		/* Icon for empty mailbox */
	*FullMboxIcon = "/usr/include/images/fullmbox";	  /* CONFIG */
		/* Icon for full mailbox */

char	*EmptyMboxLabel = "",	/* Label for empty mbox icon */
	*FullMboxLabel = "";	/* Label for full mbox icon */

char	*MailReader = "Mail";	/* Mail reading program */

int	AutoOpen = 0,		/* Automatically open window for mail */
	Clear = 0,		/* Clear window before starting MailReader */
	Ask = 0,		/* Ask before starting MailReader */
	Expose = 0;		/* Expose sun window for mail */

int	OnSun;			/* Running under SUN windows */

char	*clr_window = "\014",	/* Clear SUN window */
	*c_window = "\033[2t",	/* Close SUN window */
	*o_window = "\033[1t",	/* Open SUN window */
	*e_window = "\033[5t",	/* Expose SUN window */
	*i_label = "\033]L%s\033\\",	/* Set icon label */
	*s_label = "\033]l%s\033\\",	/* Set top stripe label */
	*s_icon = "\033]I%s\033\\";	/* Set icon */

int	checked = 0;		/* Mail check commanded (via SIGINT) */
jmp_buf	here;			/* Where to return to for signals */

	/* Usage message */
char	*Usage = "Usage: mailwatch [-a] [-c] [-e] [-o]\n\
	[-i emptymboxicon] [-I fullmboxicon]\n\
	[-l emptymboxlabel] [-L fullmboxlabel]\n\
	[-m mailreader] [-t check-time]\n";

/*
 *	Simple things
 */
#define	seticon(I)	printf(s_icon, I)
#define	seticonlabel(S)	printf(i_label, S)
#define	setstripe(S)	printf(s_label, S)
#define	putstr(S)	fputs(S, stdout)

#define	CLOSED	0	/* Window is closed */
#define	OPEN	1	/* Window is open */

main(argc, argv)
int	argc;
char	**argv;
{
	int	state = OPEN;	/* Current window status */
	int	catcher();	/* Signal catcher */

	setbuf(stdout, (char *)NULL);
	init(argc, argv);
	(void) signal(SIGINT, catcher);
	(void) signal(SIGQUIT, catcher);

	setjmp(here);
	for (;;) {
		if (filesize(MailBox) > 0) {
			state = OPEN;
			seticon(FullMboxIcon);
			seticonlabel(FullMboxLabel);
			setstripe("      You have mail.");
			if (Expose)
				putstr(e_window);
			if (AutoOpen)
				putstr(o_window);
			if (Ask) {
				char	buf[80];	/* Input buffer */

				putstr("\nPress RETURN to read mail");
				fflush(stdout);
				tossinput();
				(void) fgets(buf, 80, stdin);
			}
			if (Clear)
				putstr(clr_window);
			(void) system(MailReader);
		}
		else {
			if (state == OPEN) {
				putstr(c_window);
				seticon(EmptyMboxIcon);
				seticonlabel(EmptyMboxLabel);
				setstripe("      Waiting for mail.");
				state = CLOSED;
			}
			if (checked) {
				putstr("No mail.\n");
				checked = 0;
			}
			tsleep(CheckTime);
		}
	}
}

/*
 *	filesize - get size of file
 *
 *	Returns: size of file or -1 if file not found
 */
filesize(file)
char	*file;		/* File to size */
{
	struct stat s;	/* File status */

	if (stat(file, &s) < 0)
		return(-1);
	return(s.st_size);
}

/*
 *	catcher - signal catcher
 */
catcher(sig)
int	sig;	/* Signal which got us here */
{
	if (sig == SIGQUIT) {
		putstr("\nQuit\n");
		exit(1);
	}
	checked = 1;		/* Set user check flag */
	longjmp(here, 1);
}

/*
 *	tossinput - flush pending terminal input
 */
tossinput()
{
	tsleep(2);
	ioctl(0, TIOCFLUSH, 0);
}


/*
 *	tsleep -- sleep for <time> seconds or until terminal
 *		is ready for input.
 */
tsleep(time)
int	time;		/* Seconds to sleep */
{
	struct timeval	kw;	/* Select timeout */
	int	imask;	/* Input fd mask */ /* XXX SELECT TYPE */

	kw.tv_sec = time;
	kw.tv_usec = 0;
	imask = (1 << fileno(stdin));
	return(select(fileno(stdin)+1, &imask, (int *)0, (int *)0, &kw));
}

/*
 *	init - process arguments and initialize options
 *
 * Arguments are:
 * a = auto open
 * c = clear before mailreader exec
 * e = expose window when mail arrives
 * i fn = empty icon
 * I fn = full icon
 * l str = empty label
 * L str = full label
 * t time = sleep time
 * m prog = mail-reader
 */
init(argc, argv)
int	argc;		/* Arg count from main() */
char	**argv;		/* Arg vector from main() */
{
	extern int	optind;		/* From getopt() */
	extern char	*optarg;	/* From getopt() */
	char	*p;		/* Temp */
	int	argchar;	/* Temp */
	char	*optstring = "acei:I:l:L:t:m:o?";	/* Option list */

	extern char	*getenv();

	OnSun = (int )getenv("WINDOW_ME");
	/*
	 * If running under suntools, ignore stop signals.
	 * Actually this is desirable ONLY if running directly under
	 * shelltool, but there is no easy way to figure that out.
	 */
	if (OnSun)
		(void) signal(SIGTSTP, SIG_IGN);

	/*
	 * If TERM != sun, all bets are off!
	 */
	p = getenv("TERM");
	if (strncmp(p, "sun", 3) && strcmp(p, "Mu")) {
		printf("Warning: TERM not set to 'sun'.\n");
		printf("\tMailwatch will only work in SUN tty emulator window\n");
	}

	if ((MailBox = getenv("MAIL")) == (char *)0) {
		fprintf(stderr, "No mailbox\n");
		exit(1);
	}

	while ((argchar = getopt(argc, argv, optstring)) != EOF) {
		switch(argchar) {
		case 'a':	Ask = 1; break;
		case 'c':	Clear = 1; break;
		case 'e':	Expose = 1; break;
		case 'i':	EmptyMboxIcon = optarg; break;
		case 'I':	FullMboxIcon = optarg; break;
		case 'l':	EmptyMboxLabel = optarg; break;
		case 'L':	FullMboxLabel = optarg; break;
		case 'm':	MailReader = optarg; break;
		case 'o':	AutoOpen = 1; break;
		case 't':	CheckTime = atoi(optarg); break;
		case '?':	printf(Usage); exit(1);
		}
	}
	if (filesize(EmptyMboxIcon) < 0) {
		fprintf(stderr, "Can't find empty mailbox icon %s\n",
			EmptyMboxIcon);
		exit(1);
	}
	if (filesize(FullMboxIcon) < 0) {
		fprintf(stderr, "Can't find full mailbox icon %s\n",
			FullMboxIcon);
		exit(1);
	}
/*
 *	printf("Mailwatch starting. 
 *		Will check mail every %d seconds\n
 *		SIGINT to check mail\n
 *		SIGQUIT to exit\n", CheckTime);
 */
}
