#include <stdio.h>
#include <sys/file.h>
#include <pixrect/pixrect_hs.h>

/*
 *	Dither!  Color dither a U.S.C. tape image.
 *
 *	Original Tape code by Brian Kantor.
 *
 *	The dithering is the product of madness and extrapolation
 *	from the concepts employed for black and white dithers,
 *	Jim Hutchison. Presumes 8bit bytes.
 */

#define LARGEST		(0xff)			/* 8 bits per color */
#define UNUSED_BITS	0			/* unused bits      */

#define R_BITS		3			/* Bits of red               */
#define R_ERR		(8 - R_BITS)		/* Bits of error, from true  */
#define R_SHIFT		(8 - R_BITS)		/* shift to get usefull bits */
#define R_MASK		(LARGEST >> R_BITS)	/* masked off bits           */
#define R_NEXT		(R_MASK - 1)		/* base_color + next = left  */
#define R_TOP		(LARGEST & ~R_MASK)	/* maximal value, red        */
#define R_NVAL		(LARGEST >> R_SHIFT)	/* shades of red + black     */
#define R_MAP		0x07			/* mask to get color table   */

#define G_BITS		3			/* Bits of green             */
#define G_ERR		(8 - G_BITS)		/* Bits of error, from true  */
#define G_SHIFT		(8 - G_BITS)		/* shift to get usefull bits */
#define G_MASK		(LARGEST >> G_BITS)	/* masked off bits           */
#define G_NEXT		(G_MASK - 1)		/* base_color + next = left  */
#define G_TOP		(LARGEST & ~G_MASK)	/* maximal value, green      */
#define G_NVAL		(LARGEST >> G_SHIFT)	/* shades of green + black   */
#define G_MAP		(0x07 << 3)		/* mask to get color table   */

#define B_BITS		2			/* Bits of blue              */
#define B_ERR		(8 - B_BITS)		/* Bits of error, from true  */
#define B_SHIFT		(8 - B_BITS)		/* shift to get usefull bits */
#define B_MASK		(LARGEST >> B_BITS)	/* masked off bits           */
#define B_NEXT		(B_MASK - 1)		/* base_color + next = left  */
#define B_TOP		(LARGEST & ~B_MASK)	/* maximal value, blue       */
#define B_NVAL		(LARGEST >> B_SHIFT)	/* shades of blue + black    */
#define B_MAP		(0x03 << 6)		/* mask to get color table   */

/*
 *	2(4(8)) by 2(4(8)) ordered dither, it all hinges on this dither,
 *	and the color-table.  Note that dithers larger than 4x4
 *	require changes in lower code.
 */

#ifdef	LARGE_DITHER

#define DSIZE		4	/* must be a power of 2 */
#define DITH_LOG	4	/* log[2](DSIZE*DSIZE) */
#define DMASK		DSIZE-1	/* Dither mask to get position in dither */

short dither[DSIZE][DSIZE] = {
	0,	8,	3,	11,
	12,	4,	15,	7,
	2,	10,	1,	9,
	14,	6,	13,	5
};

#else	/* LARGE_DITHER */

#define DSIZE		2	/* must be a power of 2 */
#define DITH_LOG	2	/* log[2](DSIZE*DSIZE) */
#define DMASK		DSIZE-1	/* Dither mask to get position in dither */

short dither[DSIZE][DSIZE] = {
    0,	3,
    2,  1
};

#endif	/* LARGE_DITHER */

/* Huge dither to use with the 2 bit color, blue
*/
#ifdef	BLUE_DITHER

#define BDSIZE		8	/* must be a power of 2 */
#define BDITH_LOG	6	/* log[2](BDSIZE*BDSIZE) */
#define BDMASK		BDSIZE-1/* Dither mask to get position in dither */

short bdither[BDSIZE][BDSIZE] = {
	0,	32,	12,	44,	3,	35,	15,	47,
	48,	16,	60,	28,	51,	19,	63,	31,
	8,	40,	4,	36,	11,	43,	7,	39,
	56,	24,	52,	20,	59,	27,	55,	23,
	2,	34,	14,	46,	1,	33,	13,	45,
	50,	18,	62,	30,	49,	17,	61,	29,
	10,	42,	6,	38,	9,	41,	5,	37,
	58,	26,	54,	22,	57,	25,	53,	21
};

#else	/* BLUE_DITHER */
#define BDITH_LOG	DITH_LOG
#endif	/* BLUE_DITHER */

/*
 * Determine if we have more error than we have dither, and
 * give the number of bits we shall have to shift down.
 */
#if (R_ERR - DITH_LOG) > 0
#define R_ISHIFT	(R_ERR - DITH_LOG)
#else
#define R_ISHIFT	(0)
#endif

