
/* * igraph.c * copyright 1986 by gottfried robert walter * permission is hereby granted to use/copy/mangle this program (but not sell) * as long as this message is included */

#include <sys/file.h>
#include <stdio.h>
#include <ctype.h>

main(argc, argv)
	int		argc;
	char	*argv[];
{
	int		i, size;
	int		vmult = 1, hmult = 1;
	int		rshift = 0, dshift = 0;
	int		ncopies = 1;
	int		pen = 1;
	int		prerast = 0;
	int		header = 1;
	short	path[16000];
	char	word[100], line[10000], *pl, *getnextword();

	for(i = 1; i < argc; i++) {
		switch (argv[i][1]) {
			case 'v':
				vmult = atoi(argv[i]+2);
				if (vmult < 1)
					usage();
				break;
			case 'h':
				hmult = atoi(argv[i]+2);
				if (hmult < 1)
					usage();
				break;
			case 'r':
				rshift = atoi(argv[i]+2);
				break;
			case 'd':
				dshift = atoi(argv[i]+2);
				break;
			case '#':
				ncopies = atoi(argv[i]+2);
				if ((ncopies < 1) || (ncopies > 10))
					usage();
				break;
			case 'p':
				pen = atoi(argv[i]+2);
				if ((pen < 1) || (pen > 20))
					usage();
				break;
			case 'P':
				prerast = atoi(argv[i]+2);
				break;
			case 'H':
				header = atoi(argv[i]+2);
				break;
		}
	}

	(void)printf("@document(");
	(void)printf("language impress,");
	if (header != 0)
		(void)printf("jobheader on,");
	if (prerast != 0)
		(void)printf("prerasterization on,");
	(void)printf("copies %d)", ncopies);
	(void)putchar(232);
	(void)putchar((char)pen);
	size = 0;
	while ((fgets(line, 10000, stdin)) != NULL) {
		pl = line;
		for (; ((*pl != '\0') && (*pl != '\n'));) {
			pl = getnextword(pl, word);
			path[size++] = (short)(atoi(word)*hmult+rshift);
			pl = getnextword(pl, word);
			path[size++] = (short)(atoi(word)*vmult+dshift);
			if (path[size-2] >= 2560)	/* you may want to change the */
				path[size-2] = 2560;	/* bounds checking done here  */
			if (path[size-2] <= 0)
				path[size-2] = 1;
			if (path[size-1] >= 3328)
				path[size-1] = 3328;
			if (path[size-1] <= 0)
				path[size-1] = 1;
			if (size >= 16000) {
				(void)fprintf(stderr, "too many vertices in path\n");
				(void)exit(2);
			}
		}
		putpath(size, path);
		size = 0;
	}
	(void)putchar(219);
	(void)putchar(255);

	(void)exit(0);
}

/* usage - tell user the correct usage */

usage()
{
	(void)fprintf(stderr, "Correct usage :\n");
	(void)fprintf(stderr, "  -v = vertical multiplication factor (>0)\n");
	(void)fprintf(stderr, "  -h = horizontal multiplication factor (>0)\n");
	(void)fprintf(stderr, "  -r = displacement to right\n");
	(void)fprintf(stderr, "  -d = displacement down\n");
	(void)fprintf(stderr, "  -# = number of copies (1-10)\n");
	(void)fprintf(stderr, "  -p = pen diameter (1-20)\n");
	(void)fprintf(stderr, "  -P = page rasterisation (0-off/other-on)\n");
	(void)fprintf(stderr, "  -H = job header (0-off/other-on)\n");
	(void)exit(1);
}

/* putpath - send current path to stdout in the correct format */

putpath(size, path)
	int		size;
	short	*path;
{
	int		i;
	(void)putchar(230);
	(void)putchar((char)((size/2)/256));
	(void)putchar((char)((size/2)%256));
	for(i = 0; i < ((size/2)*2); i++) {
		(void)putchar((char)(path[i]/256));
		(void)putchar((char)(path[i]%256));
	}
	(void)putchar(234);
	(void)putchar(15);
}

/* getnextword gets the next word from old and returns it (in lowercase)
 * in new
 *
 * input  : old = pointer to line with words to be read
 *          new = pointer to area to put next word
 * output : pointer to right after where the word ended in old
 */

	char	*
