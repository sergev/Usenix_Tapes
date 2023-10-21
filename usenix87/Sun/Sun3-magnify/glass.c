/*
 *  Magnifying glass		(for Sun-3 workstations)
 *
 *  Copyright 1986, 1987 by Scott Schwartz
 *  This program may be copied with the provision that this copyright 
 *  message remains, and that further distribution of this code and it's 
 *  derivatives is not subject to additional restrictions.
 * 
 *  compile with -lsuntool -lsunwindow -lpixrect
 *
 *  vi: set ts=8
 *
 *  revision history:
 *  10 Nov 86	initial coding		 		Scott Schwartz 
 *
 *  version 1.0		25 Nov 86
 *	modified to draw big pixels with raster-ops		ses
 *
 *  version 1.1		27 Jan 87
 *	does magnification in memory, 				ses
 *	to avoid screen access
 *
 *  version 1.2		27 Jan 87
 *	flood destination with white, 				ses/af
 *      then draw only black spots
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <pixrect/pixrect_hs.h>

/* useful macros */
#define EVER ;;
#define break_when(condition)	if (condition) break
#define error(msg)  { fprintf(stderr, "%s: %s\n", argv[0], msg); exit(1); }

/* forward declarations */
extern char *getenv(), *sprintf();
extern Notify_error notify_dispatch();
static Notify_value notice_destroy_event();
extern void pw_mag();
extern void view();

/* constants */
static char frame_label[] = "Magnifying Glass 1.2";
static short icon_image[] = {
#include "glass.icon"
};
DEFINE_ICON_FROM_IMAGE(icon, icon_image);
#define NILPR ((Pixrect *)0)
#define screen_max_x  1152
#define screen_max_y  900
#define max_mag 100

/* global vars */
static Pixrect	*tmpsrc; /* working space, to avoid screen access */ 
static Pixrect	*tmpdst;
static int done = 0;

/* 
 *  main routine
 */
main(argc,argv)
int argc;
char **argv;
{
	/* externals */
	extern Pixrect *tmpsrc, *tmpdst;

	/* things to magnify */
	char *parent; /* name of the parent window, usually win0 */
	Pixrect *srcpr; /* the source pixrect, i.e. the whole screen */
	int rootfd;  /* file descriptor for root window, for cursor location */

	/* things to draw on */
	Frame frame;	/* the actual window frame */
	Canvas canvas;	/* our particular canvas, filling the frame */

	/* misc */
	int mag = 1;			/* magnification in output window */
	char *magstr = "x99999";	/* allocate space for actual string */
	int retcode;

	/* dissociate from parent process */
	if (fork()) 
		exit(0);

	/* open rootwindow */
	parent = getenv("WINDOW_PARENT");
	if (parent==NULL) 
		error("can't get WINDOW_PARENT from environment.")
	rootfd = open(parent, O_RDONLY, 0);
	if (rootfd<0)
		perror(argv[0]); 

	/* open frame buffer */
	srcpr = pr_open("/dev/fb");
	if (srcpr==NILPR)
		perror(argv[0]);

	/* create output window */
	frame = window_create(NULL, FRAME, 
			FRAME_LABEL, frame_label,
			FRAME_ICON, &icon,
			WIN_HEIGHT, 200,
			WIN_WIDTH, 200,
			FRAME_ARGC_PTR_ARGV, &argc, argv,
			0);

	/* handle arguments */		
	if (argc > 1) {
		retcode = sscanf(argv[1], "%d", &mag);
		if (retcode <= 0) 
			error("problem evaluating arguments\nUsage: glass magnification [suntools-options]");
		mag = (mag > max_mag) ? max_mag : mag;
	}

	/* allocate static pixrects */
	tmpsrc = mem_create(screen_max_x, screen_max_y, 1);
	tmpdst = mem_create(screen_max_x, screen_max_y, 1);

	/* gather data on output pixwin */
	canvas = window_create(frame, CANVAS, 0);

	/* set up frame label, and activate window */
	(void)sprintf(magstr, "%s x%d", frame_label, mag);
	window_set(frame, FRAME_LABEL, magstr, WIN_SHOW, TRUE, 0);

	/* set up an interposed event handler so we know when to quit */
	(void)notify_interpose_destroy_func(frame, notice_destroy_event);
	
	/* copy input to output forever */
	for (EVER) {
		(void) notify_dispatch();
		break_when(done);
		if (window_get(frame, FRAME_CLOSED))
			sleep(1);
		else
			view(rootfd, srcpr, canvas, mag);
	}
}