#if (G_ERR - DITH_LOG) > 0
#define G_ISHIFT	(G_ERR - DITH_LOG)
#else
#define G_ISHIFT	(0)
#endif

#if (B_ERR - BDITH_LOG) > 0
#define B_ISHIFT	(B_ERR - BDITH_LOG)
#else
#define B_ISHIFT	(0)
#endif

/*
 * Image/colormap definitions.
 */
#define	MAPSIZE		256			/* size of pallet     */
#define	COLORS		256			/* number of colors   */
#define IMAGESIZE	512			/* size of tape image */
#define IMAGE_VOL	(IMAGESIZE*IMAGESIZE)	/* volume of image    */

#define MAXPATH		1024			/* max length of filename */

struct pixrect *display;
struct pixrect *memory_frame;

/* for palette (sun) generation */

unsigned char red[MAPSIZE], grn[MAPSIZE], blu[MAPSIZE];

/* shade tables (for speed) */

static unsigned int r_base_right[COLORS];
static unsigned int r_base_left[COLORS];
static unsigned int r_dval[COLORS];

static unsigned char g_base_right[COLORS];
static unsigned char g_base_left[COLORS];
static unsigned char g_dval[COLORS];

static unsigned char b_base_right[COLORS];
static unsigned char b_base_left[COLORS];
static unsigned char b_dval[COLORS];

/* nasty procedures */
int
min(a,b)
    int a,b;
{
    return((a > b)? b : a);
}

int
max(a,b)
    int a,b;
{
    return((a > b)? a : b);
}

main(argc,argv)
    int argc;
    char **argv;
{
    register unsigned int color, dith_value;
    register unsigned int r_color, g_color, b_color;
    register unsigned char *pr, *pg, *pb;

#ifdef	BLUE_DITHER
    unsigned int b_dith_value;
#endif	/* BLUE_DITHER */

    int x_imagesize, y_imagesize, image_vol;

    int fr, fg, fb;
    int i, plen;
    int x, y;
    char buf[MAXPATH];
    unsigned char *picture, *pict_row;	/* keep row to avoid padding problems */
    int row_bytes;			/* bytes per scan-line */

    if (argc < 2) {
	fprintf(stderr, "Usage: %s rgb-imagefile [hsize] [vsize]\n", argv[0]);
	exit(-1);
    }

    if (argc > 2) {
	x_imagesize = atoi(argv[2]);

	if (argc > 3) {
	    y_imagesize = atoi(argv[3]);
	} else {
	    y_imagesize = x_imagesize;
	}

	image_vol = y_imagesize * x_imagesize;

	pr = (unsigned char *) malloc(image_vol * sizeof(unsigned char));
	pg = (unsigned char *) malloc(image_vol * sizeof(unsigned char));
	pb = (unsigned char *) malloc(image_vol * sizeof(unsigned char));
    } else {
	x_imagesize = y_imagesize = IMAGESIZE;
	pr = (unsigned char *) malloc(IMAGE_VOL);
	pg = (unsigned char *) malloc(IMAGE_VOL);
	pb = (unsigned char *) malloc(IMAGE_VOL);
	image_vol = IMAGE_VOL;
    }

    printf("high %d wide %d vol %d\n", y_imagesize, x_imagesize, image_vol);

    strcpy(buf,argv[1]);
    plen = strlen(buf);
    buf[plen] = 'R';
    fr = open(buf, O_RDONLY, 0444);
    if (fr < 0) {
	perror(buf);
	exit(1);
    }

    buf[plen] = 'G';
    fg = open(buf, O_RDONLY, 0444);
    if (fg == 0) {
	perror(buf);
	exit(1);
    }

    buf[plen] = 'B';
    fb = open(buf, O_RDONLY, 0444);
    if (fb < 0) {
	perror(buf);
	exit(1);
    }

    display = pr_open("/dev/cgone0");
    if (display == NULL) {
	fprintf(stderr,"Color Display not available, sorry\n");
	exit (-1);
    }

    puts("Reading RGB files");

    if (read(fr, pr, image_vol) <= 0)
	    perror("red");
    if (read(fg, pg, image_vol) <= 0)
	    perror("grn");
    if (read(fb, pb, image_vol) <= 0)
	    perror("blu");

    puts("Creating memory pixrect");

    /* Get a pointer to a memory pixrect */
    memory_frame = mem_create(x_imagesize, y_imagesize, 8);

    /* Get a pointer to the image buffer associated with the memory pixrect */
    pict_row = picture = (unsigned char *) mpr_d(memory_frame)->md_image;

    /* Get bytes per scan-line (note that padding exists) */
    row_bytes = mpr_d(memory_frame)->md_linebytes;

    puts("Initializing tables");

    init_tables();

    puts("Processing image");

    for (y = 0; y < y_imagesize; y++, picture = pict_row += row_bytes) {
	for (x = 0; x < x_imagesize; x++) {

	    r_color = *pr++;
	    g_color = *pg++;
	    b_color = *pb++;

#ifdef	BLUE_DITHER
	    b_dith_value = bdither[x & BDMASK][y & BDMASK];
#endif	/* BLUE_DITHER */

	    dith_value = dither[x & DMASK][y & DMASK];

	    if (r_dval[r_color] > dith_value)
		color = r_base_left[r_color];
	    else
		color = r_base_right[r_color];

	    if (g_dval[g_color] > dith_value)
		color |= g_base_left[g_color];
	    else
		color |= g_base_right[g_color];

#ifdef	BLUE_DITHER
	    if (b_dval[b_color] > b_dith_value)
#else
	    if (b_dval[b_color] > dith_value)
#endif	/* BLUE_DITHER */
		color |= b_base_left[b_color];
	    else
		color |= b_base_right[b_color];

	    *picture++ = color;
	}
    }

    /*
     *	Generate colormap with (cycle is 8 values):
     *		32  cycles of red
     *		1   cycle  of green
     *		1/2 cycle  of blue
     *	all varying smoothely, note that an improved
     *	map which employs better graduation of color
     *	can be employed, but on our interlaced monitor
     *	this caused an extremely painful flicker.
     */

    puts("Building color lookup table");

    for (i=0; i < MAPSIZE; i++) {
	red[i] = (i & R_MAP) << R_SHIFT;
	grn[i] = (i & G_MAP) << (G_SHIFT - R_BITS);
	blu[i] = (i & B_MAP) << (B_SHIFT - R_BITS - G_BITS);
    }

    pr_putcolormap(display, 0, MAPSIZE, red, grn, blu);

    /* copy the complete image to the display, center it also.
    */

    puts("Displaying");

    pr_rop(display,
	   max((display->pr_width - x_imagesize) >> 1, 0),
	   max((display->pr_height - y_imagesize) >> 1,0),
	   min(display->pr_width, x_imagesize),
	   min(display->pr_height, y_imagesize),
	   (PIX_SRC|PIX_DONTCLIP), memory_frame, 0, 0);

    pr_close(display);
    close(fr);
    close(fg);
    close(fb);
}

