/*
 * PRINTPIC: convert pic file to Postscript and print b&w halftone
 * on Laser Writer
 *
 * Paul Heckbert, PIXAR		25 April 86, 18 April 87
 * UUCP: {sun,ucbvax}!pixar!ph
 * ARPA: ph%pixar.uucp@ucbvax.berkeley.edu
 * please mail me any major enhancements you make
 *
 * notes:
 *   Postscript "image" command wants each scan line to start on
 *	byte boundary
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>

#include <pic.h>
#define STREQ(a, b) (strcmp(a, b)==0)
#define MAX(a, b) ((a)>(b) ? (a) : (b))

/* the following are system-dependent */
#define RANDOMRANGE2 (1<<30)	/* random() has range 0 to 2*RANDOMRANGE2-1 */
#define PVBITS 8
#define PVMAX 255

#define POINTSIZE 10
#define FONT "Helvetica-Oblique"
#define PAGEWID 8.5
#define PAGEHEI 11.
#define POINT(x) (int)((x)*72.)		/* convert inches to points */
#define UNINIT -666

static int wx1 = UNINIT, wx2, wy1, wy2;
static char *dev = 0, *outfile = 0, *label = 0;
static double ar = 1., size = 9., rot = 90., gam = 1.;
static double dither = 0.;		/* in nbits intensity space */
static int ditheramp;			/* in PVBITS intensity space */
static double cenx = PAGEWID/2., ceny = PAGEHEI/2.;
static double cr = .30, cg = .59, cb = .11, ca = 0.;	/* luminance coeffs */
static int nbits = 8, negate = 0;

static char Usage[] = "printpic [options] picfile\nMake halftone print of pic file on Laser Writer\n	Options:\n-P <printer>	print on named Laser Writer, default is \n-p <file>	output postscript to named file rather than spooling,\n		file name '-' means stdout\n-label <string>	label string printed below image\n-w <xmin> <xmax> <ymin> <ymax>		window to read\n-ar <ratio>	pixel aspect ratio, default is 1.\n-chan {rgba}	channel to print, default is luminance\n-bits <nbits>	number of bits per pixel to send, default is 8\n-gamma <gamma>	gamma, default is 1\n-negate		negate image\n-dither <amp>	dither amplitude, default is 0\n-size <size>	image size on page in inches, default is 9\n-rot <ang>	rotation in degrees ccw, default is 90 (landscape)\n-center <x> <y>	image center on page, inches from upper left, default 4.25 5.5\n";

static usage()
{
    fprintf(stderr, Usage);
    exit(1);
}

static badarg(opt)
char *opt;
{
    fprintf(stderr, "insufficient arguments to %s\n", opt);
    exit(1);
}

main(ac, av)
int ac;
char **av;
{
    char *picfile;
    int i, filecount;

    filecount = 0;
    for (i=1; i<ac; i++) {
	if (av[i][0]=='-') {
	    if (STREQ(av[i], "-P")) {
		if (i+1>=ac) badarg(av[i]);
		dev = av[++i];
	    }
	    else if (STREQ(av[i], "-p")) {
		if (i+1>=ac) badarg(av[i]);
		outfile = av[++i];
	    }
	    else if (STREQ(av[i], "-label")) {
		if (i+1>=ac) badarg(av[i]);
		label = av[++i];
	    }
	    else if (STREQ(av[i], "-w")) {
		if (i+4>=ac) badarg(av[i]);
		wx1 = atoi(av[++i]);
		wx2 = atoi(av[++i]);
		wy1 = atoi(av[++i]);
		wy2 = atoi(av[++i]);
	    }
	    else if (STREQ(av[i], "-ar")) {
		if (i+1>=ac) badarg(av[i]);
		ar = atof(av[++i]);
	    }
	    else if (STREQ(av[i], "-chan")) {
		if (i+1>=ac) badarg(av[i]);
		switch (av[++i][0]) {
		    case 'r': cr = 1.; cg = 0.; cb = 0.; ca = 0.; break;
		    case 'g': cr = 0.; cg = 1.; cb = 0.; ca = 0.; break;
		    case 'b': cr = 0.; cg = 0.; cb = 1.; ca = 0.; break;
		    case 'a': cr = 0.; cg = 0.; cb = 0.; ca = 1.; break;
		    default : fprintf(stderr, "bad channel: %c\n", av[i][0]);
			exit(1);
		}
	    }
	    else if (STREQ(av[i], "-bits")) {
		if (i+1>=ac) badarg(av[i]);
		nbits = atoi(av[++i]);
	    }
	    else if (STREQ(av[i], "-gamma")) {
		if (i+1>=ac) badarg(av[i]);
		gam = atof(av[++i]);
	    }
	    else if (STREQ(av[i], "-negate")) {
		negate = 1;
	    }
	    else if (STREQ(av[i], "-dither")) {
		if (i+1>=ac) badarg(av[i]);
		dither = atof(av[++i]);
	    }
	    else if (STREQ(av[i], "-size")) {
		if (i+1>=ac) badarg(av[i]);
		size = atof(av[++i]);
	    }
	    else if (STREQ(av[i], "-rot")) {
		if (i+1>=ac) badarg(av[i]);
		rot = atof(av[++i]);
	    }
	    else if (STREQ(av[i], "-cen")) {
		if (i+2>=ac) badarg(av[i]);
		cenx = atof(av[++i]);
		ceny = atof(av[++i]);
	    }
	    else usage();
	}
	else if (++filecount==1) picfile = av[i];
	else usage();
    }

    printpic(picfile);
}

