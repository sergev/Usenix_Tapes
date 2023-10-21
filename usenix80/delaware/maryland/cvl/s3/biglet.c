#
#define EIGHTLPI        06      /* causes PRINTRONIX to print at 8 lines per inch */

/*
 * Prints fonts sideways on standard output: one character per pixel
 * Call by: biglet <font>
 * Last modified: August 1, 1979
 */

struct buf {
	int fildes;
	int unused;
	char *nextp;
	char buff[512];
	};

extern int fout;

int fd, cd, i, j, high, line_num, line_flag 0, wid;
char c 1, tty;
struct buf outfile[1];

main(argc, argv)
int argc;
char *argv[];
{

fout = 2;       /* direct printf to error output */

outfile -> fildes = 1;  /* direct outfile to standard output */
outfile -> nextp = outfile -> buff;

if (argc < 2) {
	printf("Useage: %s fontname[.fnt]\n", argv[0]);
	exit(1);
	}

if ((fd = openfont(argv[1])) < 0) {
	printf("Can't open font file: '%s'\n", argv[1]);
	exit(1);
	}

high = height(fd);

if (((tty = ttyn(0)) == ttyn(2)) && (tty != 'x')) /* standard input = tty, so print prompt */
	line_flag++;

for (line_num = 1; c; line_num++) {
	if (line_flag) {        /* print prompt */
		fflush(outfile);
		printf("%2d> ", line_num);
		}

	while ((c = getchar()) && (c != '\n')) {

		if (c == '%') { /* escape character */
			c = getchar();  /* read another */
			c = (c == '%')? c: c & 037;
			}

		if ((cd = readchar(fd, c)) < 0) {
			printf((c < 040)? "Character: 0%o not defined in font\n": "Character: '%c' not defined in font\n", c);
			}
		else {
			wid = max(width(cd), r_width(cd));
			for(i = 0; i < wid; i++) {
				putc(EIGHTLPI, outfile);
				for(j = high-1; j >= 0; j--)
					putc(accesschar(cd, i, j)? '*': ' ', outfile);
				putc('\n', outfile);
				}
			freechar(cd);
			}
		}
	}
fflush(outfile);
}
