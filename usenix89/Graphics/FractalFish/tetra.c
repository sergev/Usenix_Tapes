/*
 *	Draw a recursive tetrahedron ala Mandelbrot (page 142,
 *	"The Fractal Geometry of Nature").
 *
 *	Produces sun 'shaded' format unless compiled with
 *	BLINN defined (I can find little other attribution).
 *
 *	Usage: a.out [-l<len> -m<minlen> -x<xcent> -y<ycent> -z<zcent> -f<file>]
 *
 *		len		length of an edge
 *		minlen		the length of subdivided edge at which to stop
 *		xcent		x center of object
 *		ycent		y center of object
 *		zcent		z center of object
 *		file		output file
 *
 *
 *	Author: Jim Hutchison <hutch@sdcsvax.ucsd.edu>
 *
 *	Feel free to do anything with this code except remove this comment.
 *
 */

/* Informational drawing
 *	[a,d] are the original points, and [0,5] are the
 *	points created for the next generation of tetrahedrons.
 *	'*'ed points are vertically elevated.
 *
 *	      /\c
 *	     /  \
 *	    /    \
 *	   /  *5  \
 *	  /        \
 *       2----------1
 *      /\         /\
 * ^   /  \   *d  /  \
 * |  /    \     /    \
 * | /  *3  \   /  *4  \
 * |/        \ /        \
 * a----------0----------\b--->
 */

#include <stdio.h>

#define DEF_OUTFILE	"./tropicalfish"	/* BM says tropical fish */
#define TEMP_FILE	"/tmp/tetraXXXXXX"	/* temp file name, need 6 X's */

#define SQRT2		1.4142135623731		/* C.R.C. constants */
#define SQRT3		1.7320508075688
#define SQRT3over4	0.4330127018922		/* sqrt(3.0) / 4.0  */
#define SQRT3over8	0.2165063509461		/* sqrt(3.0) / 8.0  */

#define DEF_LEN		160.0			/* default start length */
#define DEF_MIN		40.0			/* default min length */
#define DEF_CX		0.0			/* default x center */
#define DEF_CY		0.0			/* default y center */
#define DEF_CZ		0.0			/* default z center */

#define OUTBUFSIZ	128

typedef struct {		/* a point in space */
    double x, y, z;
    int id;			/* its ordinal number, for vertice definition */
} POINT;

char *malloc(), *strcpy();

static double mindist;		/* minimum distance between vertices */
static int pointcnt = 0;	/* current point count */
static int polycnt = 0;		/* current polygon count */

#ifdef BLINN

static FILE *fout;		/* outfile pointer */

#else

static char *tmppoly;		/* names of temporary files */
static char *tmppoint;

static FILE *fpoint,*fpoly;	/* point and poylgon file pointers */

#endif

main(argc, argv)
    int argc;
    char **argv;
{
    int ch;
    char outfilename[1024];
    double min, cx, cy, cz, len, half_len, projected_len;
    static char options[] = "[-l<len> -m<minlen> -x<xcenter> -y<ycenter> -z<zcenter> -f<outfile>]";

    extern int optind;
    extern char *optarg;

    /* DEFAULTS */

    len = DEF_LEN;		/* length of a side at start */
    min = DEF_MIN;		/* length of a side at which to stop  */
    cx = DEF_CX;		/* Center of mass */ 
    cy = DEF_CY;
    cz = DEF_CZ;
    strcpy(outfilename,DEF_OUTFILE);	/* sun 'shaded' file */

    while((ch = getopt(argc,argv,"l:m:x:y:z:f:HELP")) != EOF)
	switch(ch) {
	    case 'l':
		sscanf(optarg,"%lf",&len);
		break;
	    case 'm':
		sscanf(optarg,"%lf",&min);
		if (min <= 0.0)
		    die("Min can not be less than or equal to 0.");
		break;
	    case 'x':
		sscanf(optarg,"%lf",&cx);
		break;
	    case 'y':
		sscanf(optarg,"%lf",&cy);
		break;
	    case 'z':
		sscanf(optarg,"%lf",&cz);
		break;
	    case 'f':
		strcpy(outfilename,optarg);
		if (*outfilename == NULL)
		    die("Null output file name.");
		break;
	    case 'H':
	    default:
		fprintf(stderr,"Usage: %s %s\n",argv[0],options);
		exit(-1);
	 }

#ifdef BLINN
    if ((fout = fopen(outfilename,"w")) == NULL)
	die("Could not open output file");

    fprintf(fout,"\tFRACTAL SKEWED WEB, HAUSDORFF DIMENSION = 2.0\n");
    fprintf(fout,"\tCX %4.7f, CY %4.7f, CZ %4.7f\n", cx, cy, cz);
    fprintf(fout,"\tLEN %4.7f, MIN %4.7f\n\n", len, min);
#else
    init_scratchfiles();		/* point & polygon scratch files */
#endif

    half_len = len / 2.0;		/* half the length of a side */
					/* half a side projected onto y/z */
    projected_len = (half_len / 2.0) * SQRT3;

    cx = cx - half_len;			/* determine a vertex from center */
    cy = cy - projected_len;
    cz = cz - projected_len;

#ifdef sun && DEBUG
    malloc_debug(2);	/* check whole heap on every call */
#endif

    tetrahedrons( cx, cy, cz, min, len);

#ifdef BLINN
    fclose(fout);
#else
    end_files(outfilename,cx,cy,cz,len); /* sweep it all together and close */
#endif

    exit(0);
}

