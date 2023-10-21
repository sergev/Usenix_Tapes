L	[_A-Za-z]
D	[0-9]

%Start SKIP

%{
#define STACKSIZ	100
#define NTAB		1000

#define DEFINED		1
#define UNDEFINED	2
#define UNKNOWN		3

short *sp;
int changed;
%}

%%

^#[ \t]*ifdef[ \t]+{L}({L}|{D})*	{
			if (*sp & 4 ? (*sp & 2) == 0 : (*sp & 1) == 0)
				*++sp = 0;
			else
				switch (defined()) {
				case DEFINED:   *++sp = 1; changed++; break;
				case UNDEFINED: *++sp = 2; changed++; break;
				case UNKNOWN:   *++sp = 3; ECHO; break;
				}
			if (*sp != 3)
				BEGIN SKIP;
		}

^#[ \t]*ifndef[ \t]+{L}({L}|{D})*	{
			if (*sp & 4 ? (*sp & 2) == 0 : (*sp & 1) == 0)
				*++sp = 0;
			else
				switch (defined()) {
				case DEFINED:   *++sp = 2; changed++; break;
				case UNDEFINED: *++sp = 1; changed++; break;
				case UNKNOWN:   *++sp = 3; ECHO; break;
				}
			if (*sp != 3)
				BEGIN SKIP;
		}

^#[ \t]*if[ \t].*\n	{
			if (*sp & 4 ? (*sp & 2) == 0 : (*sp & 1) == 0)
				*++sp = 0;
			else
				switch (true()) {
				case DEFINED:	*++sp = 1; changed++; break;
				case UNDEFINED:	*++sp = 2; changed++; break;
				case UNKNOWN:	*++sp = 3; ECHO; break;
				}
		}

^#[ \t]*else.*\n	{
			if (*sp == 3)
				ECHO;
			*sp |= 4;
		}

^#[ \t]*endif.*\n	{
			if ((*sp & 3) == 3)
				ECHO;
			--sp;
		}

<SKIP>\n	{  BEGIN 0;  }

<SKIP>.		{  /* do nothing */;  }

.|\n		{
			if (*sp & 4 ? *sp & 2 : *sp & 1)
				ECHO;
		}

%%

#include <signal.h>

struct table {
	short t_flag;
	char *t_name;
} table[NTAB];

struct table *tabend;

short stack[STACKSIZ];

char tmpfil[] = "/tmp/rmifdXXXXXX";
char *cmd;
int overwrite, dontask;

main(argc, argv)
register char **argv;
{
	extern cleanup();

	tabend = &table[0]; sp = &stack[0]; *sp = 3;
	cmd = *argv;
	while (--argc > 0) {
		if (**++argv == '-') {
			switch (*++*argv) {
			case 'a': dontask++; break;
			case 'A': dontask = 0; break;
			case 'd':
			case 'D': defsym(++*argv); break;
			case 'u':
			case 'U': undefsym(++*argv); break;
			case 'w': overwrite++; break;
			case 'W': overwrite = 0; break;
			default:  error("unknown option"); break;
			}
		} else
			break;
	}
	if (overwrite) {
		signal(SIGINT, cleanup);
		signal(SIGQUIT, cleanup);
		signal(SIGHUP, cleanup);
		signal(SIGTERM, cleanup);
		mktemp(tmpfil);
	}
	if (argc == 0) {
		dontask = 1;
		yylex();
		exit(0);
	}
	while (argc > 0) {
		if ((yyin = fopen(*argv, "r")) == NULL)
			fprintf(stderr, "%s: cannot open %s\n", cmd, *argv);
		else {
			if (overwrite)
				yyout = fopen(tmpfil, "w");
			changed = 0;
			yylex();
			fclose(yyin);
			if (overwrite) {
				fclose(yyout);
				yyout = stdout;
				if (changed)
					copy(tmpfil, *argv);
				unlink(tmpfil);
			}
		}
		argv++;
		argc--;
	}
	exit(0);
}

defsym(sym)
char *sym;
{
	tabend->t_flag = DEFINED;
	tabend->t_name = sym;
	tabend++;
}

undefsym(sym)
char *sym;
{
	tabend->t_flag = UNDEFINED;
	tabend->t_name = sym;
	tabend++;
}

unknownsym(sym)
char *sym;
{
	tabend->t_flag = UNKNOWN;
	tabend->t_name = sym;
	tabend++;
}

defined()
{
	register char *s;
	register struct table *p;

	s = &yytext[yyleng];
	while (*--s > 32)
		;
	s++;
	for (p = &table[0]; p < tabend; p++)
		if (strcmp(p->t_name, s) == 0)
			return p->t_flag;
	if (dontask)
		return UNKNOWN;
	return ask(s);
}

ask(sym)
char *sym;
{
	register char *s;
	char buf[128];
	extern char *malloc(), *strcpy();

	fprintf(stderr, "is \"%s\" defined? ", sym);
	s = strcpy(malloc(strlen(sym)+1), sym);
	gets(buf);
	if (buf[0] == 'y' || buf[0] == 'Y') {
		defsym(s);
		return DEFINED;
	} else if (buf[0] == 'n' || buf[0] == 'N') {
		undefsym(s);
		return UNDEFINED;
	} else {
		unknownsym(s);
		return UNKNOWN;
	}
}

true()
{
	register char *s = yytext;
	char buf[128];

	if (dontask)
		return UNKNOWN;
	while (*s++ != 'f')
		;
	while (*s == ' ' || *s == '\t')
		s++;
	yytext[yyleng - 1] = 0;
	fprintf(stderr, "is \"%s\" true? ", s);
	yytext[yyleng - 1] = '\n';
	gets(buf);
	switch (buf[0]) {
	case 'y':
	case 'Y':
		return DEFINED;
	case 'n':
	case 'N':
		return UNDEFINED;
	default:
		return UNKNOWN;
	}
}

error(s)
char *s;
{
	fprintf(stderr, "%s: %s\n", cmd, s);
	exit(1);
}

yywrap()
{
	return 1;
}

cleanup(sig)
{
	unlink(tmpfil);
	exit(sig);
}

copy(file1, file2)
char *file1, *file2;
{
	static char buf[1024];
	register int n, f1, f2;

	if ((f1 = open(file1, 0)) < 0) {
		fprintf(stderr, "can't open temporary file");
		return;
	}
	if ((f2 = creat(file2, 0666)) < 0) {
		fprintf(stderr, "%s: cannot re-create %s\n", cmd, file2);
		return;
	}
	while ((n = read(f1, buf, 1024)) > 0)
		write(f2, buf, n);
	close(f1);
	close(f2);
}
