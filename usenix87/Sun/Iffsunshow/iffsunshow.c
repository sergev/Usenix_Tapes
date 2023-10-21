
iffsunshow.c - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define BG	/* run tool in background; foreground process (parent) exits */
#define FRACTIONS	/* do arithmetic with scaled integers */

/*
 * program for popping up a window to display an iff file
 *
 * Liudvikas Bukys -- 02-Sep-86
 */

#include <stdio.h>
#include <suntool/tool_hs.h>
#include <suntool/panel.h>
#include <suntool/gfxsw.h>

#include <image.h>

#include <ctype.h>
extern char *malloc();
extern struct pixrect *pr_load();

struct tool	*tool;
struct toolsw	*gfx_sw;
struct gfxsubwindow *gfx;
int sigwinched(), gfx_sigwinch();

int sigtermed();
int InputSelected();

unsigned short	icon_image[256] = {
#include "iff.icon"
};
DEFINE_ICON_FROM_IMAGE(icon, icon_image);

struct inputmask acceptmask = { IM_ASCII, };
struct inputmask flushmask = { 0, };

IMAGE *in;
int image_done = FALSE;
int image_max, image_min, image_threshold;
int image_max_specified = FALSE, image_min_specified = FALSE;
char *image_name = "";
struct pixrect *image_pixrect;
short *scandata;

long *curimrow, *nxtimrow;
int iy;

Input1()
{
	register int ix;
	static char annote[IMHDRSZE];

	in = idopen(fileno(stdin), R);
	if (in == NULL) {
		fprintf(stderr,"Unable to read image\n");
		exit(1);
	}

	/* get the image header into memory */
	if (getheader(in, annote) == -1) exit(1);

	/* scale dither mask appropriately */
	if (!image_min_specified && !get_field("min", annote, &image_min))
		image_min = 0;
	if (!image_max_specified && !get_field("max", annote, &image_max))
		image_max = (1<<in->bpp)-1;

	curimrow = (long *) malloc (in->ncols * sizeof (long));
	nxtimrow = (long *) malloc (in->ncols * sizeof (long));

	mygetrow(in, iy=0, nxtimrow);	/* prime the pump */

#ifdef FRACTIONS
	for (ix= 0;  ix < in->ncols;  ix++)
		nxtimrow[ix] <<= 8;
	image_max <<= 8;
	image_min <<= 8;
#endif
	if (image_max_specified || image_min_specified)
	    for (ix = 0; ix < in->ncols; ix++) {
		if (nxtimrow[ix] > image_max) nxtimrow[ix] = image_max;
		if (nxtimrow[ix] < image_min) nxtimrow[ix] = image_min;
	    }

	image_threshold = (image_max+image_min) >> 1;
	image_pixrect = mem_create(in->ncols, in->nrows, 1);
	pr_rop(image_pixrect, 0, 0, in->ncols, in->nrows, 
	    (PIX_NOT(PIX_DST) & PIX_DST), NULL, 0, 0);
	scandata = &mpr_d(image_pixrect)->md_image[(in->nrows-1)*(mpr_d(image_pixrect)->md_linebytes>>1)];
}

#define X1(x) (x)
#define X3(x) ((x)+(x)+(x))
#define X5(x) (((x)<<2)+(x))
#define X7(x) (((x)<<3)-(x))

short bitmask[] = {
        0x8000,0x4000,0x2000,0x1000,0x0800,0x0400,0x0200,0x0100,
        0x0080,0x0040,0x0020,0x0010,0x0008,0x0004,0x0002,0x0001
};