/*
 * view does the work of displaying the (possibly) magnified image
 * at the location indicated by rootfd (mouse). view copies srcpr to dstpw.
 */
void view(rootfd, srcpr, canvas, mag)
	int rootfd;		/* root window, for mouse data */	
	Pixrect *srcpr;		/* screen source pixrect */
	Canvas canvas;
	int mag;
{
	/* constants */
	int maxy = (srcpr->pr_size.y);
	int maxx = (srcpr->pr_size.x);
	Pixwin *dstpw = canvas_pixwin(canvas);

	/* local vars */	
	int x,y;
	int w, h;

	/* 
	 * read mouse coords from vuid register.  sadly, the sunview
	 * programmers guide is not clear on this.  appendix A tells some,
	 * but you have to look at <sundev/vuid_event.h> to see what
 	 * virtual events you can find out about.  luckily, the mouse is
	 * one of them.
	 */
	x = win_get_vuid_value(rootfd, LOC_X_ABSOLUTE);
	y = win_get_vuid_value(rootfd, LOC_Y_ABSOLUTE);

	/* 
	 * find out how big our drawing surface is.
	 */
	w = (int) window_get(canvas, CANVAS_WIDTH);
	h = (int) window_get(canvas, CANVAS_HEIGHT);

	/*
	 * draw on it.
	 */
	if (mag<=1) {
		x = min(maxx-w, max(x-w, 0));
		y = min(maxy-h, max(y-h, 0));
		pw_rop(dstpw, 0, 0, w, h, PIX_SRC, srcpr, x, y);
	}
	else {
		x = min(maxx-w/mag, max(x-w/mag, 0));
		y = min(maxy-h/mag, max(y-h/mag, 0));
		pw_mag(dstpw, 0, 0, w, h, mag, srcpr, x, y);
	}
}

/*
 * pw_mag copies a magnified view of spr to dpw using pixel replication.
 * the arguments are the same as those to the pw_rop library call, except
 * that magnification is passed instead of raster-op.
 */
void pw_mag(dpw, dx, dy, w, h, mag, spr, sx, sy)
	Pixwin *dpw;	/* destination pixwin */
	int dx, dy;  	/* destination x,y */
	int w, h;	/* width and height of block to copy */
	int mag; 	/* magnification */
	Pixrect *spr;	/* source pixrect */
	int sx,sy;	/* location in source to copy from */
{
	/* locals */
	register short si, sj;
	register short di, dj;
	register short jmax = h/mag, imax = w/mag;

	Pixrect r;	/* holds the size of the drawing region when */
			/* gaining access to the screen */
	r.pr_size.x = w;
	r.pr_size.y = h;

	/* make off screen copy of source */
	pr_rop(tmpsrc, 0, 0, w, h, PIX_SRC|PIX_DONTCLIP, spr, sx, sy);

	/* 
	 * mangify....
	 * flood destination with white (why not?)
	 * for each black pixel in the source
	 *   draw a large square on the destination
	 */
	pr_rop(tmpdst, 0, 0, w, h, PIX_CLR|PIX_DONTCLIP, tmpdst, 0, 0);
	for (sj=0, dj=0; sj<=jmax; ++sj, dj+=mag) {
	   for (si=0, di=0; si<=imax; ++si, di+=mag) {
	     if (pr_get(tmpsrc, si, sj)!=0)
	     	pr_rop(tmpdst, di, dj, mag, mag, PIX_SET|PIX_DONTCLIP,
			tmpdst, 0, 0);
	   }
	}		

	/* draw */
	pw_lock(dpw, &r);
	pw_rop(dpw, dx, dy, w, h, PIX_SRC, tmpdst, 0, 0);
   	pw_unlock(dpw);
}

/* 
 * a service routine that gets called when the frame is destroyed
 * by someone selecting 'quit'.  this is basically right out of the 
 * manual.
 */
static Notify_value
notice_destroy_event(frame, status)
	Frame *frame;
	Destroy_status status;
{
	if (status != DESTROY_CHECKING) {
		done = 1;
		(void) notify_stop();
	}
	return (notify_next_destroy_func(frame,status));
}

