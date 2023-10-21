/*	SCAME main.c				*/

/*	Revision 1.0.2	1985-02-23		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

#include "scame.h"
#ifdef BSD42
# include <sys/file.h>
#endif


/* global variables */
struct bufstruct buftab[MAXBUFS], cb = {
	"Main",
	"gazonk",
	0,
	0,
	FUNDAMENTAL,
	FALSE,
	(time_t) 0,
	{ 0L },
	{ -1, 72 }
};

struct gvarstruct gvars = {
		500,		/* Auto Push Point Option */
		100,		/* Bottom Display Margin */
		  0,		/* Cbreak Mode */
		 33,		/* Comment Column */
		  0,		/* Hackmatic */
	MAILCHECKINTERVAL,	/* Mail Check Interval */
		 17,		/* Quote Char (^Q) */
		 18,		/* Search Backward (^R) */
		 27,		/* Search Exit Char (^@) */
		 19,		/* Search Forward (^S) */
		  0,		/* System Output Holding */
		  0,		/* Tab Self Insert */
		  0		/* Top Display Margin */
};

int cmdchar;
int noofbufs = 1;
int bufno = 0;
int oldbuf = -1;
int otherbuf;			/* Buffer of other window */
char *buf;
char currentdir[FILENAMESIZE];
char mailfile[FILENAMESIZE];
char *username;
char tempfile[] = TMPFILE;
char scamelib[] = SCAMELIB;
char *modes[] = {
	"Ada",
	"C",
	"Fundamental",
	"Lisp",
	"Pascal",
	"Swedish"
};
char more[]="--MORE--",
     killbuffile[FILENAMESIZE],
     kbdmacfile[FILENAMESIZE],
     screen[SCRDIMY][SCRDIMX+1],
     oldscreen[SCRDIMY][SCRDIMX+1],
     commandprompt[80]="",
     fillprefix[80]="",
     tmpmode_string[80]="";
char *dot, *z, *home, *oldhome, *away, *mark[16], *otherdot, *otherhome;
char *gaps, *gape;
int     upcasearr[512], nupcasarr[512];
long bufsize;
int cury, curx, linpos;
int screenlen,screenwidth;
int windows=1;			/* Number of windows */
int wintop, winbot;		/* Top and bottom of window */
int oldhpos;			/* used for moving up or down */
Bool savehpos=FALSE;
Bool	killing,
	control_prefix = FALSE,
	meta_prefix = FALSE,
	echobusy = FALSE,
	updateflag = FALSE,
	typing = FALSE, savearg = FALSE;
long	arg;
Bool	xarg,xxarg;		/* xarg  => argument has been given	 */
				/* xxarg => explicit arg has been given, */
				/* 	not only C-U			 */
int pipeup[2], pipedown[2];
int uid;			/* user id. */
int ppid;			/* parent process id. */
char lastinput[LASTINPUTSIZE];
int lstindex = 0;
Bool	defining_kbd_mac = FALSE,
	quiet = FALSE;
int	kbdmfd, execfd = -1;
long	chkmailcnt;
int	recursive_level = 0;
Bool	pop_pending = FALSE;
#ifdef SIGTSTP
char lockfile[FILENAMESIZE];
#endif


