/*
 * handle the tool environment 
 */

#include <stdio.h>
#include <suntool/tool_hs.h>
#include <suntool/panel.h>
#include <suntool/gfxsw.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <strings.h>

#include "nchess.h"

Tool * NchessTool;
char ** ToolAttrs = (char **) NULL;
char * ToolName;

/*
 * iconic form of window
 */
unsigned short IconImage[] = {
#include "Icons/nchess.icon"
};

DEFINE_ICON_FROM_IMAGE(WindowIcon, IconImage);

sigwinched()
{
    tool_sigwinch(NchessTool);
}

/*
 * parse the tool args.
 */
void 
ParseToolArgs(argc, argv)
    int *argc;
    char **argv;
{
    ToolName = argv[0];
    (*argc)--;
    argv++;
    if (tool_parse_all(argc, argv, &ToolAttrs, ToolName) == -1) {
	tool_usage(ToolName);
	exit(1);
    }
    (*argc)++;
}

/*
 * set up the tool environment
 */
void
InitTool(useRetained, iconDirectory)
    BOOL useRetained;		/* use a retained pixrect for the board */
    char * iconDirectory;	/* custom piece icon directory */
{
    register unsigned int height;
    register Toolsw * twsp;
    char c[256];

    /* create the tool marquee */
    strcpy(c, " ");
    strcat(c, PlayerName[WHITE]);
    strcat(c, " vs. ");
    strcat(c, PlayerName[BLACK]);
    if ((NchessTool = tool_make( 
	WIN_LABEL, c, 
	WIN_ICON, &WindowIcon, 
	WIN_ATTR_LIST, ToolAttrs,
	WIN_WIDTH, 8 * (SQUARE_WIDTH-1) + 2 * tool_borderwidth(NchessTool) - 1,
	/*
	 * NOTE: The following line was unnecessary in Sun release 2.2,
	 * but is necessary in release 3.0.  For some unknown reason, the
	 * call to tool_set_attributes following the call to InitBoardSW
	 * now fails to uncover the board area that was obscured by the 
	 * default tool height being too small (note that the tool has not 
	 * been installed yet).
	 */
	WIN_HEIGHT, 2000,
	0)) == (Tool *) NULL) 
    {
	fputs("Can't make tool\n", stderr);
	exit(1);
    }
    tool_free_attribute_list(ToolAttrs);
    /* initialize the subwindows */
    InitTalkSW();
    InitMsgSW();
    InitControlSW();
    InitBoardSW(useRetained, iconDirectory);
    /* 
     * add up subwindow heights and force the tool to the resulting size 
     */
    height = tool_stripeheight(NchessTool) + tool_borderwidth(NchessTool) - tool_subwindowspacing(NchessTool);
    for (twsp = NchessTool->tl_sw ; twsp != (Toolsw *) 0 ; twsp = twsp->ts_next) 
	height += twsp->ts_height + tool_subwindowspacing(NchessTool);
    /*
     * NOTE: under 2.2, the above calculation yielded the correct height.
     * under 3.0, we need to add a few pixels to make it come out right (the
     * reason is not yet known).
     */
    height += 2;
    tool_set_attributes(NchessTool, WIN_HEIGHT, height, 0); 
    signal(SIGWINCH, sigwinched);
}

void
RunTool()
{
    /*
     * NOTE: this is another difference between release 2.2 and 3.0:
     * in release 2.2, the SIGWINCH handler would get called once at the
     * outset to draw the board area; in release 3.0, this doesn't happen.
     */
    DrawBoard();
    tool_select(NchessTool, 0);
    tool_destroy(NchessTool);
}

