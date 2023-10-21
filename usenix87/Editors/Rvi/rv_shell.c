#include "rv.h"
#include <signal.h>

#ifndef USG
#  define strchr index
#endif

/*
 * This entire module is a hack
 */

extern boolean	scrolled;

extern char *strchr();

rv_shell(cmd)
char *cmd;
{
	INT cpid, len, infd, outfd;
	char buf[513], *s;
	int (*prev_func)();
#ifdef USG
	struct termio tm;
#endif
	
	xmit_sync();
	(void) recv_sync(FALSE);
	xmit_ed("!sh -c '%s' ; echo '@''$''#'\n", cmd);
	fflush(file.fi_fpout);
	if (file.fi_modified)
		botprint(FALSE, "[No write since last change]");
	move(LINES-1, 0);
	rv_fscroll(1);
	move(LINES-1, 0);
	refresh();

	/* prev_func = signal(SIGINT, SIG_IGN);  Use at your own risk */
	if (use_linemode) {
#ifdef USG
		nocbreak();
#else
		nocrmode();
#endif
		nl();
		echo();
#ifdef TCGETA             /* Echo doesn't in System V, sigh */
		ioctl(0, TCGETA, &tm);
		tm.c_lflag |= ECHO;
		ioctl(0, TCSETA, &tm);
#endif
		if (set_debug > 2)
			printf("Using line mode.\n");
	}
	infd = atoi(Argv[1]);
	outfd = atoi(Argv[2]);
	if ((cpid = fork()) < 0)
		panic("Can't fork");
	if (cpid == 0) { /* Child */
		for (;;) {
			if ((len = read(0, buf, 512)) < 0)
				_exit(0);
			if (len == 0)
				write(outfd, "\004\n", 2);
			else
				write(outfd, buf, len);
		}
		/*NOTREACHED*/
	}
	for (;;) { /*  Parent */
		if ((len = read(infd, buf, 512)) <= 0)
			panic("EOF on read from pipe");
		if ((s = strchr(buf, '@')) != NULL) {
			if (s-buf+1 >= len) {
				write(1, buf, len);
				len = read(infd, buf, 512);
				s = buf;
			}
			if (*++s == '$') {
				if (s-buf+1 >= len) { 
					write(1, buf, len);
					len = read(infd, buf, 512);
					s = buf;
				}
				if (*++s == '#') {
					if (s-buf >= 3)
						write(1, buf, s-buf-2);
					kill(cpid, 9);
					while (wait((int *)0) != cpid)
						;
					break;
				}
			}
		}
		write(1, buf, len);
	}
	if (use_linemode) {
#ifdef USG
		cbreak();
#else
		crmode();
#endif
		nonl();
		noecho();
	}
	/* (void) signal(SIGINT, prev_func);  Use at your own risk */
	refresh();
	scrolled = TRUE;
	hitcr_continue();
	clearok(curscr, TRUE);
	redraw_screen((struct li_line *)0);
	move_cursor(screen.sc_lineno, screen.sc_column);
}
