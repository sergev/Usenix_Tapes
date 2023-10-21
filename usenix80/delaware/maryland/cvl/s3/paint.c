#       /* painting program - paints a color image on the Grinnell from a
	   component labelled picture */
#define FOREVER for(;;)
#define MAX_X   512
#define MAX_Y   480
#define BLUE    07400
#define GREEN   00360
#define RED     00017
#define ALL     07777
#define HEADSZE 6
#define PAL_WID 35
#define PAL_HGH 61
#define ASYNCH  1
#define NZWRITE 1
#define SIGINT  2
#define MAX_ROW 50

/* picture file header format */
struct {
	int x0, x;
	int y0, y;
	int s0, size;
	} header;

/* buffered i/o routine structure */
struct {
	int fildes;
	int nunused;
	char *xfree;
	char buff[512];
	} iobuf;

struct {
	int x1, y1;
	int x2, y2;
	} curbuf;

int window[16], palette[16], rbuf[MAX_X], row, col, byte, rows_left, rows_seeked;
int colored, row_colored;
/* default initial color mixture */
int blue 7, green 7, red 7, color;
char component; /* id # of the component selected by user */

/****************************************************************************/

main(argc, argv)
int argc;
char *argv[];
{
int *rm_bord();

if (argc < 2) {
	printf("Useage: %s <segmented file>\n", argv[0]);
	exit();
	}

if (fopen(argv[1], &iobuf) < 0) {
	printf("Can't open '%s'", argv[1]);
	perror(" ");
	exit(1);
	}

header.x0   = getw(&iobuf);
header.x    = getw(&iobuf);
header.y0   = getw(&iobuf);
header.y    = getw(&iobuf);
header.s0   = getw(&iobuf);
header.size = getw(&iobuf);

printf("Picture size: %d X %d\n", header.x, header.y);

if (header.size != 4) {
	printf("'%s' is not an 8-bit picture.\n", argv[1]);
	exit(1);
	}

if (gopen(window, 0, 0, header.x, header.y) < 0) {
	printf("Can't open window on grinnell.\n");
	exit(1);
	}

/* display the picture on the grinnell in outline form */
for (row = (header.y - 1); row >= 0; row--) {
	for (col = 0; col < header.x; col++) {
			if ((byte = getc(&iobuf)) < 0) {
				printf("Premature eof.\n");
				exit(1);
				}
		rbuf[col] = byte? 0: ALL;
		}
	if (gwrow(window, rbuf, row, 0, header.x, 1) < 0) {
		printf("Can't write to grinnell.\n");
		exit(1);
		}
	}

/* display the palette in the upper right hand corner */
if (gopen(palette, MAX_X-PAL_WID, MAX_Y-PAL_HGH, PAL_WID, PAL_HGH)) {
	printf("Can't open window for palette\n");
	exit(1);
	}
gclear(palette, 0, 0, PAL_WID, PAL_HGH, ALL);
gwvec(palette, 0, 0, 0, PAL_HGH-1, 0);
gwvec(palette, 0, PAL_HGH-1, PAL_WID-1, PAL_HGH-1, 0);
gwvec(palette, PAL_WID-1, PAL_HGH-1, PAL_WID-1, 0, 0);
gwvec(palette, PAL_WID-1, 0, 0, 0, 0);
gwvec(palette, 0, 35, PAL_WID-1, 35, 0);

dis_palette();

gwcur(palette, 2, 0, 0, 0, 0);  /* hide cursor #2 */
gwcur(palette, 1, PAL_WID/2, PAL_HGH/2, 1, 1);  /* put cursor #1 in center of palette */

if (signal(SIGINT, rm_bord) < 0)
	 perror("SIGNAL error");
printf("Hit rubout to exit\n");

/* and now for the main drudge work */
FOREVER {

	grcur(window, &curbuf, ASYNCH);
	if ((curbuf.x1 >= MAX_X-PAL_WID) && (curbuf.y1 >= MAX_Y-PAL_HGH)) {
		/* palette command */
		if (chg_color(curbuf.x1-(MAX_X-PAL_WID), curbuf.y1-(MAX_Y-PAL_HGH)))
			dis_palette();
		}
	else if ((curbuf.x1 < header.x) && (curbuf.y1 < header.y)) {
		/* coloring command */

		putchar(07);

		if (seek(iobuf.fildes, sizeof(header) + curbuf.x1, 0) < 0)
			perror("\07\07Seek error");
		for (rows_left = header.y - curbuf.y1; rows_left > 0; rows_left =- rows_seeked) {
			rows_seeked = rows_left < MAX_ROW? rows_left: MAX_ROW;
			if (seek(iobuf.fildes, rows_seeked*header.x, 1) < 0)
				perror("\07\07Seek error");
			}

		if(read(iobuf.fildes, &component, 1) < 1) {
			perror("Can't read in component id");
			exit(1);
			}
		component =& 0377;

		if (component == 0) {
			printf("\007Sorry, but you can't color a border!\n");
			continue;
			}

		printf("Coloring component # %d with b=%d, g=%d, r=%d\n", component, blue, green, red);

		color = (blue << 8) | (green << 4) | (red);

		seek(iobuf.fildes, sizeof(header), 0); /* rewind file */
		iobuf.nunused = 0;
		iobuf.xfree = iobuf.buff;

		/* write the current color into the component the user specified */
		colored = 0;
		for (row = (header.y - 1); row >= 0; row--) {
			row_colored = 0;
			/* read in the row */
			grrow(window, rbuf, row, 0, header.x, 1);
			/* mess with it */
			for (col = 0; col < header.x; col++) {
				if ((byte = getc(&iobuf)) < 0) {
					printf("Premature eof.\n");
					exit(1);
					}
				if (byte == component) {
					colored++;
					row_colored++;
					rbuf[col] = color;
					}
				}

			if (colored && !row_colored)    /* all done: don't need to read in */
				break;                  /* the rest of the picture */

			/* and write it back out */
			if (gwrow(window, rbuf, row, 0, header.x, 1) < 0) {
				printf("Can't write to grinnell.\n");
				exit(1);
				}
			}
		putchar(07);
		}
	}
}