Input2()
{
	register int ix;
	register int err;
	register int pixel;
	long *ptr;

	/*
	 * read current row (already read)
	 */
	ptr = curimrow;
	curimrow = nxtimrow;
	nxtimrow = ptr;

	/*
	 * read next row
	 */
	if (iy < in->nrows-1) {
		mygetrow(in,iy+1,nxtimrow);
#ifdef FRACTIONS
		for (ix= 0;  ix < in->ncols;  ix++)
			nxtimrow[ix] <<= 8;
#endif
	} else {
		iclose(in);
		image_done = TRUE;
	}

	if (image_max_specified || image_min_specified)
	    for (ix = 0; ix < in->ncols; ix++) {
		if (nxtimrow[ix] > image_max) nxtimrow[ix] = image_max;
		if (nxtimrow[ix] < image_min) nxtimrow[ix] = image_min;
	    }

	/*
	 * compute
	 */
	for (ix = 0; ix < in->ncols; ix++) {
		pixel = curimrow[ix];
		/* this is still a bit wrong */
		if (pixel <= image_threshold)
			err = pixel - image_min;
		else {
			err = pixel - image_max;
			scandata[ix >> 4] |= bitmask[ix & 0xf];
		}

		nxtimrow[ix] += (X5(err) + 8) >> 4;
		if (ix < in->ncols-1) {
			curimrow[ix+1] += (X7(err) + 8) >> 4;
			nxtimrow[ix+1] += (X1(err) + 8) >> 4;
		}
		if (ix > 0)
			nxtimrow[ix-1] += (X3(err) + 8) >> 4;
	}

	/* advance scandata to start of next row */
	scandata -= mpr_d(image_pixrect)->md_linebytes >> 1;

	pw_write(gfx->gfx_pixwin, 0, in->nrows-iy-1, image_pixrect->pr_width,
		1, PIX_SRC, image_pixrect, 0, in->nrows-iy-1);

	iy++;
}

InputSelected(nil, ibits)
int *ibits;
{
	struct inputevent ie;
	int c;
	char buf[10];
	char *oldlabel, *newlabel;

	if (((*ibits)&1) != 0) {
		Input2();
		if (image_done) {
			*ibits &= ~1;
#ifdef BG
			/* if I've gotten this far, everything's OK,
			 * so tell parent to exit
			 */
			kill(getppid(), SIGTERM);
#endif
		}
	}

	if (((*ibits)&(1<<gfx->gfx_windowfd)) == 0)
		return;

	if (input_readevent(gfx->gfx_windowfd, &ie) != 0) {
		perror("input_readevent");
		return;
	}

	c= ie.ie_code;
	if (ASCII_FIRST <= c && c <= ASCII_LAST) {
		oldlabel = (char *) tool_get_attribute(tool, WIN_LABEL);

		switch (c) {
		 case 'h'&037:
		 case 0177:
			if (oldlabel[0] != '\0') {
				oldlabel[strlen(oldlabel)-1] = '\0';
				tool_set_attributes(tool, WIN_LABEL, oldlabel, 0);
				tool_set_attributes(tool, WIN_ICON_LABEL, oldlabel, 0);
			}
			break;

		 case '\n':
		 case '\r':
			tool_set_attributes(tool, WIN_LABEL, "", 0);
			tool_set_attributes(tool, WIN_ICON_LABEL, "", 0);
			break;

		 default:
			if (isascii(c))
				if (isprint(c))
					sprintf(buf, "%c", c);
				else if (iscntrl(c))
					sprintf(buf, "^%c", 0100^c);
				else
					sprintf(buf, "\\%03o", c);
			else
				sprintf(buf, "\\%03o", c);
			newlabel = malloc(strlen(oldlabel)+strlen(buf)+1);
			sprintf(newlabel, "%s%s", oldlabel, buf);
			tool_set_attributes(tool, WIN_LABEL, newlabel, 0);
			tool_set_attributes(tool, WIN_ICON_LABEL, newlabel, 0);
			free(newlabel);
			break;
		}

		tool_free_attribute(WIN_LABEL, oldlabel);
	}

	gfxsw_inputinterrupts(gfx, &ie);

	return;
}

Update()
{
	pw_write(gfx->gfx_pixwin, 0, 0, image_pixrect->pr_width,
		image_pixrect->pr_height, PIX_SRC, image_pixrect, 0, 0);
	pw_writebackground(gfx->gfx_pixwin,
		0, image_pixrect->pr_height,
		gfx->gfx_rect.r_width, gfx->gfx_rect.r_height - image_pixrect->pr_height,
		PIX_SRC);
	pw_writebackground(gfx->gfx_pixwin,
		image_pixrect->pr_width, 0,
		gfx->gfx_rect.r_width - image_pixrect->pr_width, gfx->gfx_rect.r_height,
		PIX_SRC);
}