init_tables()
{
    register unsigned int intensity, i;
    register double true_i;
    register double rfactor, gfactor, bfactor;

    rfactor = ((double)R_NVAL) / ((double)R_NVAL+1);
    gfactor = ((double)G_NVAL) / ((double)G_NVAL+1);
    bfactor = ((double)B_NVAL) / ((double)B_NVAL+1);

    for (i = 0; i < 256; i++) {
	true_i = (double) i;

	/*
	 * scale color to fit inside of range
	 * calculate right base shade by trimming off error.
	 */
	intensity = (unsigned int)(true_i * rfactor);
	r_base_right[i] = intensity & ~R_MASK;

	if (r_base_right[i] != R_TOP) {
	    r_dval[i] = (intensity & R_MASK) >> R_ISHIFT;
	    r_base_left[i] =
		((intensity + R_NEXT) & R_TOP) >> (UNUSED_BITS+G_BITS+B_BITS);
	} else
	    r_dval[i] = 0;

	r_base_right[i] >>= (UNUSED_BITS + G_BITS + B_BITS);

	/*
	 * scale color to fit inside of range
	 * calculate right base shade by trimming off error.
	 */
	intensity = (unsigned int)(true_i * gfactor);
	g_base_right[i] = intensity & ~G_MASK;

	if (g_base_right[i] != G_TOP) {
	    g_dval[i] = (intensity & G_MASK) >> G_ISHIFT;
	    g_base_left[i] =
		((intensity + G_NEXT) & G_TOP) >> (UNUSED_BITS + B_BITS);
	} else
	    g_dval[i] = 0;

	g_base_right[i] >>= (UNUSED_BITS + B_BITS);

	/*
	 * scale color to fit inside of range
	 * calculate right base shade by trimming off error.
	 */
	intensity = (unsigned int)(true_i * bfactor);
	b_base_right[i] = intensity & ~B_MASK;

	if (b_base_right[i] != B_TOP) {
	    b_dval[i] = (intensity & B_MASK) >> B_ISHIFT;
	    b_base_left[i] =
		((intensity + B_NEXT) & B_TOP) >> UNUSED_BITS;
	} else
	    b_dval[i] = 0;

	b_base_right[i] >>= UNUSED_BITS;
    }
}
