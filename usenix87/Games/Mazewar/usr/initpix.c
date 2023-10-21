#ifndef lint
static char rcsid[] = "$Header: initpix.c,v 1.1 84/08/25 17:11:18 jcoker Exp $";
#endif

#include <stdio.h>
#include <pixrect/pixrect_hs.h>

char	PR_DEVICE[] = "/dev/fb";	/* console frame buffer */
char	FONT_FILE[] = "/usr/suntool/fixedwidthfonts/gacha.r.7";

struct pixrect	*px_screen;		/* full screen pixrect */
struct pixrect	*px_viewwin,		/* 3-D vview window */
		*px_statwin,		/* player stat window */
		*px_mapwin;		/* ovverhead map window */
struct pixfont	*pf_font;		/* game font */

#define VIEWW		752
#define VIEWH		568
#define STATW		200
#define STATH		752
#define MAPW		320
#define MAPH		160


/*
 *  Set up the sun screen graphics and
 *  the window pixrects.
 */

sun_init_screen()
{
	if (initpix())
		return(-1);

	px_viewwin = mem_create(VIEWW, VIEWH, 1);
	px_statwin = mem_create(STATW, STATH, 1);
	px_mapwin = mem_create(MAPW, MAPH, 1);
	if (!px_viewwin || !px_statwin || !px_mapwin) {
		fputs("Cannot allocate memory window pixrects.\n", stderr);
		return(-1);
	}

	/*
	 *  Clear all the pixrects.
	 */
	pr_rop(px_viewwin, 0, 0, px_viewwin->pr_width, px_viewwin->pr_height,
	    PIX_SET, (struct pixrect *)0, 0, 0);
	pr_rop(px_statwin, 0, 0, px_statwin->pr_width, px_statwin->pr_height,
	    PIX_SET, (struct pixrect *)0, 0, 0);
	pr_rop(px_mapwin, 0, 0, px_mapwin->pr_width, px_mapwin->pr_height,
	    PIX_SET, (struct pixrect *)0, 0, 0);

	/*
	 *  Open the game font file.
	 */
	pf_font = pf_open(FONT_FILE);
	if (pf_font == NULL) {
		fputs("Can't open font file ", stderr);
		perror(FONT_FILE);
		return(-1);
	}
	
	/*
	 *  Clear the sun screen.
	 */
	pr_rop(px_screen, 0, 0, px_screen->pr_width, px_screen->pr_height,
	    PIX_SET, (struct pixrect *)0, 0, 0);

	return(0);
}


/*
 *  Set up the screen for the subsequent screen 
 *  graphics mmanipulations.  All operations are 
 *  done for the pixrect facilities of the SUN
 *  workstation 1.1 release.
 */

initpix()
{
	extern char	*getenv();

	if (getenv("WINDOW_PARENT") != NULL) {
		fputs("Maze Wars: Can't run under suntools.\n", stderr);
		return(-1);
	}

	if ((px_screen = pr_open(PR_DEVICE)) == NULL) {
		fputs("Cannot open screen frame buffer ", stderr);
		perror(PR_DEVICE);
		return(-1);
	}

	return(0);
}


/*
 *  Close down the screen pixrect.
 */

endpix()
{
	pr_close(px_screen);

	return(0);
}
