/*
 * TREK73: main.c
 *
 * Originally written (in HP-2000 BASIC) by
 *	William K. Char, Perry Lee, and Dan Gee
 *
 * Rewritten in C by
 *	Dave Pare (sdcsvax!sdamos!mr-frog)
 * 		  (mr-frog@amos.ling.ucsd.edu)
 *		and
 *	Christopher Williams (ucbvax!ucbmerlin!williams)
 *			     (williams@merlin.berkeley.edu)
 *
 * Corrected, Completed, and Enhanced by
 *	Jeff Okamoto	(ucbvax!okamoto)
 *			(okamoto@cory.berkeley.edu)
 *	Peter Yee	(ucbvax!yee)
 *			(yee@ucbarpa.berkeley.edu)
 *	Matt Dillon	(ucbvax!dillon)
 *			(dillon@berkeley.edu)
 *	Dave Sharnoff	(ucbvax!ucbcory!muir)
 *			(muir@cogsci.berkeley.edu)
 *	Joel Duisman	(ucbvax!duisman)
 *			(duisman@miro.berkeley.edu)
 *	    and
 *	Roger J. Noe    (riccb!rjnoe)
 *			(ihnp4!riccb!rjnoe@berkeley.edu)
 *
 * Main Loop
 *
 * main, alarmtrap, quitgame, buffering
 *
 */

#include "externs.h"
#include <signal.h>
#include <setjmp.h>

static jmp_buf	jumpbuf;


main(argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{
	register char		*env;
	int			alarmtrap();
	int			quitgame();

	if (buffering(stdout) < 0)
		perror("cannot fstat stdout");
	(void) signal(SIGALRM, alarmtrap);
	(void) signal(SIGINT, quitgame);
	srandom(time(0));
	time_delay = 30;
	set_save();
	if (argc > 1)
		if (argv[1][0] != '-') {
			restore(argv[1], envp);	/* Will not return */
			exit(1);
		}
	options = getenv("TREK73OPTS");
	if (options != NULL)
		parse_opts(options);
	get_comlineopts(argc, argv);
	if (restart && savefile[0] != '\0') {
		restore(savefile, envp);	/* Will not return */
		exit(1);
	}
	name_crew();
	init_ships();
	(void) mission();
	(void) alert();
	playit();
}
	/*
	 * Main loop
	 */
playit()
{
	struct cmd		*scancmd();
	int			alarmtrap();
	int			quitgame();
	register struct ship	*sp;
	char			buf1[30];
	struct cmd		*cp;
	int			loop;

	(void) setjmp(jumpbuf);
	sp = shiplist[0];
	if (!(is_dead(sp, S_DEAD))) {
next:
		for (loop = 0; loop < HIGHSHUTUP; loop++)
			shutup[loop] = 0;
		fflush(stdin);
		printf("\n%s: Code [1-%d] ", captain, high_command);
		fflush(stdout);
		(void) alarm((unsigned) time_delay);
		if (gets(buf1) != NULL) {
			(void) alarm(0);
			cp = scancmd(buf1);
			if (cp != NULL) {
				(*cp->routine)(sp);
				if (cp->turns == FREE)
					goto next;
			} else
				printf("\n%s: %s, I am unable to interpret your last utterance.\n", science, title);
		} else
			(void) alarm(0);
	}
	alarmtrap(0);
	/* This point is never reached since alarmtrap() always concludes
	   with a longjmp() back to the setjmp() above the next: label */
	/*NOTREACHED*/
}

alarmtrap(sig)
int sig;
{
	register int i;

	if (sig) {
		puts("\n** TIME **");
		(void) signal(sig, alarmtrap);
		stdin->_cnt = 0;
	}
	for (i = 1; i <= shipnum; i++)
		shiplist[i]->strategy(shiplist[i]);
	if (!(is_dead(shiplist[0], S_DEAD)))
		printf("\n");
	(void) move_ships();
	(void) check_targets();
	(void) misc_timers();
	(void) disposition();
	longjmp(jumpbuf, 1);
}


quitgame()
{
	char answer[20];
	unsigned timeleft;

	timeleft = alarm(0);
	(void) signal(SIGINT, SIG_IGN);
	puts("\n\nDo you really wish to stop now?  Answer yes or no:");
	if(gets(answer) == NULL || answer[0] == 'y' || answer[0] == 'Y')
		exit(0);
	(void) signal(SIGINT, quitgame);
	if(timeleft)
		(void) alarm((unsigned)timeleft);
	return;
}


/* buffering: Determine whether or not stream is to be buffered.  If it's a
   character-special device, any buffering in effect will remain.  If it's not
   a character-special device, then stream will be unbuffered.  There are many
   ways to decide what to do here.  One would have been to make it unbuffered
   if and only if !isatty(fileno(stream)).  This is usually implemented as a
   single ioctl() system call which returns true if the ioctl() succeeds,
   false if it fails.  But there are ways it could fail and still be a tty.
   Then there's also examination of stream->_flag.  UNIX is supposed to make
   any stream attached to a terminal line-buffered and all others fully buf-
   fered by default.  But sometimes even when isatty() succeeds, stream->_flag
   indicates _IOFBF, not _IOLBF.  And even if it is determined that the stream
   should be line buffered, setvbuf(3S) doesn't work right (in UNIX 5.2) to
   make it _IOLBF.  So about the only choice is to do a straightforward
   fstat() and ascertain definitely to what the stream is attached.  Then go
   with old reliable setbuf(stream, NULL) to make it _IONBF.  The whole reason
   this is being done is because the user may be using a pipefitting program
   to collect a "transcript" of a session (e.g. tee(1)), or redirecting to a
   regular file and then keeping a tail(1) going forever to actually play the
   game.  This assures that the output will keep pace with the execution with
   no sacrifice in efficiency for normal execution.	[RJN]		*****/

#include <sys/types.h>
#include <sys/stat.h>

int
buffering(stream)
FILE	*stream;
{
	struct stat	st;
	if (fstat(fileno(stream), &st) < 0)
		return -1;
	if ((st.st_mode & S_IFMT) != S_IFCHR)
		setbuf(stream, NULL);
	return 0;
}
