
/*
 * rev 10-2-86 Added command save on error, !, ?
 * rev 7-28-85 select array, command prompt, -f flag
 * rev 10-8-80 CAF cbreak & backup
 * rev 10-4-80 CAF for stdio
 *  Based on the "pic" program Copyright (c) 1976 Thomas S. Duff
 */

/*% cc -O -K -DDUP2 -o choos % -lx
 */

/*#define DUP2	/* use the dup2 call */

#include <stdio.h>
#include <sgtty.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

struct sgttyb oldtty, newtty;
int Filesonly = 0;
int Nathan = 0;
int Selcount = 0;
char Tempname[] = "/tmp/choosXXXXXX";
char Command[256];

onintr()
{
	ioctl(0,TIOCSETP,&oldtty);
	exit(1);
}
main(argc, argv)
char *argv[];
{
	register i, c;
	register char *s;
	register FILE *fout;
	register char *select;
	char *calloc();
	struct stat e;
	char *shell = "/bin/sh";
	char *getenv();

	if ((select = calloc(argc+1, 1)) == (char *) 0) {
		fprintf(stderr, "choos: can't alloc memory!");
		exit(1);
	}
	if ( !strcmp(argv[1], "-f")) {
		--argc; ++argv; ++Filesonly;
	}
	if (argc < 2) {
		prompt(); exit(1);
	}
	if ((s = getenv("SHELL")) && *s)
		shell = s;

	signal(SIGINT, onintr);
	signal(SIGQUIT, onintr);
	signal(SIGTERM, onintr);
	signal(SIGPIPE, onintr);
	ioctl(0,TIOCGETP,&oldtty);
	newtty = oldtty;
	newtty.sg_flags |= CBREAK;
	ioctl(0,TIOCSETP,&newtty);


	fprintf(stderr, "Choos called with %d arguments\n", argc -1);
	for (i=1;;) {
		s = argv[i];
		fprintf(stderr,"%3d %14s %c? ", i, s, select[i]?'*':' ');
		if(Nathan)
			putc(Nathan,stderr);
		else
			c=getchar();
		if (c == '?') {
			prompt();
			fprintf(stderr, "\n%d of %d arguments selected\n",
			  Selcount,  argc -1);
			continue;
		}
		if(c=='*')
			Nathan = c ='y';
		if(c=='t')
			c ='y';
		if(c=='y') {
			if (Filesonly &&
			 (stat(s, &e) || (e.st_mode&S_IFMT) != S_IFREG)) {
				fprintf(stderr, " Not a regular file");
			}
			else if ( !select[i]) {
				select[i] = 1;  ++Selcount;
			}
		}
		if(c != '\n')
			putc('\n', stderr);
		if (c=='u' && select[i]) {
			select[i] = 0;  --Selcount;
		}
		if(c=='-' || c=='b') {
			if(i <= 2)
				i = argc;
			--i;
			continue;
		}
		if(c == '!') {
			fprintf(stderr, "Shell command: ");
			gets(Command);
			if (Command[0] == '\0')
				strcpy(Command, shell);
/*
 * To preserve functionality with "command `choos *` we must map
 * the stdio into stderr, then restore stdio upon return.  This
 * gets a bit messey without the dup2 call.
 */
#ifdef DUP2
			dup2(1, 19); dup2(2, 1);
			system(Command);
			dup2(19,1); close(19);
#else
			c = fork();
			if (c > 0)
				wait((int *)0);
			else if (c == 0) {
				close(1); dup(2); system(Command); exit(0);
			}
			else {
				fprintf(stderr, "Can't fork!\n");
				continue;
			}
#endif

			fprintf(stderr, "!\n"); continue;
		}
		if(c=='m' || c=='x') {
			if (Selcount)
				break;
			fprintf(stderr, "Nothing selected!\7\n");
		}
		if (++i == argc) {
			if (Nathan)
				break;
			i = 1;
		}
	}

	ioctl(0,TIOCSETP,&oldtty);

	if (isatty(1)) {
		Command[0] = Nathan = 0;
		while ( !Command[0]) {
			fprintf(stderr, "Command: ");
			gets(Command);
		}
		mktemp(Tempname);
		if ((fout = fopen(Tempname, "w")) == NULL) {
			perror("choos");
			exit(2);
		}
		for (s=Command; *s; ++s) {
			if (*s == '%') {
				Nathan = 1;
				putc(' ', fout);
				for(i=1;i!=argc;i++)
					if (select[i])
						fprintf(fout, "%s ", argv[i]);
			}
			else
				putc(*s, fout);
		}
		if ( !Nathan) {
			putc(' ', fout);
			for(i=1;i!=argc;i++)
				if (select[i])
					fprintf(fout, "%s ", argv[i]);
		}
		putc('\n', fout);
		fflush(fout);
		fclose(fout);
		sprintf(Command, "%s <%s", shell, Tempname);
		c = system(Command);
		if (c) {
			fprintf(stderr, "choos: Command exit status %d: Command is left in %s\n",
			 c, Tempname);
		} else
			unlink(Tempname);
	}
	else {
		for(i=1;i!=argc;i++){
			if (select[i])
				printf("%s\n", argv[i]);
		}
	}
	exit(0);
}
 
char *uprompt[] = {
 "\nchoos REV 11-2-86 by Chuck Forsberg, Omen Technology Incorporated",
 "	\"The High Reliability Communications Software\"\n",
 "command [-flags] `choos [-f] arguments` OR choos [-f] arguments",
 "	-f	Select only regular files (no directories, etc.)\n",
 "?	Repeat this message",
 "!	Shell Escape",
 "t	Tag (select) argument",
 "u	Untag argument",
 "CR	Skip argument",
 "b	Back up to last argument",
 "*	Select remaining arguments and execute",
 "x	Execute: Prompt for a command OR output the selected arguments.",
 "	Each '%' in the command is replaced by the selected arguments.",
 "	If no % appears in the command, they are added to the end of command.",
 "	The command is then executed by $SHELL (/bin/sh) >",
 ""
};
prompt()
{
	register char **p;

	for (p=uprompt; **p; ++p)
		fprintf(stderr, "%s\n", *p);
}