/****************************************************************************/

dis_palette() { /* fills in the bars in the palette display according to
		   global integers: blue, green & red */

/* first the color sample splotch */
genter(palette, 0, ((blue << 8) | (green << 4) | (red)), 0, 0, 0);
gwvec(palette, 1, 36, PAL_WID-2, PAL_HGH-2, 1);

/* and then the color bars */

gclear(palette,  5, 2, 5, 32, ALL);
if (blue) {
	genter(palette, 0, BLUE, 0, 0, 0);
	gwvec(palette,  5, 2,  9, 2 + 2*blue, 1);
	}

gclear(palette, 15, 2, 5, 32, ALL);
if (green) {
	genter(palette, 0, GREEN, 0, 0, 0);
	gwvec(palette, 15, 2, 19, 2 + 2*green, 1);
	}

gclear(palette, 25, 2, 5, 32, ALL);
if (red) {
	genter(palette, 0, RED, 0, 0, 0);
	gwvec(palette, 25, 2, 29, 2 + 2*red, 1);
	}

genter(palette, 0, ALL, 0, 0, 0);
}

/****************************************************************************/

chg_color(ax, ay)       /* change the current color mixture according to */
int ax, ay;             /* the cursor position within the palette window */
{
register int x, y;

x = ax;
y = ay;

if ((x >=  5) && (x <=  9) && (y <= 33))
	blue = y >= 2? (y - 2)/2: 0;
else if ((x >= 15) && (x <= 19) && (y <= 33))
	green = y >= 2? (y - 2)/2: 0;
else if ((x >= 25) && (x <= 29) && (y <= 33))
	red = y >= 2? (y - 2)/2: 0;
else
	return(0);

return(1);
}

/****************************************************************************/

