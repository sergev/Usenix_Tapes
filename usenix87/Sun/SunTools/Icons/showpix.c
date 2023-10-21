#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include "bitmap.h"

/*
 * Typedef'ing things...
 */
typedef struct pixrect	Pixrect;

/*
 * Unique menu identifiers 
 */
#define FILEN			((caddr_t) 1)
#define MORE			((caddr_t) 2)
#define QUIT			((caddr_t) 3)

#define MAX_BITMAPS		256
#define NULL_PR			((Pixrect *) NULL)
#define canvas_width(canvas)	(int) window_get(canvas, CANVAS_WIDTH)
#define canvas_height(canvas)	(int) window_get(canvas, CANVAS_HEIGHT)

#define NUM_ROW			5
#define NUM_COLUMN		5
#define ROW_GAP			10
#define ROW_HEIGHT		64
#define COLUMN_GAP		10
#define COLUMN_WIDTH		64
#define DISPLAY_MARGIN		10
#define DISPLAY_WIDTH		((COLUMN_WIDTH + COLUMN_GAP) * NUM_COLUMN)
#define DISPLAY_HEIGHT		((ROW_HEIGHT + ROW_GAP) * NUM_ROW)
#define WINDOW_MARGIN		10
#define WINDOW_WIDTH		(DISPLAY_WIDTH + 2 * DISPLAY_MARGIN + \
					2 * WINDOW_MARGIN)
#define WINDOW_HEIGHT		(DISPLAY_HEIGHT + 2 * DISPLAY_MARGIN + \
					2 * WINDOW_MARGIN)

/*
 * The tool's icon
 */
static short	icon_data[] = {
#include "showpix.icon"
};
mpr_static(icon_pixrect, 64, 64, 1, icon_data);

/*
 * The data for grey background
 */
static short	grey_data[] = {
        0xAAAA,0x5555,0xAAAA,0x5555,
	0xAAAA,0x5555,0xAAAA,0x5555,
	0xAAAA,0x5555,0xAAAA,0x5555,
	0xAAAA,0x5555,0xAAAA,0x5555
};

/*
 * The data for white background
 */
static short	white_data[] = {
			0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000
};

/*
 * Local variables
 */
Bitmap		*bitmaps[MAX_BITMAPS];
static int	nbitmap;
static Menu	menu;
static int	display_width = DISPLAY_WIDTH;
static int	display_height = DISPLAY_HEIGHT;
static int	canvas_resized = FALSE;
static int	ending_bitmap = 0;
static Pixrect	*bkg_pr;

static void
usage()
{
	printf("usage: showpix [ -grey ] bitmap1 [ bitmap2 ... ]\n");
	exit(1);
} /* end usage() */

static void
punt(str)
char	*str;
{
	fprintf(stderr, "%s\n", str);
	exit(1);
} /* end punt() */

/*
 * Load all bitmaps specified on command line.
 */
static void
load_bitmaps(argc, argv)
int	argc;
char	**argv;
{
	register int	i;
	struct stat	statbuf;
	Bitmap		*tmp_bitmap;

	/* process each argument */
	for (i = 0, nbitmap = 0; i < argc; i++) {
		/* check that this is a regular file */
		stat(argv[i], &statbuf);
		if (statbuf.st_mode & S_IFMT != TRUE) {
			fprintf(stderr, "File %s not a plain file.\n", argv[i]);
			continue;
		} 
		/* load the bitmap */
		if ((tmp_bitmap = bm_load(argv[i])) == NULL_BM) {
			fprintf(stderr, "Error loading %s\n", argv[i]);
			continue;
		} else {
			bitmaps[nbitmap++] = tmp_bitmap;
		}
	} /* end for */
} /* end load_bitmaps() */

/* 
 * Print the bitmap pixrects into the canvas pixwin.
 */
static void
paint_bitmaps(canvas)
Canvas	canvas;
{
	register int	i, dst_x, dst_y, off_x;
	Pixwin		*canvas_pw;
	Pixrect		*canvas_mpr;

	/* initalialize canvas pixwin */
	canvas_pw = canvas_pixwin(canvas);
	/* get the memory pixrect */
	canvas_mpr = canvas_pw->pw_prretained;
	/* replicate background source pixrect over background pixwin */
	pr_replrop(canvas_mpr, 0, 0, display_width, display_height, PIX_SRC, 
		bkg_pr, 0, 0);
	/* initialize vertical offset into memory pixrect */
	dst_y = ROW_GAP;
	/* copy bitmaps into memory pixrect */
	for (i = ending_bitmap; i < nbitmap; ) {
		if (dst_y + bitmaps[i]->height > display_height) break;
		/* initialize horizontal offset into memory pixrect */
		dst_x = COLUMN_GAP;
		off_x = 0;
		while (i < nbitmap) {
		    	if (dst_x + bitmaps[i]->width > display_width) break;
			pr_rop(canvas_mpr, dst_x, dst_y, bitmaps[i]->width, 
				bitmaps[i]->height, PIX_SRC, 
				bitmaps[i]->bitmap_pr, 0, 0);
			dst_x += (bitmaps[i]->width + COLUMN_GAP);
			if (bitmaps[i]->height > off_x) {
				off_x = bitmaps[i]->height;
			}
			i++;
		} /* end while */
		dst_y += (off_x + ROW_GAP);
	} /* end while */
	/* refresh the screen image */
	pw_write(canvas_pw, 0, 0, display_width, display_height, PIX_SRC,
		canvas_mpr, 0, 0);
	/* if we have hit the end, cycle around */
	ending_bitmap = (i < nbitmap) ? i : 0;
} /* end paint_bitmaps() */

