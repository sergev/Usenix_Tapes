
#include <stdio.h>

main(argc, argv)
    char **argv;
{
    int width, height, hskip, threshold;
    int pixel;
    register i, j, k, h, h1;
    register unsigned char b;

    width = 451;
    height = 451;
    fprintf(stderr, "Width %d, height %d\n", width, height);


    printf("@document(language impress)");
    putchar(213);	/* PAGE message- sets up parameters for Impress pg. */

    putchar(135);	/* SET-ABS-H: horiz. offset in points-req. 2 bytes */
    putchar(00) ;	/*offset currently set to 250/300 of in. in */
    putchar(250);	/*consideration of printer's margin error.  */

    putchar(137);	/* SET-ABS-V: horiz. offset in points-req. 2 bytes */
    putchar(00) ;
    putchar(250);	/* offset set to 250/300 of an in. */

    output_grey();
    putchar(209); /* SET_BOL */
    fputw(250, stdout);
    putchar(208); /* SET_IL */
    fputw(4, stdout);

    /* show grey scale */
    for ( i = 0; i < 10; i++ ) {
        for ( j = 0; j < 16; j++ )
            putchar(j+'A');
        putchar(197);
    }
    putchar(197);
    putchar(197);

    for ( i = 0; i < height; i++ ) {
        for ( j = 0; j < width; j++ ) {
            pixel = getchar() >> 4;
            putchar(pixel + 'A');
        }
        putchar(197);/*crlf*/
    }
    fprintf(stderr, "Done\n");
    exit(0);
}

char greys[16*4] = {
    0xf0, 0xf0, 0xf0, 0xf0        /* 1111 1111 1111 1111 */
    0xf0, 0x70, 0xf0, 0x70,       /* 1111 0111 1111 0111 */
    0xf0, 0x70, 0xe0, 0x70,       /* 1111 0111 1110 0111 */
    0xe0, 0x70, 0xe0, 0x70,       /* 1110 0111 1110 0111 */
    0xe0, 0x70, 0xe0, 0x30,       /* 1110 0111 1110 0011 */
    0xe0, 0x30, 0xe0, 0x30,       /* 1110 0011 1110 0011 */
    0xe0, 0x30, 0xc0, 0x30,       /* 1110 0011 1100 0011 */
    0xc0, 0x30, 0xc0, 0x30,       /* 1100 0011 1100 0011 */
    0xc0, 0x30, 0xc0, 0x10,       /* 1100 0011 1100 0001 */
    0xc0, 0x10, 0xc0, 0x10,       /* 1100 0001 1100 0001 */
    0xc0, 0x10, 0xc0, 0x00,       /* 1100 0001 1100 0000 */
    0x80, 0x20, 0x80, 0x20,       /* 1000 0010 1000 0010 */
    0x80, 0x20, 0x80, 0x00,       /* 1000 0010 1000 0000 */
    0x80, 0x00, 0x80, 0x00,       /* 1000 0000 1000 0000 */
    0x80, 0x00, 0x00, 0x00,       /* 1000 0000 0000 0000 */
    0x00, 0x00, 0x00, 0x00,       /* 0000 0000 0000 0000 */
};

output_grey()
{
    int i, j;

    for ( i = 0; i < 16; i++ ) {
        putc(199, stdout);    /* BGLY */
        fputw(i+'A', stdout); /* rotation 0, family, member */
        fputw(4, stdout);     /* advance_width */
        fputw(4, stdout);     /* width */
        fputw(0, stdout);     /* left offset */
        fputw(4, stdout);     /* height */
        fputw(4, stdout);     /* top offset */
        for ( j = 0; j < 4; j++ )
            putchar(greys[(i<<2)+j]);
    }
}

fputw(w, f)
    short w;
    FILE *f;
{
    char c;
    c = w>>8;
    putc(c, f);
    c = w;
    putc(c, f);
}
