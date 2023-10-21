/*		Copyright (c) 1981 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

#include <stdio.h>		/* stander i/o library definitions */
#include <math.h>		/* math function declarations */
#include <sgtty.h>		/* used for getting terminal status */
#include <ctype.h>		/* character type determiners */
#define	W	12		/* format field widths */
#define	D	3		/* format field decimal lengths */
#define	MAXPAIRS	1000	/* at most this many pairs */
#define	MAXWIDTH	100	/* maximum width of plot */
#define	MAXHEIGHT	100	/* maximum height of plot */
#define	MINWIDTH	20	/* minimum reasonable width of plot */
#define	MINHEIGHT	5	/* minimum reasonable height of plot */
#define	SHORTSTRING	32	/* used for char arrays */
char	plotchar = '*';		/* this char plotted if >= 10 pairs/point */
float	xy_data[2][MAXPAIRS];	/* data stored for plot in here */
char	plot[MAXWIDTH][MAXHEIGHT];/* plot built in here */
int	height = 20, width = 50;/* default height and width of plot */
int	perline = 2;		/* number of points per line */
int	count = 0;		/* number of points counted in */
double	x, y, d;		/* x, y, and difference */
double	sum_x, sum_y, sum_d;	/* sums of scores */
double	ss_x, ss_y, ss_d, sum_xy;/* sums of squares and cross product */
float	min_x, max_x, min_y;	/* minimums */
float	max_y, min_d, max_d;	/* maximums */
char	name_x[SHORTSTRING] = "Column 1", name_y[SHORTSTRING] = "Column 2";
char	program[SHORTSTRING];	/* name of program as called in shell */
char	line[BUFSIZ];		/* used for interal formatting */
int	storedata = 1;		/* assume data will be stored */
int	wantstats = 0;		/* are statistics wanted? */
int	wantplot = 0;		/* is a plot wanted? */

main (argc, argv) char **argv;
	{
	initial (argc, argv);
	readdata ();
	if (wantstats) printstats ();
	if (wantplot) printplot ();
	}

initial (argc, argv) char **argv;
    {
    int	argnum;
    char	*optr;
    struct	sgttyb	ttystatus;
    strcpy (program, argv[0]);
    checkstdin (argv[0]);
    if (gtty (fileno (stdout), &ttystatus) == 0 && ttystatus.sg_ospeed <= B1200)
		{
		width = 30; height = 10; /* set small size or slow terminal */
		}
    if (!strcmp (program, "biplot"))
	{
	wantstats = 0;
	wantplot = 1;
	}
    else if (argc == 1)
	{
	wantstats = 1;
	wantplot = 0;
	}
    for (argnum = 1; argnum < argc; argnum++)
	{
	optr = argv[argnum];
	while (*optr) switch (*optr++)
	    {
	    case 'c': wantplot = 1; plotchar = *optr++; break;
	    case 's': wantstats = 1; break;
	    case 'b':
	    case 'p': wantplot = 1; break;
	    case 'w':
		    wantplot = 1;
		    if ((width = atoi (optr)) < MINWIDTH) width = MINWIDTH;
		    else if (width > MAXWIDTH)
			eprintf ("%s: Requested plot width exceeds %d\n",
				program, MAXWIDTH);
		    while (isdigit (*optr)) optr++;
		    break;
	    case 'h':
		    wantplot = 1;
		    if ((height = atoi (optr)) < MINHEIGHT) height = MINHEIGHT;
		    else if (height > MAXHEIGHT)
			eprintf ("%s: Requested plot height exceeds %d\n",
				program, MAXHEIGHT);
		    while (isdigit (*optr)) optr++;
		    break;
	    case 'x':
		    wantstats = 1;
		    strcpy (name_x, optr);
		    *optr = NULL;
		    break;
	    case 'y':
		    wantstats = 1;
		    strcpy (name_y, optr);
		    *optr = NULL;
		    break;
	    default:
		eprintf ("%s: Unknown option '%c'\n",program,*(optr-1));
	    }
	}
    if (!wantplot)
	{
	storedata = 0;
	wantstats = 1;
	}
    }

