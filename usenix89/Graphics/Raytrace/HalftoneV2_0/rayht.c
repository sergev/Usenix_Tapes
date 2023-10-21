/*
 * traceconv.c - a program to convert 256 level gray files to halftone
 *               pictures.  Produces output in format for printing on
 *               IMAGEN printers using language BITARRAY (compatible with
 *               imagen IMAGESERVER printers running V2.2 and later sw).
 * Written by: Steve Hawley   Aztec C V1.06H
 * Modified to produce language BITARRAY for imagen printers by
 *   Geoffrey Cooper at IMAGEN (August 12, 1986).  The main procedure
 *   is basically new.  The dither function is borrowed from Steve's
 *   code.
 */
#include <stdio.h>

/* Magnification - must be a power of two (fix it if you don't like it) */
/* (and send me the fixes :-).  MAGSHFT is log base 2 of MAG */
#define MAG 4
#define MAGSHFT 2
unsigned char line[1000];

main (argc, argv)
    char **argv;
{
    int height, width, wremainder;
    register int x, y, x1, mx, my;
    register int level;
    FILE *fopen(), *fp;
    register unsigned long b;
    
    /* put up a new window */
    if ( (fp = fopen(argv[1], "r")) == NULL ) {
            perror("on file open");
            exit(1);
    }
    /* Get dimensions */
    fscanf(fp, "%d %d\n", &width, &height);
    wremainder = width & 31;
    width  &= ~31;
    height &= ~31;

    fprintf(stdout, "@document(language bitarray, width %d, height %d, leftoffset 256, topoffset 256, File \"%s\")",
                    width<<MAGSHFT, height<<MAGSHFT, argv[1]);
    b = 0;
    for ( y = 0; y < height; y++ ) {
        for ( x = 0; x < width; x++ )
            line[x] = getc(fp);
        for ( x = 0; x < wremainder; x++ ) getc(fp);
        for ( my = 0; my < MAG; my++ ) {
            for ( x = 0; x < width; x += (32>>MAGSHFT) ) {
                b = 0;
                for ( x1 = 0; x1 < (32>>MAGSHFT); x1++ ) {
                    for ( mx = 0; mx < MAG; mx++ ) {
                        level = line[x+x1];
                        b = (b << 1) | dither(((x+x1)<<MAGSHFT)+mx, (y<<MAGSHFT)+my, level);
                    }
                }
                putc(b>>24, stdout);
                putc(b>>16, stdout);
                putc(b>>8 , stdout);
                putc(b    , stdout);
            }
        }
    }
}

#define MAXLEV
#ifdef MAXLEV
/*
 * The following dithering function creates little boxes whose
 * size is proportional to the level and x/y position of the
 * point, quantized to an 8X8 grid.  An exponential scale of gray levels
 * is used, corresponding closer to the human eye's luminance
 * response than a linear scale.
 *
 * The result is a little light; probably it can be improved by using
 * a less steep exponential than I did (I mapped the curve of exp(x)
 * from -5 to 0 -- the curve of exp(x/1.5) looks a bit better).
 * 
 * If you change the map, don't forget to make sure that 0 and 255 are
 * left out.  That way the whites and blacks can be solid.
 *
 * The dmask is the cascade of two functions ( grid * expluminance ).
 * the cascading has been done manually, to save (miniscule) time.
 * the two functions are reproduced below:
 *
 *    static unsigned char expluminance[64] = {
 *      254, 235, 217, 201, 186, 172, 159, 147, 
 *      136, 125, 116, 107,  99,  91,  84,  78, 
 *       72,  66,  61,  57,  52,  48,  44,  41, 
 *       38,  35,  32,  30,  27,  25,  23,  21, 
 *       20,  18,  16,  15,  14,  13,  12,  11, 
 *       10,   9,   8,   7,   7,   6,   6,   5, 
 *        5,   4,   4,   3,   3,   3,   2,   2, 
 *        2,   1,   1,   1,   1,   1,   1,   1
 *     };
 *     static unsigned char grid[64] = {
 *       63,  62,  61,  60,  59,  58,  57,  56, 
 *       36,  35,  34,  33,  32,  31,  30,  55, 
 *       37,  16,  15,  14,  13,  12,  29,  54, 
 *       38,  17,   4,   3,   2,  11,  28,  53, 
 *       39,  18,   5,   0,   1,  10,  27,  52, 
 *       40,  19,   6,   7,   8,   9,  26,  51, 
 *       41,  20,  21,  22,  23,  24,  25,  50, 
 *       42,  43,  44,  45,  46,  47,  48,  49
 *     };
 */
dither(x, y, level)
    int x, y, level;
{
    int dlev;   /* dither level */
    static unsigned char dmask[64] = {
       1,   1,   1,   1,   1,   1,   1,   2, 
      14,  15,  16,  18,  20,  21,  23,   2, 
      13,  72,  78,  84,  91,  99,  25,   2, 
      12,  66, 186, 201, 217, 107,  27,   3, 
      11,  61, 172, 254, 235, 116,  30,   3, 
      10,  57, 159, 147, 136, 125,  32,   3, 
       9,  52,  48,  44,  41,  38,  35,   4, 
       8,   7,   7,   6,   6,   5,   5,   4
    };

    /* apply an 8x8 grid: */
    dlev = ((y & 7) << 3) + (x & 7);

    /* apply dithering function and compare */
    return ( level < dmask[dlev] );
}
#endif

/*
 * The following is Steve Hawley's mask.  Looks to me like a "sinx/x" type
 * function.  It is pleasing to the eye, but comes out rather dark on
 * the printer.
 */
/*#define ORIG*/
#ifdef ORIG
dither (x,y,level) /* dithering function */
register int x, y, level;
{
     /* 8 x 8 dithering matrix */
    static int matrix[8][8] = {
        {  0, 128,  32, 160,   8, 136,  40, 168}, 
        {192,  64, 224,  96, 200,  72, 232, 104}, 
        { 48, 176,  16, 144,  56, 184,  24, 152}, 
        {240, 112, 208,  80, 248, 120, 216,  88}, 
        { 12, 140,  44, 174,   4, 132,  36, 164}, 
        {204,  76, 236, 108, 196,  68, 228, 100}, 
        { 60, 188,  28, 156,  52, 180,  20, 148}, 
        {252, 124, 210,  92, 244, 116, 212,  84}
    };

/* dithering by area access to matrix by using
    the MOD function on the x and y coordinates */
    return(level < matrix[x & 7][y & 7]);
}
#endif
