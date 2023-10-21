/*
 *	main.c
 *
 *	main routine for ic
 */

# include	<setjmp.h>
# include	<signal.h>
# include	<stdio.h>
# include	"ic.h"

jmp_buf	jmpint;

char femess[] =  "Floating Exception\n";

main (argc, argv)
char	**argv;
{
	int	intr(), ferr();

	initbuiltin ();
	switch (setjmp (jmpint)) {
	case 2:
		fprintf (stderr, femess);
	case 0:
		signal (SIGINT, intr);
		signal (SIGFPE, ferr);
		while (*++argv)
			parsefile (*argv);
		break;
	case 1:
		putchar ('\n');
		break;
	}
	switch (setjmp (jmpint)) {
	case 0:
		break;
	case 1:
		putchar ('\n');
		break;
	case 2:
		fprintf (stderr, femess);
		break;
	}
	signal (SIGINT, intr);
	signal (SIGFPE, ferr);
	yyparse ();
}

intr ()
{
	int	intr();
	signal (SIGINT, intr);
	longjmp (jmpint, 1);
}

ferr()
{
	int	ferr();
	signal (SIGFPE, ferr);
	longjmp (jmpint, 2);
}

parsefile (s)
char *s;
{
	if (lexfile (s)) {
		yyparse ();
		lexstdin ();
	}
}