static void
canvas_resize_proc(canvas, width, height)
Canvas	canvas;
int	width;
int	height;
{
	/* reset height && width */
	display_height = canvas_height(canvas);
	display_width = canvas_width(canvas);
	/* cycle to beginning */
	ending_bitmap = 0;
	/* repaint the bitmaps */
	paint_bitmaps(canvas);
} /* end canvas_resize_proc() */

static void
canvas_event_proc(canvas, eventp, arg)
Canvas	canvas;
Event	*eventp;
caddr_t	arg;
{
	caddr_t	mi;	/* a menu item */

	/* handle the mouse button events */
	switch(event_id(eventp)) {
		case MS_LEFT:		/* handle left button events */
			break;
		case MS_MIDDLE:		/* handle middle button events */
			break;
		case MS_RIGHT:		/* handle right button events */
			mi = menu_show(menu, canvas, eventp, 0);
			if (mi == NULL) break;
			switch (mi) {
				case FILEN:
					/* get new file name(s) */
					break;
				case MORE:
					/* show next page */
					paint_bitmaps(canvas);
					break;
				case QUIT:
					/* destroy all windows && exit */
					window_done(canvas);
					break;
			} /* end switch */
			break;
	} /* end switch */
} /* end canvas_event_proc() */

main(argc, argv)
int	argc;
char	**argv;
{
	Canvas		canvas;
	Frame		base_frame;

	/* check for usage */
	if (argc < 2) usage();
	/* skip over the program name */
	argc--;
	argv++;
	/* check to see what kind of background they want */
	if (strcmp(*argv, "-grey") == 0) {
		bkg_pr = mem_point(16, 16, 1, grey_data); 
		/* skip over the background option */
		argv++;
		if (argc-- < 1) usage();
	} else {
		bkg_pr = mem_point(16, 16, 1, white_data);
	}
	/* create the root window */
	if ((base_frame = window_create(NULL, FRAME, 
	    WIN_WIDTH,			WINDOW_WIDTH,
	    WIN_HEIGHT,			WINDOW_HEIGHT,
	    FRAME_SHOW_LABEL,		FALSE,
	    FRAME_ICON,			icon_create(ICON_IMAGE, &icon_pixrect),
	    0)) == NULL) {
		punt("Error creating base frame.");
	}
	/* create its child */
	if ((canvas = window_create(base_frame, CANVAS, 
	    CANVAS_WIDTH,		DISPLAY_WIDTH,
	    CANVAS_HEIGHT,		DISPLAY_HEIGHT,
	    CANVAS_MARGIN,		DISPLAY_MARGIN,
	    CANVAS_AUTO_EXPAND,		TRUE,
	    CANVAS_AUTO_SHRINK,		TRUE,
	    CANVAS_FIXED_IMAGE,		FALSE,
	    CANVAS_RETAINED,		TRUE,
	    CANVAS_RESIZE_PROC,		canvas_resize_proc,
	    WIN_EVENT_PROC,		canvas_event_proc,
	    0)) == NULL) {
		punt("Error creating canvas.");
	}
	/* create a menu w/ the three menu items (FILEN, MORE && QUIT) */
	if ((menu = menu_create(
	    MENU_ITEM, MENU_STRING_ITEM, 	"File ...", FILEN, 0,
	    MENU_ITEM, MENU_STRING_ITEM, 	"More", MORE, 0,
	    MENU_ITEM, MENU_STRING_ITEM, 	"Quit", QUIT, 0,
	    MENU_INITIAL_SELECTION_SELECTED,    TRUE,
	    MENU_INITIAL_SELECTION,             MENU_SELECTED,
	    0)) == NULL) {
		punt("Error creating menu.");
	}
	/* load the bitmaps */
	load_bitmaps(argc, argv);
	/* start the program */
	window_main_loop(base_frame);
} /* end main() */
