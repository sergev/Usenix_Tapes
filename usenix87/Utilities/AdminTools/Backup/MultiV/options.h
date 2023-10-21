/*
 *	Option processing.
 *
 *	Conforms to intro(1) in System V.
 *
 *	Typical layout:
 *
 *		OPTIONS ("-b -t c -f file -w N file ...")
 *			FLAG	('b', flag)
 *			CHAR	('t', tab_ch)
 *			STRING	('f', filename)
 *				f = fopen(filename, "r");
 *			NUMBER	('w', width)
 *		ENDOPTS
 */
char	*O_name, *O_usage;

void
usage()
{
	/*
	 * Poor man's version of:
	 *
	 *	fprintf(stderr, "Usage: %s %s\n", O_name, O_usage);
	 */
	write(2, "Usage: ", 7);
	write(2, O_name, strlen(O_name));
	write(2, " ", 1);
	write(2, O_usage, strlen(O_usage));
	write(2, "\n", 1);
	exit(1);
}

/*
 *	An argument "-" is interpreted as a program argument and stops
 *	option processing.
 *
 *	An argument "--" stops option processing and is discarded.
 *
 *	An option which takes an argument uses either the rest of the
 *	the current argument, or, if at the end of an argument, the
 *	next argument.
 */
#define OPTIONS(usage)							\
	O_usage = usage;						\
	O_name = argv[0];						\
	while (*++argv && **argv == '-' && argv[0][1]) {		\
		register int O_cont = 1;				\
									\
		argc--;							\
		if (argv[0][1] == '-' && argv[0][2] == '\0') {		\
			argv++;						\
			break;						\
		}							\
		while (O_cont)						\
			switch (*++*argv) {				\
			case '\0':					\
				O_cont = 0;

# define FLAG(c,flag)	break;						\
			case c:						\
				flag = 1;

# define NUMBER(c,num)	break;						\
			case c:						\
				if (*++*argv == '\0') {			\
					if (--argc == 0)		\
						usage();		\
					argv++;				\
				}					\
				num = atol(*argv);			\
				O_cont = 0;

# define STRING(c,str)	break;						\
			case c:						\
				if (*++*argv == '\0') {			\
					if (--argc == 0)		\
						usage();		\
					argv++;				\
				}					\
				str = *argv;				\
				O_cont = 0;

# define CHAR(c,ch)	break;						\
			case c:						\
				if (*++*argv == '\0') {			\
					if (--argc == 0)		\
						usage();		\
					argv++;				\
				}					\
				ch = **argv;				\
				O_cont = 0;

# define ENDOPTS	break;						\
			default:					\
				usage();				\
			}						\
	}								\
	*--argv = O_name;