/***/

main(argc, argv)
	int argc;
	char **argv;
{
	char **tool_attrs = NULL;
	char *tool_name = argv[0];
	int pid;
	int argi;

	/* interpret tool arguments */
	argv++, argc--;
	if (tool_parse_all(&argc, argv, &tool_attrs, tool_name) == -1) {
		tool_usage(tool_name);
		exit(1);
	}

	/* interpret iffsunshow arguments */
	for (argi= 0;  argi < argc;  argi++)
		if (argv[argi][0] == '-')
			switch (argv[argi][1]) {
			  case 'l':
				image_min = atoi(&argv[argi][2]);
				image_min_specified = TRUE;
				break;
			  case 'h':
				image_max = atoi(&argv[argi][2]);
				image_max_specified = TRUE;
				break;
			  default:
				fprintf(stderr, "warning: unknown switch '%s'\n", argv[argi]);
				break;
			}
		else {
			image_name = argv[argi];
			if (freopen(image_name, "r", stdin) == NULL) {
				fprintf(stderr, "couldn't open '%s'\n", image_name);
				exit(1);
			}
		}

	/* read image size, initialize display pixrect, get 1st row */
	Input1();

#ifdef BG
	signal(SIGTERM, sigtermed);
	if (pid= fork())
		if (pid < 0) {
			perror("fork");
			exit(1);
		} else {
			/* parent waits... */
			wait(0);
			/* normally child will kill -TERM parent */
			fprintf(stderr, "failed\n");
			exit(1);
		}
	/* child continues... */
	close(1);
	close(2);
	signal(SIGTSTP, SIG_IGN);
#endif

	tool = tool_make(
		WIN_ICON,	&icon,
		WIN_LABEL,	image_name,
		WIN_ATTR_LIST,	tool_attrs,
		0);
	if (tool == (struct tool *) NULL) {
	        fputs("Can't make tool\n", stderr);
		exit(1);
	}
	tool_free_attribute_list(tool_attrs);

	/* graphics subwindow */
	if ((gfx_sw = gfxsw_createtoolsubwindow(tool, "", 
	    TOOL_SWEXTENDTOEDGE, TOOL_SWEXTENDTOEDGE, NULL)) == NULL) {
		fputs("Can't create graphics subwindow\n", stderr);
		exit(1);
	}
	gfx = (struct gfxsubwindow *) gfx_sw->ts_data;
	pw_whiteonblack(gfx->gfx_pixwin, 0, 1);
	/* gfxsw_getretained(gfx); */
	gfx_sw->ts_io.tio_handlesigwinch = gfx_sigwinch;
	gfx_sw->ts_io.tio_selected = InputSelected;
	gfx_sw->ts_io.tio_inputmask |= (1<<gfx->gfx_windowfd) | (1);
	win_setinputmask(gfx->gfx_windowfd, &acceptmask, &flushmask, WIN_NULLLINK);

	tool_set_attributes(tool, 
		WIN_WIDTH,	image_pixrect->pr_width + 2*tool_borderwidth(tool),
		WIN_HEIGHT,	image_pixrect->pr_height
					+ tool_stripeheight(tool)
					+ 2 /* a 2-line bonus from somewhere */
					+ tool_borderwidth(tool),
		0);

	signal(SIGWINCH, sigwinched);
	tool_install(tool);	/* install tool in window tree */
	tool_select(tool, 0);	/* main loop to read input */
	tool_destroy(tool);	/* clean up tool */
}

sigtermed()
{
	/* child process kills parent if everything goes OK */
	exit(0);
}

sigwinched()	/* note window size change and damage repair signal */
{
	tool_sigwinch(tool);
}

