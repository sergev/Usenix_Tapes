/*
 *  batch [-v] [-Pn] queue [file ...]
 */
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

char grade = 'q';	/* Priority of this job */
int vflag;
extern errno;
char *getenv();
char *index();
enum { BOURNE, CSH } theshell;
char *deleteerr;
char *tffmt = "%s/%.14s/tf%c%D";
char *cffmt = "%s/%.14s/cf%c%D";
#define	SPOOLDIR	"/usr/spool/batch"
char Pid[] = "/usr/spool/pid/batchd";

main(argc, argv, arge)
char **argv;
char **arge;
{
	int i, j;
	register char **ep;
	char *queue = NULL;
	char tfname[256];
	char cfname[256];
	long unique();
	long spoolnumber;
	char *shell;
	FILE *tf;
	char dir[MAXPATHLEN];
	struct stat sb;
	int cleanup();

#ifndef DEBUG
	if(geteuid() != 0)
		error("System error-Effective uid is %d, not root\n", getuid());
#endif
	/*
	 *  Find and validate the grade.  a-z, only SU can set it to values
	 *  lower than 'h'.
	 */
	for(i = 1 ; i < argc ; i++) {
		if(argv[i][0] == '-')
			switch(argv[i][1]) {
			case 'P':
				grade = argv[i][2];
				break;
			case 'v':
				vflag = 1;
				break;
			default:
				Usage();
			}
	}
	if(grade < 'a'  ||  grade > 'z'  ||  (grade < 'h')  &&  getuid())
		error("Grade `%c' is invalid.\n", grade);
	/*
	 *  First non - arg is the queue.
	 */
	for(i = 1 ; i < argc ; i++)
		if(argv[i][0] != '-') {
			queue = argv[i];
			break;
		}
	if(queue == NULL)
		Usage();
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, cleanup);
	if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, cleanup);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, cleanup);
	sprintf(tfname, "%s/%.14s", SPOOLDIR, queue);
	if(stat(tfname, &sb) < 0  ||
	   (errno=ENOENT, (sb.st_mode & S_IFMT) != S_IFDIR)) {
		if(errno == ENOENT)
			error("%s: No such queue\n", queue);
		else {
			perror(tfname);
			error("%s: System error\n", tfname);
		}
	}
	/*
	 *  Spool file name starts with tf, followed by grade, followed by
	 *  a unique sequential number.  The files are sorted by this name
	 *  for priority of execution.
	 */
	spoolnumber = unique();
	sprintf(tfname, tffmt, SPOOLDIR, queue, grade, spoolnumber);
	deleteerr = tfname;
	umask(0077);
	if((tf = fopen(tfname, "w")) == NULL) {
		perror(tfname);
		error("%s: System error\n", tfname);
	}
	/*
	 *  Since it will be directly executable by the system put out a
	 *  system magic word followed by the shell.
	 *  THESHELL tells which type of environment setting to use.
	 */
	if((shell = getenv("SHELL")) == NULL)
		shell = "/bin/sh";
	if(strcmp(shell, "/bin/sh") == 0)
		theshell = BOURNE;
	else if(strncmp(shell+strlen(shell)-3, "csh", 3) == 0)
		theshell = CSH;
	else
		error("%s: Unrecognized shell in $SHELL variable\n", shell);
	fprintf(tf, "#!%s\n", shell);
	/*
	 *  Set everything up so that it will execute in the same env. as we
	 *  have right now.  BUG::::  Since we're root, we can get the current
	 *  directory, but since the chdir back here is done by the user prog
	 *  later that could fail if he didn't have full perms to get to the
	 *  current location.   We should at very least mention this to the user
	 */
	if(getwd(dir) == NULL)
		error("getwd: System error: %s\n", dir);
	if( theshell == CSH )
		fprintf(tf,"set histchars\n");
	fprintf(tf, "cd ");
	putstring(tf, dir);
	fprintf(tf, "\n");
	/*
	 *  Dump the current environment variables.  Ignore them if they
	 *  don't have = signs, throw away TERMCAP and TERM.
	 */
	for(ep = arge ; *ep ; ep++) {
		register char *p;

		if((p = index(*ep, '=')) == NULL)
			continue;
		*p++ = 0;
		if(strcmp(*ep, "TERMCAP") == 0)
			continue;
		if(strcmp(*ep, "TERM") == 0)
			continue;
		putenv(tf, *ep, p);
	}
	/*
	 *  If using csh, set $home so the ~ character expands correctly.
	 */
	if(theshell == CSH)
		fprintf(tf, "set home=$HOME\n");
	/*
	 *  Dump the files specified; or stdin otherwise.
	 */
	j = 0;
	for(i++ ; i < argc ; i++) {
		FILE *input;

		if(argv[i][0] == '-')
			continue;
		if((input = fopen(argv[i], "r")) == NULL) {
			perror(argv[i]);
			cleanup();
		}
		concat(tf, input);
		++j;
	}
	if(j == 0)
		concat(tf, stdin);
	/*
	 *  Set everything up for direct execution; and then link it to
	 *  its real filename.
	 */
	fchown(fileno(tf), getuid(), getgid());
	fchmod(fileno(tf), 0700);
	fclose(tf);
	sprintf(cfname, cffmt, SPOOLDIR, queue, grade, spoolnumber);
	if (rename(tfname, cfname) < 0) {
		perror(tfname);
		error("%s: System error\n", tfname);
	}
	/*
	 * Send SIGALRM to the batchd to wake it up.
	 */
	if ((tf = fopen(Pid, "r")) != NULL) {
		if (fscanf(tf, "%d", &i) == 1 && i > 0)
			(void) kill(i, SIGALRM);
		fclose(tf);
	}
	if(vflag)
		printf("%s\n", cfname);
	exit(0);
}

/*VARARGS1*/
error(s)
char *s;
{
	fprintf(stderr, "batch: %r", &s);
	cleanup();
}

cleanup()
{
	if(deleteerr != NULL)
		unlink(deleteerr);
	exit(1);
}

concat(to, from)
	register FILE *to, *from;
{
	register int c;

	while ((c = getc(from)) != EOF)
		putc(c, to);
}

/*
 *  Dump out an environment variable in the format for this shell.
 */
putenv(tf, ep, p)
FILE *tf;
char *ep, *p;
{
	switch(theshell) {
	case BOURNE:
		fprintf(tf, "%s=", ep);
		putstring(tf, p );
		fprintf(tf, "; export %s\n", ep);
		break;
	case CSH:
		fprintf(tf, "setenv %s ", ep);
		putstring(tf, p );
		fprintf(tf, "\n");
		break;
	}
}

/*
 *  God help us; put out a string with whatever escapes are needed to make
 *  damn sure that the shell doesn't interpret it.  BOURNE shell doesn't
 *  need escapes on newlines; CSH does.
 */
putstring(tf, s)
register FILE *tf;
register char *s;
{
	fputc('\'', tf);
	while(*s) {
		if(*s == '\n' && theshell == CSH )
			fputc('\\', tf);
		if(*s == '\'')
			s++, fprintf(tf,"'\"'\"'"); /* 'You can'"'"'t come' */
		else
			fputc(*s++, tf);
	}
	fputc('\'', tf);
}

Usage()
{
	error("Usage: batch [-Pn] queue [file ...]\n");
}