main(argc,argv)
int argc;
char **argv;
{
/*
Bool updflg;
*/

  initupcase();
  ppid = getppid();
  uid = getuid();
  if (!isatty(0)) {
	printf("Sorry, but SCAME must communicate directly with a terminal\n");
	exit(1);
  }
  if (getenv("TERM") == NIL) {
		printf("You haven't set the terminal type, please do\
 \"man scame\"\n");
 	exit(1);
  }
#ifdef SIGTSTP
  { int fd;
	sprintf(lockfile, tempfile, ppid, uid);
	strcat(lockfile, "S");
	if ((fd=open(lockfile, 0)) >= 0
#ifdef BSD42
		&& flock(fd, LOCK_EX | LOCK_NB) != 0
#endif
		) {
		printf("You have Scame already running somewhere\n");
		exit(1);
	}
	fd = creat(lockfile, 0600);
#ifdef BSD42
	flock(fd, LOCK_EX | LOCK_NB);
#endif
  }
#endif
  getwd(currentdir);
  free(malloc(0x80));		/* For getchar(). */
  /*
   * Make sure at least one byte gets allocated.
   */
  buf=sbrk(1);			/* Obtain the startaddress of the buffer */
  bufsize = 0;
  dot=home=z=otherdot=otherhome=gaps=gape=buf;
  away=NIL;
  oldhome= NIL;
#ifdef PDP11
  inittypeaheadcheck();
#endif
  gotobegin();
  for (lstindex = LASTINPUTSIZE; lstindex > 0;)
	lastinput[--lstindex] = 0200;
  gettermtype();
  setupterm(TRUE);
  wintop = 0;
  winbot = screenlen-3;
  sprintf(killbuffile, tempfile, ppid, uid);
  strcat(killbuffile, "k");
  sprintf(kbdmacfile, tempfile, ppid, uid);
  strcat(kbdmacfile, "m");
#ifdef BSD42
   username = getenv("USER");
#else
   username = getenv("LOGNAME");
#endif
  sprintf(mailfile, "%s/%s", MAILDIR, username);
  checkmail(FALSE);

if ((argc == 1 || argv[1][0] != '-') && restart()) {
	ttycbreak();
	cls();
	while (--argc > 0) {
		if (argv[argc][0] != '-') {
			buildfilename(cb.filename,argv[argc]);
			findfile();
			fixbuftab(PUT);
		}
	}
}
else {
int i;
	cls();
	execfile(".scamerc");
	i=1;
	while (argc > i && argv[i][0] == '-') i++;
	if (argc > i) buildfilename(cb.filename,argv[i]);
	findmode();
	readfile();
	cb.modified=FALSE;
	while (--argc > i) {
		if (argv[argc][0] != '-') {
			fixbuftab(PUT);
			buildfilename(cb.filename,argv[argc]);
			findfile();
		}
	}
}
killing=FALSE;
*mark=buf;

  editloop();
  exitscame(0);
}	/* end of main */


exitscame(code)
int code;
{
	if (!code) {
		savescame();
		if (CE != NIL) {
			cur(screenlen-2,0);
			cleol(FALSE);
		}
		cur(screenlen-1,0);
		setupterm(FALSE);
		pchar('\n');
	}
#ifdef SIGTSTP
	unlink(lockfile);
#endif
	exit(code);
}


editloop()
{
int c;
	if (execfd < 0) recursive_level++;
	update();
	if (!quiet) {
		modeline();
		refresh(TRUE);
	}
/*	cur(cury,curx); */
	oldhpos = curx;
	while (!pop_pending) {
		if (control_prefix || meta_prefix) {
			if (xarg) sprintf(commandprompt,"%ld ",arg);
			else *commandprompt = '\0';
			if (control_prefix) strcat(commandprompt, "C-");
			if (meta_prefix) strcat(commandprompt, "M-");
		}
		else if (!savearg) {
			arg=1;
			xarg=xxarg=FALSE;
			*commandprompt = '\0';
		}
		savearg = FALSE;
		c=inchar(TRUE);
		if (c == EOF) return;
		if (c & 0200) meta_prefix=TRUE;
		if ((c & 0177) == '\007') {
			if (!quiet) pchar(BELL);
			control_prefix = meta_prefix = FALSE;
			if (!quiet && echobusy) clearecho();
		}
		else {
			if ((c & 0177) >= 0 && (c & 0177) < 32)
				c = (c + '@') | 0400;
			if (control_prefix) c = b_control(c);
			if (meta_prefix) c = b_meta(c);
			control_prefix = meta_prefix = FALSE;
fflush(stdout);
			cmdchar = c;
			(*disptab[cmdchar])();
		}
		if (typing) {
			int ch;
			typing = FALSE;
			*commandprompt = '\0';
			ch = inchar(TRUE);
			if (ch == EOF) return;
			if (execfd < 0 && ch != ' ')
				unget(ch);
		}
		if (updateflag || dot<home || dot>away || home != oldhome) {
			update();
			modeline();
		}
		else findxy();
		if (!(pop_pending || quiet)) refresh(TRUE);
/*
		if (!typeahead() && (cury != vpos || curx != hpos))
			cur(cury,curx);
		if (c!= '\007' && c!= '\016' && c != '\020' && c!= '\0207')
*/
		if (!savehpos)
			oldhpos = curx;
		else
			savehpos = FALSE;
		if (!(quiet || echobusy || --chkmailcnt)) checkmail(TRUE);
		echobusy = FALSE;
	}
	if (execfd < 0) recursive_level--;
	pop_pending = FALSE;
}



recurse()
{
char tstr[SCRDIMX+1];
	strcpy(tstr, tmpmode_string);
	*tmpmode_string = '\0';
	editloop();
	tmodlin(tstr);
}

