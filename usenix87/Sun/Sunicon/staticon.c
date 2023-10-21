#ifndef lint
static char rcsid[]
	= "$Header: staticon,v 1.10 86/10/13 15:42:52 gworek Locked $";
#endif

/*
 * Your favorite icon, when open or closed.
 *
 * Compile: cc -O -s -o staticon staticon.c -lsuntool -lsunwindow -lpixrect
 */

#include <stdio.h>
#include <suntool/tool_hs.h>
#include <suntool/gfxsw.h>
#include <suntool/msgsw.h>
#include <suntool/icon_load.h>

char *progname;				/* Who am i? */
struct tool *tool;			/* The suntool */

char *rindex();
int sigwinched();

main(argc, argv)
int argc;
char **argv;
{
	char **tool_attrs = NULL;	/* User supplied suntool options */
	struct icon my_icon;		/* User's icon */
	struct toolsw *my_sw;		/* Subwindow for the tool */
	struct gfxsubwindow *my_gfx;	/* Graphics subwindow */

/*
 * What's the name of this program?
 */
	progname = (progname = rindex(*argv, '/')) ? ++progname : *argv;
	argc--; argv++;
/*
 * Get user's generic suntool args, if any.
 */
	if ((tool_parse_all(&argc, argv, &tool_attrs, progname)) == -1)
		usage();
/*
 * Load the user's icon
 */
	if (argc != 1)
		usage();
	{
		char errorbuf[IL_ERRORMSG_SIZE + 1];

		if (icon_load(&my_icon, *argv, errorbuf) != 0) {
			(void) fprintf(stderr, "%s: %s: %s.\n",
				       progname, *argv, errorbuf);
			exit(1);
		}
	}
/*
 * Make the sun tool, cleanup attributes,
 */
	if ((tool = tool_make(
			      WIN_WIDTH, my_icon.ic_mpr->pr_width
						+ (2 * TOOL_BORDERWIDTH),
			      WIN_HEIGHT, my_icon.ic_mpr->pr_height
						+ (2 * TOOL_BORDERWIDTH),
			      WIN_NAME_STRIPE, FALSE,
			      WIN_ICONIC, TRUE,
			      WIN_ICON, &my_icon,
			      WIN_ATTR_LIST, tool_attrs,
			      0)) == NULL) {
		fputs("%s: Can't make tool\n", stderr);
		exit(1);
	}
	tool_free_attribute_list(tool_attrs);
	(void) signal(SIGWINCH, sigwinched);
/*
 * Put the user's icon into a subwindow, 
 */
	if ((my_sw = gfxsw_createtoolsubwindow(tool, "",
			my_icon.ic_mpr->pr_width,
			my_icon.ic_mpr->pr_height, NULL)) == NULL) {
		fputs("Can't create icon graphics subwindow\n", stderr);
		exit(1);
	}
	my_gfx = (struct gfxsubwindow *) my_sw->ts_data;
	gfxsw_getretained(my_gfx);
	pw_write(my_gfx->gfx_pixwin, 0, 0, my_icon.ic_mpr->pr_width,
		 my_icon.ic_mpr->pr_height, PIX_SRC, my_icon.ic_mpr, 0, 0);
/*
 * Run the tool.  First install the tool in the tree of windows.
 * then sit in tool_select until the user kills the tool.  When the
 * tool is exited, "destroy" the tool, and exit.
 */
	tool_install(tool);
	tool_select(tool, 0);
	tool_destroy(tool);
}

/*
 * When the window size changes call this routine to effect
 * any damage repairs that are needed.
 */
sigwinched()
{
	tool_sigwinch(tool);
}

usage()
{
	(void) fprintf(stderr, "%s: [suntool args] iconfile\n", progname);
	exit(1);
}