/*
 *	Create a point structure containing x,y,z and return the pointer.
 */
POINT *
pcreate(x, y, z)
    double x, y, z;
{
    POINT *p;

    extern int pointcnt;
    extern FILE *fpoint, *fout;

    p = (POINT *)malloc(sizeof(POINT));
    if (p == NULL)
	die("Out of Core in pcreate()");

    p->x = x;
    p->y = y;
    p->z = z;
    p->id = ++pointcnt;

#ifdef BLINN
    fprintf(fout, "PNT %d, %4.7f, %4.7f, %4.7f\n", p->id, x, y, z);
#else
    fprintf(fpoint, "%4.7f\t%4.7f\t%4.7f\n", x, y, z);
#endif

    return(p);
}

/*
 *	Make the 'prime' points, the points of a new generation.
 *
 *	p[0] is point 0 on the 'sketch', etc.
 */
makeprime(a, p, n)
    POINT *a, *p[6];
    register double n;
{
    register double ax, ay, az, n1, n2, nhalf, n3fourth, nfourth;

    ax = a->x;
    ay = a->y;
    az = a->z;

    nfourth = n / 4.0;
    nhalf = n / 2.0;
    n3fourth = 1.5 * nhalf;

    n1 = n * SQRT3over4;
    n2 = n * SQRT3over8;

    p[0] = pcreate(ax + nhalf,    ay,      az);
    p[1] = pcreate(ax + n3fourth, ay,      az + n1);
    p[2] = pcreate(ax + nfourth,  ay,      az + n1);
    p[3] = pcreate(ax + nfourth,  ay + n1, az + n2);
    p[4] = pcreate(ax + n3fourth, ay + n1, az + n2);
    p[5] = pcreate(ax + nhalf,    ay + n1, az + 3.0 * n2);
}

/*
 *	Output a tetrahedron
 */
drawtetra(a, b, c, d)
    POINT *a, *b, *c, *d;
{
    extern int polycnt;
    extern FILE *fpoly, *fout;

    polycnt += 4;

#ifdef BLINN
    fprintf(fout,"POLY %5d, %5d, %5d\n", a->id, b->id, c->id);
    fprintf(fout,"POLY %5d, %5d, %5d\n", a->id, b->id, d->id);
    fprintf(fout,"POLY %5d, %5d, %5d\n", a->id, c->id, d->id);
    fprintf(fout,"POLY %5d, %5d, %5d\n", b->id, c->id, d->id);
#else
    fprintf(fpoly,"3 %5d %5d %5d\n", a->id, b->id, c->id);
    fprintf(fpoly,"3 %5d %5d %5d\n", a->id, b->id, d->id);
    fprintf(fpoly,"3 %5d %5d %5d\n", a->id, c->id, d->id);
    fprintf(fpoly,"3 %5d %5d %5d\n", b->id, c->id, d->id);
#endif
}