getnextword(old, new)
	register char	*old;
	register char	*new;
{
	char			*strspn();

	old = strspn(old, " ,/:");
	if (*old != '\0') {
		for(;((*old != ' ') && (*old != ',') && (*old != ':')
			&& (*old != '\0') && (*old != '/')); old++) {
			*new = *old;
			if (isascii(*new))
				if (isupper(*new))
					*new += 'a' - 'A';
			new++;
		}
	}
	*new = '\0';
	return (old);
}

/* strspn - span any occurances of the "spn" characters
 *
 * input  : str = pointer to the string
 *          spn = pointer to the string of characters to be spanned
 * output : pointer to right after the last spanned character
 */

	char *
strspn(str, spn)
	register char	*str, *spn;
{
	register int	i;

	for(;;) {
		i = 0;
		while ((spn[i] != *str) && (spn[i] != '\0')) 
			i++;
		if ((spn[i] == '\0') || (*str == '\0'))
			return (str);
		str++;
	 }
}
------------wow, yet another cut line-----------------------------------

oh, ya, some of you wanted to see other stuff that did things on imagens.
so here is a simple fractal dragon generating program that will produce
a bit map that the imagen can print. if you find more good seeds, i
would appreciate being mailed them.

oh ya, it is a real cpu hog, and when you are generating large fractals,
it will take some time, so you may want to run it overnight.

i also have a version that puts out dragons that a printronix can print
out.

magfact should be 0 for the best looking dragon (but it takes a long time
to calculate).

also, the higher ntests is, the better looking the dragon is (a good value
is 150 - i used 50 here so you could do a quick test to see if it worked on your
imagen).

-------------yet a, yet a nuther cut line------------------------------------
/*
 * this program will generate fractal dragons
 * original work by c. goudeseune
 * current version by g. r. walter as of february 28, 1986
 * imagen output version as of july 11, 1986
 */

#include <stdio.h>

#define RSTART -0.3      /* start of real      range */
#define REND    1.35     /* end   of real      range */
#define ISTART -0.9      /* start of imaginary range */
#define IEND    0.9      /* end   of imaginary range */

main()
{
	register float	i, j, u, v;
    float			rseed, iseed, width, a, b;
	register int	k, l, m;
    int				columns, rows, ntests, magfact;
	int				xpos, counter;
	char			hsize, vsize;
	char			line[32][300];

	/*
	 * input seed (real and imaginary parts),
	 * output size (columns * rows), and
	 * number of tests per location
	 */

	rseed	= 1.64542;
	iseed	= 0.9635;
	magfact = 2;
	hsize   = 18*(4>>magfact);
	columns = 32*hsize;     /*width  of square*/
	vsize   = 18*(4>>magfact);
	rows	= 32*vsize;     /*height of square*/
	ntests	= 50;

	(void)printf("@document(language impress, jobheader off)");
	(void)putchar(138);		/* position fractal vertically on page */
	(void)putchar(2);		/* hi byte */
	(void)putchar(0);		/* lo byte */
	(void)putchar(236);		/* set bitmap magnification */
	(void)putchar(magfact);
	(void)putchar(235);		/* set bitmap parameters */
	(void)putchar(15);
	(void)putchar(hsize);
	(void)putchar(vsize);

	width   = (IEND - ISTART) / columns;

	a	= RSTART;

	for (counter = 0; counter < vsize; counter++) {
		for (l = 0; l < 32; l++) {
			b    = ISTART;
			xpos = 0;
			for (k=0; k < (int)(hsize)*4; line[l][k++]=0);

			while (b <= IEND)
			{
				k = 0;
				i = a;
				j = b;

				while ((k++ < ntests) && ((i * i + j * j) < 4))
				{
					u = j * j + i * (1 - i);
					v = j * (i + i - 1);
					i = rseed * u + iseed * v;
					j = iseed * u - rseed * v;
				}

				if (k >= ntests)
					line[l][xpos>>3] |= 1<<(7-(xpos&7));
				++xpos;
				b += width;
			}
			a += width * columns / rows;
		}
		for (k = 0; k < hsize; k++)
			for (l = 0; l < 32; l++)
				for (m = 0; m < 4; putchar(line[l][k*4+m++]));
	}
	(void)exit(0);
}