gfx_sigwinch(sw)
	caddr_t sw;
{
	/* Let graphics subwindow notice that sigwinched */
	gfxsw_interpretesigwinch(gfx);
	/* Let graphics subwindow update retained pixwin */
	gfxsw_handlesigwinch(gfx);
	/* See if need to redraw the window due to size change */
	if (gfx->gfx_flags & GFX_RESTART) {
		gfx->gfx_flags &= ~GFX_RESTART;
		Update();
	}
}

mygetrow(ifp, row, buf)
    IMAGE *ifp;
    int row;
    long *buf;
{
    register long *bufptr = buf;
    register int i;

    for (i=ifp->ncols; i--; ) *bufptr++ = getpix(ifp);
}

iff.icon  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Format_version=1, Width=64, Height=64, Depth=1, Valid_bits_per_item=16
 */
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x8000,0x0000,0x0000,0x0001,
	0xBFFF,0xFFFF,0xFFFF,0xFFFD,0xA000,0x0000,0x0000,0x0005,
	0xA000,0x0001,0xC000,0x0005,0xA000,0x0001,0xC000,0x0005,
	0xA000,0x0002,0xA000,0x0005,0xA000,0x0002,0xA000,0x0005,
	0xA000,0x0002,0xA000,0x0005,0xA000,0x0002,0xA000,0x0005,
	0xA003,0xCCCC,0xDCCC,0xF005,0xA002,0x3333,0xB333,0x1005,
	0xA002,0xFFFF,0xFFFF,0xD005,0xA002,0x8000,0x0000,0x5005,
	0xA001,0x8000,0x0000,0x6005,0xA001,0x8000,0x0000,0x6005,
	0xA002,0x81F3,0xE7C0,0x5005,0xA002,0x8042,0x0400,0x5005,
	0xA001,0x8042,0x0400,0x6005,0xA001,0x8042,0x0400,0x6005,
	0xA002,0x8043,0xC780,0x5005,0xA002,0x8042,0x0400,0x5005,
	0xA001,0x8042,0x0400,0x6005,0xA001,0x8042,0x0400,0x6005,
	0xA002,0x81F2,0x0400,0x5005,0xA002,0x8000,0x0000,0x5005,
	0xA001,0x8000,0x0000,0x6005,0xA001,0x8000,0x0000,0x6005,
	0xA002,0x8000,0x0000,0x5005,0xA002,0xFFFF,0xFFFF,0xD005,
	0xA002,0x3333,0xB373,0x1005,0xA003,0xCDCC,0xCCCC,0xF005,
	0xA000,0x0100,0x8040,0x0005,0xA000,0x0200,0x8020,0x0005,
	0xA000,0x0200,0x8020,0x0005,0xA000,0x0200,0x8020,0x0005,
	0xA000,0x0200,0x8010,0x0005,0xA000,0x0400,0x8010,0x0005,
	0xA000,0x0400,0x8010,0x0005,0xA000,0x0400,0x8008,0x0005,
	0xA000,0x0800,0x8008,0x0005,0xA000,0x0800,0x8008,0x0005,
	0xA000,0x0800,0x8004,0x0005,0xA000,0x0800,0x8004,0x0005,
	0xA000,0x1000,0x8004,0x0005,0xA000,0x1000,0x8002,0x0005,
	0xA000,0x1000,0x8002,0x0005,0xA000,0x2000,0x8002,0x0005,
	0xA000,0x2000,0x8001,0x0005,0xA000,0x2000,0x8001,0x0005,
	0xA000,0x4000,0x8001,0x0005,0xA000,0x4000,0x8000,0x8005,
	0xA000,0x4000,0x0000,0x8005,0xA000,0x4000,0x0000,0x8005,
	0xA000,0x8000,0x0000,0x4005,0xA000,0x8000,0x0000,0x4005,
	0xA000,0x0000,0x0000,0x0005,0xA000,0x0000,0x0000,0x0005,
	0xA000,0x0000,0x0000,0x0005,0xA000,0x0000,0x0000,0x0005,
	0xA000,0x0000,0x0000,0x0005,0xBFFF,0xFFFF,0xFFFF,0xFFFD,
	0x8000,0x0000,0x0000,0x0001,0xFFFF,0xFFFF,0xFFFF,0xFFFF