/*
 *	Create the seed and begin the generation cycle.
 */
tetrahedrons(x, y, z, min, n)
    double x, y, z, min, n;
{
    POINT *a, *b, *c, *d;

    extern double mindist;

    mindist = min;

    a = pcreate(x,           y,                   z);
    b = pcreate(x + n,       y,                   z);
    c = pcreate(x + n / 2.0, y,                   z + n * SQRT3 / 2.0);
    d = pcreate(x + n / 2.0, y + n * SQRT3 / 2.0, z + n * SQRT3 / 4.0);

    tetra(a, b, c, d, n);
}

/*
 *	Run the generation cycle
 */
tetra(a, b, c, d, n)
    POINT *a, *b, *c, *d;
    double n;
{
    extern double mindist;

    if (n < mindist)
	drawtetra(a, b, c, d);
    else {
	POINT *p[6];

	makeprime(a, p, n);

	n /= 2.0;

	/* rad! a diagonally symetric recursion matrix!!!
	*/
	tetra(   a, p[0], p[2], p[3], n);
	tetra(p[0],    b, p[1], p[4], n);
	tetra(p[2], p[1],    c, p[5], n);
	tetra(p[3], p[4], p[5],    d, n);

#ifndef FAT
	free(p[0]);	/* if have lots of memory, can remove these for speed */
	free(p[1]);
	free(p[2]);
	free(p[3]);
	free(p[4]);
	free(p[5]);
#endif
    }
}

/*
 *	Spew a message on stderr, and die.
 */
die(message)
    char *message;
{
    fprintf(stderr,"Error: %s\n",message);
    exit(-1);
}

#ifndef BLINN		/* <----- Cut Point */

/*
 *	Open the scratch point and polygon files
 *	Set value of tmppoint and tmppoly so that
 *	we can unlink them later.  Will not do that
 *	now because we are told 4.3 may break that ``technique''.
 */
init_scratchfiles()
{
    extern FILE *fpoint, *fpoly;
    extern char *tmppoint, *tmppoly;
    char *mktemp();

    tmppoint = mktemp(TEMP_FILE);

    fpoint = fopen(tmppoint,"w+");
    if (fpoint == NULL)
	die("Could not open input file for writing");

    tmppoly = mktemp(TEMP_FILE);

    fpoly = fopen(tmppoly,"w+");
    if (fpoly == NULL)
	die("Could not open scratch polygon file for writing");
}

/*
 *	Open the output file, fill it from the scratch files
 *	and then close them all.  Use the center and length
 *	to generate the bounding box information.
 */
end_files(outfile,cx,cy,cz,len)
    char *outfile;
    double cx, cy, cz, len;
{
    int i;
    register FILE *fout;
    char buf[OUTBUFSIZ];
    double projectedlen;

    fout = fopen(outfile,"w");
    if (fout == NULL)
	die("Could not open output file");

    /* Inventory */

    fprintf(fout,"%d %d\n", pointcnt, polycnt);

    /* Bounding box */

    projectedlen = len * SQRT3 / 2.;

    fprintf(fout,"%-4.7f %-4.7f ", cx, cx + len);
    fprintf(fout,"%-4.7f %-4.7f ", cy, cy + projectedlen);
    fprintf(fout,"%-4.7f %-4.7f\n",cz, cz + projectedlen);

    /* Points */

    fflush(fpoint);		/* flush output and rewind */
    rewind(fpoint);

    for (i = pointcnt; i > 0; i--) {	/* copy scratch file into output file */
	fgets(buf,OUTBUFSIZ,fpoint);
	fputs(buf,fout);
    }

    fclose(fpoint);		/* close and unlink the scratch points file */
    unlink(tmppoint);

    /* Polygons */

    fflush(fpoly);		/* flush output and rewind */
    rewind(fpoly);

    for (i = polycnt; i > 0; i--) {	/* copy scratch file into output file */
	fgets(buf,OUTBUFSIZ,fpoly);
	fputs(buf,fout);
    }

    fclose(fpoly);		/* close and unlink the scratch points file */
    unlink(tmppoly);

    fclose(fout);
}

#endif			/* <----- Cut Point */
