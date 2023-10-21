/*************************************************************/
/*                                                           */
/*  Copyright (c) 1987                                       */
/*  Marc S. Majka - UBC Laboratory for Computational Vision  */
/*                                                           */
/*  Permission is hereby granted to copy all or any part of  */
/*  this program for free distribution.   The author's name  */
/*  and this copyright notice must be included in any copy.  */
/*                                                           */
/*************************************************************/

/* sunview - read a raster image file, halftone and scale it */
/* to fit into a suntools graphics window.  Does not handle  */
/* damage to the window.  Crufty and slow, but it works.     */

#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <sunwindow/rect.h>
#include <sunwindow/rectlist.h>
#include <sunwindow/pixwin.h>
#include <suntool/gfxsw.h>
#include <stdio.h>

/* screen output area */
int c1, r1, c2, r2, dw, dh;

/* output window */
struct pixwin *pw;
struct gfxsubwindow *gfx;

/* memory image pixrect */
struct pixrect *impr;

/* The whole thing goes in core */
#define MAX 512
unsigned char image[MAX][MAX];

/* Modulating signal */
#define MAXP 16
int period, modsignal[MAXP][MAXP];

main (argc, argv)
int argc;
char *argv[];
{
	FILE *fopen(), *ifp;
	int row, col, nrows, ncols;
	int done, pcnt, next;
	int gotimage, argn;
	float sin(), alpha, beta, delta, sinalpha, ampl, atof();

	gotimage = 0;
	
	/* Frobs to play with if you are so inclined */
	/* Amplitude of modulating signal */
	ampl = 75.0;

	/* Period of modulating signal */
	period = 5;

	/* assume a 256 * 256 input image */
	nrows = 256;
	ncols = 256;

	for (argn = 1; argn < argc; argn++) {
		if (argv[argn][0] == '-') {
			switch (argv[argn][1]) {
				case 'a': /* Amplitude */
					ampl = atof(argv[++argn]);
					break;
				case 'p': /* period */
					period = atoi(argv[++argn]);
					if (period > MAXP) {
						fprintf(stderr,"Maximum period is %d\n",MAXP);
						exit(1);
					}
					break;
				case 'r': /* nrows */
					nrows = atoi(argv[++argn]);
					if (nrows > MAX) {
						fprintf(stderr,"Maximum nrows is %d\n",MAX);
						exit(1);
					}
					break;
				case 'c': /* ncols */
					ncols = atoi(argv[++argn]);
					if (ncols > MAX) {
						fprintf(stderr,"Maximum ncols is %d\n",MAX);
						exit(1);
					}
					break;
				default:
					usage();
					exit(0);
			}
		}
		else {
			if (gotimage) {
					usage();
					exit(0);
			}

			gotimage = 1;
			ifp = fopen(argv[argn],"r");
			if (ifp == NULL) {
				fprintf(stderr,"can't open image file %s\n",argv[argn]);
				exit(1);
			}
		}
	}

	if (!gotimage) ifp = stdin;

	done = 0;
	pcnt = 0;
	next = 0;
	fprintf(stderr,"reading image ...\n");
	for (row = 0; row < nrows; row++) {
		for (col = 0; col < ncols; col++) image[row][col] = getc(ifp);

		done += 100;
		pcnt = done / nrows;
		if (pcnt >= next) {
			fprintf(stderr,"%d%% ",pcnt);
			fflush(stderr);
			next += 10;
		}
	}
	fprintf(stderr,"\n");
	
	fclose(ifp);

	/* Open gfx subwindow */
	gfx = gfxsw_init(0,NULL);
	pw = gfx->gfx_pixwin;

	/* Initialize a table (modsignal) containing the modulating signal */
	delta = 3.141592657 * 2.0 / (float)period;
	for (row = 0, alpha = 0.0; row < period; row++, alpha += delta) {
		sinalpha = sin(alpha);
		for (col = 0, beta = 0.0; col < period; col++, beta += delta)
			modsignal[row][col] = sin(beta) * ampl * sinalpha;
	}

	start();
	resize(nrows,ncols);
	pw_rop(pw,c1,r1,dw,dh,PIX_SRC,impr,0,0);
	fprintf(stderr,"Type <return> to erase screen and quit." );
	getchar();
	exit(0);
}

/* get output window size and clear it */
start()
{
	struct rect rect;
	
	win_getsize(gfx->gfx_windowfd,&rect);
	c1 = rect.r_left;
	r1 = rect.r_top;
	dw = gfx->gfx_rect.r_width;
	dh = gfx->gfx_rect.r_height;
	c2 = rect_right(&rect);
	r2 = rect_bottom(&rect);
	pw_writebackground(gfx->gfx_pixwin,0,0,rect.r_width,rect.r_height,PIX_CLR);
}

/* resize nrows * ncols to fit in output window */
/* modulation-style halftoning is done on the fly */
resize(nr,nc)
int nr,nc;
{
	struct pixrect *mem_create();
	float x, y, dx, dy;
	int r0, c0, a, q, r, c, rows, cols, s, t, done, pcnt, next;
	
	pcnt = 0;
	done = 0;
	next = 0;
	fprintf(stderr,"halftoning image ...\n");
	
	impr = mem_create(dw,dh,1);
	
	rows = dh;
	cols = dw;
	t = 0;
	
	dx = (nr * 1.0) / dh;
	dy = (nc * 1.0) / dw;

	for (x = 0.0, r = 0; r < rows; x += dx, r++) {
		t = (t + 1) % period;
		s = 0;
		r0 = x;
		
		for (y = 0.0, c = 0; c < cols; y += dy, c++) {
			c0 = y;
			s = (s + 1) % period;
			a = image[r0][c0];
			q = (int)(a + modsignal[t][s]) <= 127; 
			pr_put(impr,c,r,q);

		}

		done += 100;
		pcnt = done / rows;
		if (pcnt >= next) {
			fprintf(stderr,"%d%% ",pcnt);
			fflush(stderr);
			next += 10;
		}
	}
	fprintf(stderr,"\n");
}

usage()
{
	fprintf(stderr,"sunview [image] [-r %%d] [-c %%d] [-a %%f] [-p %%d]\n");
	fprintf(stderr,"    image - input raster image file\n");
	fprintf(stderr,"    -r    - number of image rows [default is 256]\n");
	fprintf(stderr,"    -c    - number of image columns [default is 256]\n");
	fprintf(stderr,"    -a    - modulation amplitude [default is 75.0]\n");
	fprintf(stderr,"    -p    - modulation period [default is 5]\n");
}

