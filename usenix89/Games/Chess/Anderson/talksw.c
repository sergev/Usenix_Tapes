/*
 * talk subwindow handling
 */

#include <stdio.h>
#include <suntool/tool_hs.h>
#include <suntool/panel.h>
#include <strings.h>

#include "nchess.h"

#define	MAX_SEND_LENGTH		60

struct toolsw * TalkSW;
Panel TalkPanel;

Panel_item SendItem;
Panel_item RecvItem;

/*ARGSUSED*/
sendProc(item, event)
    Panel_item item;
    struct inputevent *event;
{
    char c[MAX_SEND_LENGTH+1];

    strcpy(c, (char *) panel_get_value(SendItem));
    SendTalkMsg(c);
    /* set the command text to nil */
    panel_set_value(SendItem, "");
}

/*
 * set up the talk send and receive subwindows 
 * (if we are playing against the machine, leave them out)
 */
void
InitTalkSW()
{
    if (IsMachine[PeerColor])
	return;
    if ((TalkSW = panel_create(NchessTool, 0)) == NULL) {
	fprintf(stderr, "Can't create talk subwindows\n");
	exit(1);
    }
    TalkPanel = TalkSW->ts_data;
    RecvItem = panel_create_item(TalkPanel, PANEL_MESSAGE,
	PANEL_LABEL_STRING, "Recv:",
	PANEL_SHOW_ITEM, TRUE,
	0);
    SendItem = panel_create_item(TalkPanel, PANEL_TEXT,
	PANEL_LABEL_STRING, "Send:",
	PANEL_NOTIFY_STRING, "\n\r",
	PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
	PANEL_NOTIFY_PROC, sendProc,
	PANEL_VALUE_STORED_LENGTH, MAX_SEND_LENGTH,
	0);
    panel_fit_height(TalkPanel); 
}

/*
 * receive a pithy banality from the opponent
 */
void
RecvTalkMsg(cp)
    char *cp;
{
    char ncp[128];

    strcpy(ncp, "Recv: ");
    panel_set(RecvItem, 
	PANEL_LABEL_STRING, strcat(ncp, cp),
	0);
}
