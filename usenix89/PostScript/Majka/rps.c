#include <stdio.h>
static char tohex[17];
static int npixo, copy;

/************************************************************************/
/*                                                                      */
/* This little program lets you print an "image" file on a Laser Writer */
/* using PostScript's "image" operator.  This program is based on the   */
/* example given in the PostScript Cookbook.  It is set up for a 300dpi */
/* machine.  The image dimensions default to 256*256, with 1 byte per   */
/* image pixel.  See the manual page for more details.                  */
/*                                                                      */
/************************************************************************/

main(argc,argv)
int argc;
char *argv[];
{
    FILE *ifp, *fopen();
    int r, c, nr, nc, row, col, sc, argn, win, im;
    int r1, c1, r2, c2, x1, y1, x2, y2; 
    unsigned char pix;
    char head[1024];
    double bscale, boff, temp, min, max, range;

    strcpy(tohex,"0123456789ABCDEF");
    npixo = 0;
    x1 = 0;
    y1 = 3300;
    x2 = 2550;
    y2 = 0;
    copy = 1;
    im = 0;
    nr = 256;
    nc = 256;

    for (argn = 1; argn < argc; argn++) {
        if (argv[argn][0] == '-') {
            switch (argv[argn][1]) {
                case 'w':
                    win = argn;
                    r1 = atoi(argv[++win]);
                    c1 = atoi(argv[++win]);
                    r2 = atoi(argv[++win]);
                    c2 = atoi(argv[++win]);
                    x1 = c1;
                    y1 = 3300 - r1;
                    x2 = c2;
                    y2 = 3300 - r2;
                    win = 1;
                    argn += 4;
                    break;
                case 'i':
                    nr = atoi(argv[++argn]);
                    nc = atoi(argv[++argn]);
                    break;
                case 'c': copy = atoi(argv[++argn]); break;
                default:
                    fprintf(stderr,
                    "iffps [file] [-w r1 c1 r2 c2] [-c n] [-i nr nc]\n");
                    fprintf(stderr,"  file  input image   [stdin]\n");
                    fprintf(stderr,"  -w    output window [0 0 3300 2550]\n");
                    fprintf(stderr,"  -c    copies        [1]\n");
                    fprintf(stderr,"  -i    input size    [256 256]\n");
                    exit(0);
            }
        }
        else {
             if (im) {
                fprintf(stderr,
                    "iffps [file] [-w r1 c1 r2 c2] [-c n] [-i nr nc]\n");
                exit(0);
            }
            im = 1;
            ifp = fopen(argv[argn],"r");
            if (ifp == NULL) {
                fprintf(stderr,"iffps: can't open image file %s\n",argv[argn]);
                exit(1);
            }
        }
    }

    if (!im) ifp = stdin;
    
    prologue(nr,nc,x1,y1,x2,y2);
    
    for (r = 0; r < nr; r++) 
        for (c = 0; c < nc; c++) {
             pix = getc(ifp);
             puthexpix(pix);
        }

    epilogue();
    
    fclose(ifp);
    exit(0);
}

prologue(nr,nc,x1,y1,x2,y2)
int nr,nc,x1,y1,x2,y2;
{
    printf("gsave\n");
    printf("initgraphics\n");
    printf("0.24 0.24 scale\n");
    printf("/imline %d string def\n",nc*2);
    printf("/drawimage {\n");
    printf("    %d %d 8\n",nc,nr);
    printf("    [%d 0 0 %d 0 %d]\n",nc,-1*nr,nr);
    printf("    { currentfile imline readhexstring pop } image\n");
    printf("} def\n");
    printf("%d %d translate\n",x1,y2);
    printf("%d %d scale\n",x2-x1,y1-y2);
    printf("drawimage\n");
}

epilogue()
{
    if (npixo) printf("\n");
    if (copy > 1) printf("%d {copypage} repeat\n",copy-1);
    printf("showpage\n");
    printf("grestore\n");
}

puthexpix(p)
unsigned char p;
{
    char hi, lo;

    hi = (p >> 4) & 15;
    lo = p & 15;
    
    printf("%c%c",tohex[hi],tohex[lo]);
    npixo += 1;
    if (npixo >= 32) {
        printf("\n");
        npixo = 0;
    }
}
