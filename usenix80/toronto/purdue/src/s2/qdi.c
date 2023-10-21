#
#define EOF	-1

/*
 *	q d i  -  Display file
 *
 *	qdi <file> [<low line> [<high line>]] [<options> ...]
 */

main(argc, argv)
int argc;
char **argv;
{
	extern fin, fout;
	register int line;
	register char c;
	register char *format;
	int low_line, high_line, nlf;

	fout = dup(1);
	format = "%7l=";
	low_line = high_line = line = 0;

	/* Decode params */

	do {	/* once only thru this "loop" */
		if (--argc < 1) break;
		argv++;
		if (**argv != '-' || argv[0][1]) {
			fin = open(*argv, 0);
			if (fin < 0) {
				write(2, "can't open ", 11);
				while (**argv)
					write(2, (*argv)++, 1);
				write(2, "\n", 1);
				exit(1);
			}
		}

		if (--argc < 1) break;
		argv++;
		if (low_line = atoi(*argv)) {
			argc--;
			argv++;
			if (high_line = atoi(*argv)) {
				argc--;
				argv++;
			}
		}

		if (argc < 1) break;
		if ((argv[0][0] == 's') && (argv[0][1] == 'u') &&
			(argv[0][2] == 'p') && (argv[0][3] == '\0'))
			format = 0;

	/* Process options here */

	} while (0);

	if (low_line > high_line)
		high_line = low_line;
	if (high_line == 0)
		high_line = 077777;

	/* Skip to low line */

	line = 1;
	while (line < low_line && (c = getchar()) != EOF)
		if ((c =& 0177) == '\n')
			line++;

	/* Print requested part */

	nlf = 1;
	while (line <= high_line && (c = getchar()) != EOF) {
		if (nlf) {
			if (format)
				printf(format, line);
			nlf = 0;
		}
		putchar(c =& 0177);
		if (c == '\n') {
			nlf++;
			line++;
		}
	}
	flush();
}

int	fin	0;
int	_ieof	0;
int	_ipos	0;
int	_ibytes	0;
char	_ibuf[512];

getchar() {
	if (_ieof)
		return(EOF);
	if (_ipos >= _ibytes) {
		_ipos = 0;
		if ((_ibytes = read(fin, _ibuf, 512)) <= 0) {
			_ieof++;
			return(EOF);
		}
	}
	return(_ibuf[_ipos++]);
}

int	fout	1;
int	_opos	0;
char	_obuf[512];

putchar(c)
char c;
{
	_obuf[_opos++] = c;
	if (_opos >= 512) {
		write(fout, _obuf, _opos);
		_opos = 0;
	}
	return(c);
}

flush() {
	if (_opos)
		write(fout, _obuf, _opos);
}
