#include <stdio.h>
#include <sys/types.h>

#define WIDTH		64
#define HEIGHT		64
#define WORDSIZ		16
#define NCOL		(WIDTH / WORDSIZ)
#define NROW		(HEIGHT)

main(argc, argv)
int	argc;
char	**argv;
{
	register int	i, j, k;
	register char	c;
	int		width, height;
	u_int		data[NROW][NCOL];
	FILE		*fp;

	/* check to make sure they specified the correct # of args */
	if (argc != 3 && argc != 4) {
		fprintf(stderr, "usage: trimicon width height { icon }\n");
		exit(1);
	}
	/* get the size */
	width = abs(atoi(argv[1]));
	height = abs(atoi(argv[2]));
	/* did they specify a filename? */
	if (argc == 4 && argv[3] != (char *) NULL) {
		/* yes -- open it */
		if ((fp = fopen(argv[3], "r")) == NULL) {
			fprintf(stderr, "Can't open file %s for reading\n", 
				argv[3]);
			exit(1);
		}
	} else {
		/* no -- set <fp> to be stdin */
		fp = stdin;
	}
		
	/* skip the comments */
	while ((c = getc(fp)) != '\t');

	/* read in the icon */
	for (i = 0; i < NROW; i++) {
		for (j = 0; j < NCOL; j++) {
			if (fscanf(fp, " 0x%x,", &data[i][j]) != 1) {
				fprintf("Error reading file %s\n", argv[3]);
				exit(1);
			}
		} /* end for */
	} /* end for */

	/* close the file */
	fclose(fp);

	for (i = 0; i < NROW; i++) {
		if (i > height) {
			for (j = 0; j < NCOL; j++) {
				data[i][j] = (u_int) 0;
			}
		} else {
			for (j = 0; j < NCOL; j++) {
				if (j * WORDSIZ > width) {
					data[i][j] = (u_int) 0;
				} else if (j == width / WORDSIZ) {
					k = width - j * WORDSIZ;
					data[i][j] = (data[i][j] >> k) << k;
				} /* end else */
			} /* end for */
		} /* end else */
	} /* end for */

	/* output the icon file */
	printf("/* Format_version=1, Width=%d, Height=%d, ", width, height);
	printf("Depth=1, Valid_bits_per_item=%d */", WORDSIZ);
	for (i = 0; i < height; i++) {
		/* print two rows on a line */
		if (i % 2 == 0) printf("\n\t");
		/* print the row */
		for (j = 0; j * WORDSIZ < width; j++) {
			printf("0x%04x,", data[i][j] & 0xffff);
		} /* end for */
	} /* end for */
	printf("\n");
} /* end main() */
