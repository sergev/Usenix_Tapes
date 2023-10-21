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
	int		off_x, off_y;
	u_int		data[NROW][NCOL];
	FILE		*fp;

	/* check to make sure they specified the correct # of args */
	if (argc != 3 && argc != 4) {
		fprintf(stderr, "usage: moveicon offset_x offset_y { icon }\n");
		exit(1);
	}
	/* get the offset */
	off_x = atoi(argv[1]);
	off_y = atoi(argv[2]);
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

	/* move the icon vertically */
	if (off_y < 0) {
		for (i = 0; i < NROW; i++) {
			if (i - off_y < NROW) {
				for (j = 0; j < NCOL; j++) {
					data[i][j] = data[i - off_y][j];
				} /* end for */
			} else {
				for (j = 0; j < NCOL; j++) {
					data[i][j] = (u_int) 0;
				} /* end for */
			} /* end else */
		} /* end for */
	} else {
		for (i = NROW - 1; i >= 0; i--) {
			if (i - off_y >= 0) {
				for (j = 0; j < NCOL; j++) {
					data[i][j] = data[i - off_y][j];
				} /* end for */
			} else {
				for (j = 0; j < NCOL; j++) {
					data[i][j] = (u_int) 0;
				} /* end for */
			} /* end else */
		} /* end for */
	} /* end else */

	/* move the icon horizontally */
	for (i = 0; i < NROW; i++) {
		u_int	ofbits = 0;		/* overflow bits */
		u_int	prev = 0;		/* previous overflow bits */

		if (off_x > 0) { 
			/* do we need to shift words to the right? */
			if (off_x >= WORDSIZ) {
				/* yes -- shift low to high */
				j = NCOL - 1;
				k = j - off_x / WORDSIZ;
				/* shift words to the right */
				while (j >= 0) {
					data[i][j] = data[i][k];
					data[i][k] = (u_int) 0;
					j--, k--;
				} /* end while */
				/* subtract the word shift from the offset */
				off_x = off_x % WORDSIZ;
			} /* end if */
			/* shift the bits in the words */
			for (j = 0; j < NCOL; j++) {
				/* store the overflow bits */
				ofbits = data[i][j] << (WORDSIZ - off_x);
				/* set the new value */
				data[i][j] = prev | (data[i][j] >> off_x);
				/* make the current overflow bits previous */
				prev = ofbits;
			} /* end for */
		} else {
			int	tmp_x = abs(off_x);

			/* do we need to shift words to the left? */
			if (tmp_x >= 16) {
				/* yes -- shift high to low */
				j = tmp_x / WORDSIZ;
				k = 0;
				while (j < NCOL) {
					data[i][k] = data[i][j];
					data[i][j] = (u_int) 0;
					j++, k++;
				} /* end while */
				/* subtract the word shift from the offset */
				tmp_x = tmp_x % WORDSIZ;
			} /* end if */
			/* shift the bits in the words */
			for (j = NCOL - 1; j >= 0; j--) {
				/* store the overflow bits */
				ofbits = data[i][j] >> (WORDSIZ - tmp_x);
				/* set the new value */
				data[i][j] = prev | (data[i][j] << tmp_x);
				/* make the current overflow bits previous */
				prev = ofbits;
			} /* end for */
		} /* end else */
	} /* end for */

	/* output the icon file */
	printf("/* Format_version=1, Width=%d, Height=%d, ", WIDTH, HEIGHT);
	printf("Depth=1, Valid_bits_per_item=%d */", WORDSIZ);
	for (i = 0; i < NROW; i++) {
		/* print two rows on a line */
		if (i % 2 == 0) printf("\n\t");
		/* print the row */
		for (j = 0; j < NCOL; j++) {
			printf("0x%04x,", data[i][j] & 0xffff);
		} /* end for */
	} /* end for */
	printf("\n");
} /* end main() */