printpic(file)
char *file;
{
    char command[40];
    FILE *ofp;
    pic *p;

    if (nbits!=1 && nbits!=2 && nbits!=4 && nbits!=8) {
	fprintf(stderr, "nbits must be power of two\n");
	exit(1);
    }
    /*
     * strange printer behavior: if you ask for 8 bits you get only 5.
     * dither accordingly
     */
    ditheramp = dither*(1 << PVBITS-(nbits==8 ? 5 : nbits));

#   ifdef DEBUG
	fprintf(stderr, "ar=%g size=%g rot=%g cen=(%g,%g) nbits=%d file=%s\n",
	    ar, size, rot, cenx, ceny, nbits, file);
	fprintf(stderr, "gam=%g negate=%d win={%d,%d,%d,%d}\n",
	    gam, negate, wx1, wx2, wy1, wy2);
	fprintf(stderr, "dither=%g ditheramp=%d\n", dither, ditheramp);
	fprintf(stderr, "coeffs=(%g,%g,%g,%g)\n", cr, cg, cb, ca);
#   endif

    p = pic_open(file, "r");
    if (p==NULL) {
	fprintf(stderr, "nonexistent or bad picture file: %s\n", file);
	exit(1);
    }

    if (outfile==0) {		/* spool to printer */
	if (dev) sprintf(command, "lpr -P%s", dev);
	else strcpy(command, "lpr");
	ofp = popen(command, "w");
	if (ofp==NULL) {
	    fprintf(stderr, "croak: can't run 'lpr' program\n");
	    exit(1);
	}
    }
    else {			/* create file */
	if (STREQ(outfile, "-")) ofp = stdout;
	else ofp = fopen(outfile, "w");
	if (ofp==NULL) {
	    fprintf(stderr, "can't create %s\n", outfile);
	    exit(1);
	}
    }

    pic_to_ps(p, ofp);

    if (outfile==0) pclose(ofp);
    else if (ofp!=stdout) fclose(ofp);
}

/* pic_to_ps: read picture file p, write postscript to stream ofp */

