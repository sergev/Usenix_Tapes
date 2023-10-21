

/***************************************************************
 *            basic ray tracing package tracer2.0              *
 *            version 0.0 done 6/1/86-6/10/86                  *
 *            version 1.0 released 6/29/86                     *
 *            version 2.0 released 7/5/86                      *
 *            by friedrich knauss (one tired programmer)       *
 ***************************************************************/
#include <stdio.h>
#include <math.h>
#include "rtd.h"
#include "macros.h"


FILE * fp;
double  suzie[300][300],
        sam = 1.0;
int     xsue,
        ysue;
struct ball *bl[150];
int     level,
        nob;
struct sphere   ls;

main (argc, argv)
int     argc;
char  **argv;
{
    FILE * df, *texfile;
    static double   xco,
                    yco;
    struct ray  rr;
    struct vector   vp;
    double  x,
            y,
            z;
    int     i,
            in = 0,
            out = 0,
            tex = 0;
    int     c;

/* command interp */

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-')
	    booboo ("Options strt with a '-' bozo");
	c = argv[i][1];

	switch (c) {
	    case ('i'): /* input file */
		if (in)
		    booboo ("Sorry, but you may only have one input file");
		in = 1;
		if ((i + 1) >= argc || argv[i + 1][0] == '-')/* no arg */
		    df = stdin;
		else
		    if ((df = fopen (argv[++i], "r")) == NULL)
			booboo ("input file not found");
		break;
	    case ('o'): /* output file */
		if (out)
		    booboo ("Sorry, but you may have only one output file");
		out = 1;
		if ((i + 1) >= argc || argv[i + 1][0] == '-')/* no arg */
		    fp = stdout;
		else
		    fp = fopen (argv[++i], "w");
		break;
	    case ('s'): /* susie file */
		if (tex)
		    booboo ("Sorry, but you may have only one image file");
		if ((i + 1) >= argc || argv[i + 1][0] == '-')/* no arg */
		    booboo ("-s requires an argument");
		tex = 1;
		if ((texfile = fopen (argv[++i], "r")) == NULL)
		    booboo ("image file not found");
		break;
		booboo ("this line shouldn't do anything");
	    case ('S'): /* amount of susie */
		if (argv[i][2] < '0' || argv[i][2] > 9)
		    booboo ("-S needs a numerical argument");
		sam = atof (&(argv[i][2]));
		break;
	    default: 
		booboo ("Unrecognized option. Better try again");
	}
    }


    if (!in)
	if ((df = fopen ("bdata.i", "r")) == NULL)
	    booboo ("bdata.i not found");
    if (!out)
	fp = fopen ("data.dis", "w");
    if (!tex)
	if ((texfile = fopen ("pat.def", "r")) == NULL)
	    booboo ("pat.def not found");


/* if you can't figure *this* out, you should go home */
    nob = g_bal (df);
    g_bod (texfile);



    MV (95.0, 140.0, -200.0, vp);
    MV (0.0, 900.0, 0.0, ls.cent);
    ls.rad = 40;
    fprintf (fp, "%d %d\n", (int) ((XMAX - XMIN) * SCALE), (int) ((YMAX - YMIN) * SCALE));

    for (yco = YMAX * SCALE; yco > YMIN * SCALE; yco--)
	for (xco = XMIN * SCALE; xco < XMAX * SCALE; xco++) {
	    MV (xco / SCALE, yco / SCALE, 0.0, rr.org);
	    SV (rr.dir, rr.org, vp);
	    fprintf (fp, "%c", shade (&rr));
	}
}

booboo (str)
char   *str; {
    printf ("%s\n", str);
    exit (-1);
}