readdata ()
	{
	double	atof ();
	char	line[BUFSIZ];
	char	array[3][SHORTSTRING];
	int	fieldcount;
    firstline:
	if (fgets (line, BUFSIZ, stdin) == NULL)
		eprintf ("%s: No data\n", program);
	switch (sstrings (line, array, 3, SHORTSTRING))
		{
		case 0: goto firstline;
		case 1: perline = 1;
			min_x = 1.0;
			min_y = atof (array[0]);
			break;
		case 2: perline = 2;
			max_x = atof (array[0]);
			max_y = atof (array[1]);
			break;
		}
	max_x = min_x;
	max_y = min_y;
	min_d = max_d = min_x - min_y;
	do  {
	    fieldcount = sstrings (line, array, 3, SHORTSTRING);
	    if (fieldcount == 0) continue;
	    if (fieldcount != perline)
		eprintf ("%s: Must have 1 or 2 numbers per line, see line %d\n",
		    program, count+1);
	    if (!number (array[0]) || ((perline == 2) && !number (array[1])))
		    eprintf ("%s: Non-numerical input at line %d\n",
			    program, count+1);
	    if (perline == 2)
		{
		x	= atof (array[0]);
		y	= atof (array[1]);
		}
	    else
		{
		x	= (float) (count+1);
		y	= atof (array[0]);
		}
	    d	= x - y;
	    sum_d	+= d;
	    ss_d	+= d*d;
	    sum_x	+= x;
	    ss_x	+= x*x;
	    sum_xy	+= x*y;
	    sum_y	+= y;
	    ss_y	+= y*y;
	    if (x > max_x) max_x = x;
	    else if (x < min_x) min_x = x;
	    if (y > max_y) max_y = y;
	    else if (y < min_y) min_y = y;
	    if (d > max_d) max_d = d;
	    else if (d < min_d) min_d = d;
	    if (storedata)
		if (count < MAXPAIRS)
		    {
		    xy_data[0][count] = x;
		    xy_data[1][count] = y;
		    }
		else
		    {
		    fprintf (stderr, "%s: maximumn number of points for a plot is %d\n",
			    program, MAXPAIRS);
		    storedata = 0;
		    wantstats = 1;
		    }
	    count++;
    } while (fgets (line, BUFSIZ, stdin));
	}