pic_to_ps(p, ofp)
pic *p;
FILE *ofp;
{
    int tx, ty, dx, dy, x, y, i, gray, table[PVMAX+1];
    pic_pixel *line, *lp;

    pic_read_size(p, &tx, &ty);
    if (wx1==UNINIT) {
	wx1 = 0; wx2 = tx-1;
	wy1 = 0; wy2 = ty-1;
    }
    else if (wx1<0 || wx2>=tx || wy1<0 || wy2>=ty) {
	fprintf(stderr, "window larger than image\n");
	exit(1);
    }
    dx = wx2-wx1+1;		/* window size */
    dy = wy2-wy1+1;

    fprintf(stderr, "%dx%dx%d-bit\n", dx, dy, nbits);
    for (i=0; i<=PVMAX; i++) {	/* initialize intensity table */
	gray = negate ? PVMAX-i : i;
	table[i] = PVMAX.*pow(gray/PVMAX., gam) + .5;
    }

    ps_head(ofp);
    ps_image_head(ofp, dx, dy);

    /*
     * decode picture file, ignoring lines before wy1 and skipping those
     * after wy2
     */
    assert(line = (pic_pixel *)malloc(tx*sizeof(pic_pixel)));
    for (y=wy1; y<=wy2; y++) {
	pic_read_line(p, y, line);
	for (lp=line+wx1, x=wx1; x<=wx2; x++, lp++) {
	    gray = cr*lp->r + cg*lp->g + cb*lp->b + ca*lp->a;
	    if (gray<0) gray = 0;
	    else if (gray>PVMAX) gray = PVMAX;
	    gray = table[gray];		/* intensity mapping */
	    if (ditheramp) {
		/* perturb gray by a random number between -amp and amp */
		gray += random()/(RANDOMRANGE2/ditheramp) - ditheramp;
		if (gray<0) gray = 0;
		else if (gray>PVMAX) gray = PVMAX;
	    }
	    gray >>= PVBITS-nbits;	/* quantize */
	    ps_pixel(ofp, gray, nbits);
	}
	ps_pixelflush(ofp, nbits);
    }
    pic_close(p);

    ps_tail(ofp);
}

/*-------------------- postscript nitty-gritty --------------------*/

ps_head(ofp)
FILE *ofp;
{
    char host[20];
    int curtime;

    gethostname(host, sizeof host);
    curtime = time(0);
    fprintf(ofp, "%%!PS-Adobe-1.0\n");
    fprintf(ofp, "%%%%Creator: %s:%s\n", host, getlogin());
    fprintf(ofp, "%%%%CreationDate: %s", ctime(&curtime));
    fprintf(ofp, "%%%%EndProlog\n");
    fprintf(ofp, "%%%%Page: ? 1\n\n");
}

ps_image_head(ofp, dx, dy)
FILE *ofp;
int dx, dy;
{
    int strlen;
    double max, wid, hei;

    strlen = (dx*nbits+7)/8;	/* this must be right or no output! */
    fprintf(ofp, "/picstr %d string def\n", strlen);
    max = MAX(dx*ar, dy);
    wid = size*dx*ar/max;
    hei = size*dy/max;
    fprintf(ofp, "%d %d translate %g rotate %d %d translate\n",
	POINT(cenx), POINT(PAGEHEI-ceny), rot, POINT(-wid/2.), POINT(-hei/2.));
    if (label) {
	fprintf(ofp, "/%s findfont %d scalefont setfont\n", FONT, POINTSIZE);
	ps_quote_string(ofp, label);
	fprintf(ofp, " 0 %g moveto show\n", -1.5*POINTSIZE);
    }
    fprintf(ofp, "%d %d scale\n\n", POINT(wid), POINT(hei));

    fprintf(ofp, "%d %d %d [%d 0 0 %d 0 %d]\n", dx, dy, nbits, dx, -dy, dy);
    fprintf(ofp, "{currentfile picstr readhexstring pop}\n");
    fprintf(ofp, "image\n");
}

ps_tail(ofp)
FILE *ofp;
{
    fprintf(ofp, "\nshowpage\n");
}

/* ps_quote_string: print string str to stream ofp, quoted for postscript */

ps_quote_string(ofp, str)
FILE *ofp;
char *str;
{
    putc('(', ofp);
    for (; *str; str++) {
	switch (*str) {
	    case '(':
	    case ')':
	    case '\':
		putc('\', ofp);
		break;
	}
	putc(*str, ofp);
    }
    putc(')', ofp);
}

static int accum = 0, count = 0, col = 0;

/* ps_pixelflush: pad to finish this byte */

ps_pixelflush(ofp, n)
FILE *ofp;
int n;
{
    while (count!=0) ps_pixel(ofp, 0, n);
    if (col!=0) {
	fprintf(ofp, "\n");
	col = 0;
    }
}

/*
 * ps_pixel: accumulate n bit data, where n = 1, 2, 4, or 8,
 * outputting 2 hex digits for each byte
 */

ps_pixel(ofp, data, n)
FILE *ofp;
int data, n;
{
    accum = accum<<n | data;
    count += n;
    if (count>=8) {
	fprintf(ofp, "%02x", accum);
	accum = 0;
	count = 0;
	col += 2;
	if (col>=72) {
	    fprintf(ofp, "\n");
	    col = 0;
	}
    }
}
