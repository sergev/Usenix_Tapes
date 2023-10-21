#include <stdio.h>
#include <sys/types.h>

main(argc, argv)
int	argc;
char	**argv;
{
	register int	i, j, k;
	register char	c;
	u_int		data[256];
	FILE		*fp;

	if (argc < 2) {
		fp = stdin;
	} else {
		if ((fp = fopen(argv[1], "r")) == NULL) exit(1);
	}
	while ((c = getc(fp)) != '\t');
	for (i = 0; i < 256; i++) {
		if (fscanf(fp, " 0x%x,", &data[i]) != 1) {
			exit(1);
		}
	}
	for (i = 0; i < 256; i+= 4) {
		for (j = 0; j < 4; j++) {
			for (k = 15; k >= 0; k--) {
				if (data[i+j] & (1 << k)) {
					putchar('x');
				} else {
					putchar(' ');
				}
			}
		}
		putchar('\n');
	}
	fclose(fp);
} /* end main() */
