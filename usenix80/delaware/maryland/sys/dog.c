#

/*
 *      dog [-pgsize] [-t] [command [arguments] ]
 *
 *	executes the command, pausing after every pgsize-1
 *	lines of standard output.
 *	If the command is not found, then the command and
 *	its arguments are taken to be files that are meant
 *	to be 'cat'ed to the terminal.  Thus, dog can be
 *	used instead of cat.
 *	If there are no arguments, dog reads from standard
 *	input; it can therefore be used as a filter.
 *	Default is pgsize=24.
 *
 *	Written 6/77 by Steve Eisen.
 *
 *      Modified: July 3, '79 by Fred Blonder, to handle the
 *                                           Tektronix 4013.
 *
 *      Modified: February 19, '80 to handle nulls, line overflow and
 *              formfeeds. F.L.B.
 *
 */

#define DEFAULT 24
#define LINELEN 80
#define FF      014

struct {
	int fildes;
	int nleft;
	char *nextp;
	char buff[512];
	} buf[1];

char	*args[100];
char	string[10000];
int tek_flag 0;
extern int fout;

main(argc, argv)
char *argv[];  {
	int len, count;
	register char *p;
	int	i, fildes[2];
	char	**argp, *strp, c;
	register int llen, lcount;

	len = DEFAULT;
	llen = LINELEN;

	while (argc > 1 && argv[1][0] == '-')  {
		switch (argv[1][1]) {
			case 't': case 'T':     /* Tektronix mode */
				tek_flag++;
				len = 35;
				break;

			default:
				len = atoi(&argv[1][1]);
			}
		argc--;
		argv++;
		}

	if (argc < 2) goto nargs;
	pipe(fildes);
	if (fork() == 0)  {
		close(1);
		dup(fildes[1]);
		close(fildes[0]);
		close(fildes[1]);
		argc--;
		argv++;
		argp = &args[1];
		strp = string;
		for (i=0; i<9; i++)
			*strp++ = "/usr/bin/"[i];
		for (i=0; i<argc; i++)  {
			*argp++ = strp;
			p = *argv++;
			while (*strp++ = *p++);
		}
		*argp = 0;
		execv(string+9, &args[1]);
		execv(string+4, &args[1]);
		execv(string, &args[1]);
		args[0] = "cat";
		execv("/bin/cat", args);
		puts("/bin/cat: not found");
	}
	close(0);
	dup(fildes[0]);
	close(fildes[0]);
	close(fildes[1]);

nargs:
	if (tek_flag) {
		fout = 2;
		printf("\033\014");
		sleep(1);
		fout = 1;
		}

	len--;

	count = lcount = 0;

	while ((c = getc(buf)) >= 0) {

		if (c == FF)
			count = 999;
		else {
			putchar(c);
			if ((c == '\n') || (++lcount >= llen)) {
				lcount = 0;
				count++;
				}
			}

		if (count >= len) {
			fout = 2;
			putchar(':');
			do  {
				read(2, &c, 1);
				}  while (c != '\n');
			if (tek_flag) {
				printf("\033\014");
				sleep(1);
				}
			fout = 1;
			count = lcount = 0;
			}
		}

}

puts(as)
char *as;  {
	register char *sp;

	sp = as;
	while (*sp)
		putchar(*sp++);
	putchar('\n');
}
