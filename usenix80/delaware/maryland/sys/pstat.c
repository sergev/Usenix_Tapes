#define PRNAME  "/dev/xml0"     /* raw X-press line printer */

main(argc, argv)
int argc;
char *argv[];
{
int print;

if ((argc > 1) && (argv[1][0] == '-'))
	print = 0;	/* silent mode */
else
	print = 1;

if (open(PRNAME, 1) < 0) {
	if (print)
		printf("The printer is not available.\n");
	exit(1);        /* ``false'' for ``if'' command */
	}
else {
	if (print)
		printf("The printer is available.\n");
	exit(0);        /* ``true'' for ``if'' command */
	}
}
