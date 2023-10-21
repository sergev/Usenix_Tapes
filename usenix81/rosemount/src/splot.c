# include <stdio.h>

# define MAX_WID 151
# define MAX_LEN 101
# define DEF_WID 61
# define DEF_LEN 21
# define MAXENT 500

int wid = DEF_WID;
int len = DEF_LEN;
char disp[MAX_WID][MAX_LEN];
float xmin = 0.0, xmax = 0.0, xspan = 0.0;
float ymin = 0.0, ymax = 0.0, yspan = 0.0;



main(argc, argv)	/* splot */
int argc;
char **argv;
{
	FILE *fp;
	int fcnt = 0;

	while (--argc > 0) {
		argv++;
		if (**argv == '-')
			switch (*++*argv) {

			case 'w':
				wid = atoi(++*argv)+1;
				if (wid < 6 || wid > MAX_WID)
					wid = DEF_WID;
				break;

			case 'l':
				len = atoi(++*argv)+1;
				if (len < 6 || wid > MAX_LEN)
					len = DEF_LEN;
				break;

			default:
				fprintf(stderr, "-%c ?\n", **argv);
				exit(1);

			}
		else {
			if ((fp = fopen(*argv, "r")) == NULL) {
				fprintf(stderr, "%s: can't open\n", *argv);
				exit(1);
			}
			else {
				process(fp);
				fclose(fp);
			}
			fcnt++;
		}
	}
	if (! fcnt)
		process(stdin);
	exit(0);
}



process(fp)
FILE *fp;
{
	int xind = 0, yind = 0, xloc = 0, yloc = 0, entry = 0, i = 0;
	float xnum[MAXENT], ynum[MAXENT];

	if ((entry = getnum(fp, xnum, ynum, MAXENT)) <= 0)
		exit(1);
	for (xind = 0; xind < wid; xind++)
		for (yind = 0; yind < len; yind++)
			disp[xind][yind] = ' ';
	for (i = 0; i < entry; i++) {
		xloc = (((xnum[i]-xmin)/xspan)*(wid-1)+0.5);
		yloc = (((ynum[i]-ymin)/yspan)*(len-1)+0.5);
		disp[xloc][yloc] = '*';
	}
	for (yloc = len-1; yloc >= 0; yloc--) {
		if (yloc % 5 == 0)
			printf("%6.2f  +", (yloc/(float)(len-1))*yspan+ymin);
		else
			printf("        |");
		for (i = wid-1; i > 0; i--)
			if (disp[i][yloc] != ' ')
				break;
		for (xloc = 0; xloc <= i; xloc++)
			putchar(disp[xloc][yloc]);
		putchar('\n');
	}
	printf("        |");
	for (i = 0; i < wid; i++)
		putchar((i % 15 == 0 ? '+' : '-'));
	putchar('\n');
	printf("  ");
	for (i = 0; i < wid; i++)
		if (i % 15 == 0)
			printf("    %6.2f     ",
			(i/(float)(wid-1))*xspan+xmin);
	putchar('\n');
	return;
}



getnum(fp, x, y, max)
FILE *fp;
float x[], y[];
int max;
{
	int entry, i, c;

	for (entry = 0; entry < max; entry++) {
		if ((c = getc(fp)) == EOF)
			break;
		ungetc(c, fp);
		if (fscanf(fp, " %f",  &x[entry]) != 1)
			break;
		if ((c = getc(fp)) == EOF)
			break;
		ungetc(c, fp);
		if (fscanf(fp, " %f\n", &y[entry]) != 1)
			break;
	}

	if (entry >= MAXENT) {
		fprintf(stderr, "too much data\n");
		return -1;
	}
	if (entry == 0) {
		fprintf(stderr, "null file\n");
		return 0;
	}
	xmin = xmax = x[0];
	ymin = ymax = y[0];
	for (i = 0; i < entry; i++) {
		if (x[i] < xmin)
			xmin = x[i];
		if (x[i] > xmax)
			xmax = x[i];
		if (y[i] < ymin)
			ymin = y[i];
		if (y[i] > ymax)
			ymax = y[i];
	}
	xspan = xmax-xmin;
	yspan = ymax-ymin;

	return entry;
}
