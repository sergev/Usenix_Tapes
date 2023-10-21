

/* tracer version 2.1 */
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
	    case ('i'): 
		if (in)
		    booboo ("Sorry, but you may only have one input file");
		in = 1;
		if ((i + 1) >= argc || argv[i + 1][0] == '-')/* no arg */
		    df = stdin;
		else
		    if ((df = fopen (argv[++i], "r")) == NULL)
			booboo ("input file not found");
		break;
	    case ('o'): 
		if (out)
		    booboo ("Sorry, but you may have only one output file");
		out = 1;
		if ((i + 1) >= argc || argv[i + 1][0] == '-')/* no arg */
		    fp = stdout;
		else
		    fp = fopen (argv[++i], "w");
		break;
	    case ('s'): 
		if (tex)
		    booboo ("Sorry, but you may have only one image file");
		if ((i + 1) >= argc || argv[i + 1][0] == '-')/* no arg */
		    booboo ("-s requires an argument");
		tex = 1;
		if ((texfile = fopen (argv[++i], "r")) == NULL)
		    booboo ("image file not found");
		break;
		booboo ("this line shouldn't do anything");
	    case ('S'): 
		if (argv[i][2] < '0' || argv[i][2] > '9'){
printf("%c\n",argv[i][2]);
		    booboo ("-S needs a numerical argument");}
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



    nob = g_bal (df);
    g_bod (texfile);



    MV (95.0, 140.0, -200.0, vp);
    MV (0.0, 900.0, 0.0, ls.cent);
    ls.rad = 40;
    fprintf (fp, "%d %d\n", (int) ((XMAX - XMIN) * SCALE +0.9999999), (int) ((YMAX - YMIN) * SCALE +0.9999999));

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
