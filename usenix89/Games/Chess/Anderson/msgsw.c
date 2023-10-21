/*
 * message subwindow handling
 */

#include <stdio.h>
#include <ctype.h>
#include <suntool/tool_hs.h>
#include <suntool/panel.h>
#include <strings.h>

#include "nchess.h"

struct toolsw * MessageSW;
Panel MessagePanel;

Panel_item MessageItem;

/*
 * set up the message subwindow
 */
void
InitMsgSW()
{
    if ((MessageSW = panel_create(NchessTool, 0)) == NULL) {
	fprintf(stderr, "Can't create message panel\n");
	exit(1);
    }
    MessagePanel = MessageSW->ts_data;
    /* create the message panel item */
    MessageItem = panel_create_item(MessagePanel, PANEL_MESSAGE,
	PANEL_LABEL_STRING, 
	    RestoringGame ? "Please wait..." :
	    SetupMode ? "Setup: left - source, middle - delete, right - end" :
	    IsMachine[Turn] || Turn != MyColor ? 
		(Turn == WHITE ?
		    "Waiting for white to play..." : 
		    "Waiting for black to play...") :
		"Your move" ,
	PANEL_SHOW_ITEM, TRUE,
	0);
    panel_fit_height(MessagePanel); 
}

void 
Message(cp)
    char * cp;
{
    panel_set(MessageItem,
	PANEL_LABEL_STRING, cp,
	PANEL_SHOW_ITEM, TRUE,
	0);
}

/*
 * write the standard "your move", "waiting ..." message
 * pre-pended with the passed string (if non-null)
 */
void
WhoseMoveMessage(cp)
    char * cp;
{
    char c1[256], c2[256];

    if (GameOver) 
	strcpy(c2, "Game over");
    else if (IsMachine[Turn] || Turn != MyColor)
	strcpy(c2, Turn == WHITE ? 
	    "Waiting for white to play..." :
	    "Waiting for black to play...");
    else if (InCheck())
	strcpy(c2, "Your move (check)");
    else
	strcpy(c2, "Your move");

    if (cp != (char *) 0) {
	c2[0] = tolower(c2[0]);
	strcpy(c1, cp);
	strcat(c1, " - ");
	Message(strcat(c1, c2));
    } else 
	Message(c2);
}