printstats ()
	{
	double	mean_x, mean_y, mean_d;		/* means stored here */
	double	sd_x, sd_y, sd_d, standev ();	/* standard deviations */
	double	t_x, t_y, t_d;			/* t test values */
	double	p_x, p_y, p_d, pof ();		/* probability levels */
	char	tmpstr[SHORTSTRING];		/* for internal formatting */
	double	a, b;				/* intercept and slope */
	double	r, t_r, p_r;			/* correlation, t val & prob */
	float	fcount = (float) count;		/* floating count */
	if (count <= 1)
		eprintf ("%s: Need at least two data pairs\n", program);
	printf ("%*s%*.*s%*.*s%*s\n", W, "",
		W, W, name_x, W, W, name_y, W, "Difference");
	prettyprint ("Minimums", min_x, min_y, min_d);
	prettyprint ("Maximums", max_x, max_y, max_d);
	prettyprint ("Sums", sum_x, sum_y, sum_d);
	prettyprint ("SumSquares", ss_x, ss_y, ss_d);
	mean_x = sum_x / fcount;
	mean_y = sum_y / fcount;
	mean_d = sum_d / fcount;
	prettyprint ("Means", mean_x, mean_y, mean_d);
	sd_x = standev (sum_x, ss_x, count);
	sd_y = standev (sum_y, ss_y, count);
	sd_d = standev (sum_d, ss_d, count);
	prettyprint ("SDs", sd_x, sd_y, sd_d);
	if (sd_x)
		{
		t_x = mean_x*sqrt(fcount)/sd_x;
		p_x = pof (t_x*t_x, 1, count-1);
		}
	else	p_x = t_x = 0.0;
	if (sd_y)
		{
		t_y = mean_y*sqrt(fcount)/sd_y;
		p_y = pof (t_y*t_y, 1, count-1);
		}
	else	p_y = t_y = 0.0;
	if (sd_d)
		{
		t_d = mean_d*sqrt(fcount)/sd_d;
		p_d = pof (t_d*t_d, 1, count-1);
		}
	else	p_d = t_d = 0.0;
	sprintf (tmpstr, "t(%d)", count-1);
	prettyprint (tmpstr, t_x, t_y, t_d);
	prettyprint ("p", p_x, p_y, p_d);
	if (sd_x)
		{
		b = (sum_xy - sum_x*sum_y/fcount) / (ss_x - sum_x*mean_x);
		a = mean_y - b * mean_x;
		if (sd_y) r = b * sd_x / sd_y;
		else r = 0.0;
		}
	else a = b = r = 0.0;
	if (count > 2 && r != 1.0)
		{
		t_r = r / sqrt ((1.0 - r*r) / (fcount - 2.0));
		p_r = pof (t_r*t_r, 1, count-2);
		}
	else	t_r = p_r = 0.0;
	printf ("\n");
	sprintf (tmpstr, "t(%d)", count-2);
	printf ("%*s%*s%*s%*s\n", W, "Correlation",
		 W, "r-squared", W, tmpstr, W, "p");
	printf ("%*.*f%*.*f%*.*f%*.*f\n", W, D, r,
		W, D, r*r, W, D, t_r, W, D, p_r);
	printf ("%*s%*s\n", W, "Intercept",
		W, "Slope");
	printf ("%*.*f%*.*f\n", W, D, a, W, D, b);
	}

double
standev (sum, ss, n) double sum, ss;
	{
	if (count <= 1) return (0.0);
	return (sqrt ((ss-sum*sum/count)/(count-1)));
	}

prettyprint (title, x, y, d) double x, y, d;
	{
	printf ("%-*s%*.*f%*.*f%*.*f\n", W, title, W, D, x, W, D, y, W, D, d);
	}

printplot ()
	{
	double	epsilon = .01;
	double	xdiv = (max_x - min_x + epsilon) / width;
	double	ydiv = (max_y - min_y + epsilon) / height;
	int	plot_x, plot_y;
	int	pmean_x = (sum_x/count - min_x) / xdiv;
	int	pmean_y = (sum_y/count - min_y) / ydiv;
	int	y, x;
	if (!storedata) return;
	for (y = 0; y < count; y++)
		{
		plot_x = (xy_data[0][y] - min_x) / xdiv;
		plot_y = (xy_data[1][y] - min_y) / ydiv;
		plot[plot_y][plot_x]++;
		}
	if (wantstats) putchar ('\n');
	printf ("%*.*f", W, D, max_y);
	putchar ('|');
	for (x = 0; x < width; x++) printf ("-");
	putchar ('|');
	putchar ('\n');
	for (y = height-1; y >= 0; y--)
		{
		if (pmean_y == y)
			printf ("%*.*fY", W, D, sum_y/count);
		else printf ("%*s|", W, "");
		for (x = 0; x < width; x++)
			if (plot[y][x])
				{
				if (plotchar != '*') putchar (plotchar);
				else if (plot[y][x] >= 10) putchar (plotchar);
				else printf ("%d", plot[y][x]);
				}
			else putchar (' ');
		putchar ('|');
		putchar ('\n');
		}
	printf ("%*.*f", W, D, min_y);
	putchar ('|');
	for (x = 0; x < width; x++)
		if (x == pmean_x) printf ("X");
	else	putchar ('-');
	putchar ('|');
	putchar ('\n');
	sprintf (line+pmean_x, "%-.*f", D, sum_x/count);
	sprintf (line, "%-.*f", D, min_x);
	sprintf (line+width-D, "%.*f\n", D, max_x);
	for (x = 0; line[x] != '\n'; x++)
		if (!line[x]) line[x] = ' ';
	printf ("%*s%s", W, "", line);
	}