rm_bord() {     /* this routine cleans up the picture and exits */
struct {
	int  l_marg;
	int  p_buf[MAX_X];
	int  r_marg;
	char c_buf[MAX_X];
	} buffers[3], *p1, *p2, *p3;
int n_color;

gclear(palette, 0, 0, PAL_WID, PAL_HGH, ALL);   /* erase palette */
gwcur(palette, 1, 0, 0, 0, 0);                  /* hide cursor #1 */

/* rewind the file */
seek(iobuf.fildes, sizeof(header), 0);
iobuf.nunused = 0;
iobuf.xfree = iobuf.buff;

p1 = &buffers[0];
p2 = &buffers[1];
p3 = &buffers[2];

/* set left & right margin pixels as border points */
p1->l_marg = p1->r_marg = p2->l_marg = p2->r_marg = p3->l_marg = p3->r_marg = 0;

/* set up the top margin as a border & read in the first row of the picture */
grrow(window, p2->p_buf, header.y - 1, 0, header.x, 1); /* picture from grinnell */
for (col = 0; col < header.x; col++)                    /* border := 0 */
	p1->c_buf[col] = 0;
for (col = 0; col < header.x; col++)                    /* labelled image */
	p2->c_buf[col] = getc(&iobuf);

/* and now zap each row */
for (row = (header.y - 1); row >= 0; row--) {

	if (row > 0) {  /* then read in the following row */
		grrow(window, p3->p_buf, row-1, 0, header.x, 1);
		for (col = 0; col < header.x; col++)
			p3->c_buf[col] = getc(&iobuf);
		}
	else            /* assume the next row is a solid border */
		for (col = 0; col < header.x; col++)
			p3->c_buf[col] = 0;

	for (col = 0; col < header.x; col++)
		if (p2->c_buf[col] == 0) {      /* if this is a border point */
			n_color = blue = green = red = 0;

			/* examine all 8 neighbors */
			if (p1->c_buf[col-1] > 0) {
				n_color++;
				color = p1->p_buf[col-1];
				blue  =+ (color &  BLUE) >> 8;
				green =+ (color & GREEN) >> 4;
				red   =+ (color &   RED);
				}

			if (p1->c_buf[col] > 0) {
				n_color++;
				color = p1->p_buf[col];
				blue  =+ (color &  BLUE) >> 8;
				green =+ (color & GREEN) >> 4;
				red   =+ (color &   RED);
				}

			if (p1->c_buf[col+1] > 0) {
				n_color++;
				color = p1->p_buf[col+1];
				blue  =+ (color &  BLUE) >> 8;
				green =+ (color & GREEN) >> 4;
				red   =+ (color &   RED);
				}

			if (p2->c_buf[col-1] > 0) {
				n_color++;
				color = p2->p_buf[col-1];
				blue  =+ (color &  BLUE) >> 8;
				green =+ (color & GREEN) >> 4;
				red   =+ (color &   RED);
				}

			if (p2->c_buf[col+1] > 0) {
				n_color++;
				color = p2->p_buf[col+1];
				blue  =+ (color &  BLUE) >> 8;
				green =+ (color & GREEN) >> 4;
				red   =+ (color &   RED);
				}

			if (p3->c_buf[col-1] > 0) {
				n_color++;
				color = p3->p_buf[col-1];
				blue  =+ (color &  BLUE) >> 8;
				green =+ (color & GREEN) >> 4;
				red   =+ (color &   RED);
				}

			if (p3->c_buf[col] > 0) {
				n_color++;
				color = p3->p_buf[col];
				blue  =+ (color &  BLUE) >> 8;
				green =+ (color & GREEN) >> 4;
				red   =+ (color &   RED);
				}

			if (p3->c_buf[col+1] > 0) {
				n_color++;
				color = p3->p_buf[col+1];
				blue  =+ (color &  BLUE) >> 8;
				green =+ (color & GREEN) >> 4;
				red   =+ (color &   RED);
				}

			/* average the colors of the neighbors to produce the color for this point */
			p2->p_buf[col] = ((blue / n_color) << 8) | ((green / n_color) << 4) | (red / n_color);
			}

	gwrow(window, p2->p_buf, row, 0, header.x, 1);  /* write out the modified row */
	shift(&p1, &p2, &p3);   /* shift the buffers within memory */
	}

printf("\07Done\n");
exit(0);
}

/****************************************************************************/

shift(a, b, c)  /* does a left circular shift of its arguments */
int *a, *b, *c;
{
int d;

 d = *a;
*a = *b;
*b = *c;
*c =  d;

}

/****************************************************************************/
