/*
 * control panel subwindow handling
 */

#include <stdio.h>
#include <suntool/tool_hs.h>
#include <suntool/panel.h>
#include <suntool/menu.h>

#include "nchess.h"

BOOL SaveWanted = FALSE;		/* machine opponent and game save deferred */

struct toolsw * ControlPanelSW;
Panel ControlPanel;

/* button items */
Panel_item UndoButton;
Panel_item LastPlayButton;
Panel_item ResignButton;
Panel_item TranscriptButton;
Panel_item SaveButton;

/*
 * undo move button event handler
 */
/*ARGSUSED*/
undoProc(item, event)
    Panel_item item;
    struct inputevent * event;
{
    if ( ! IHaveMoved()) {
	Message("You haven't moved yet!");
    } else if (Mouse == IDLE) {
	UndoWanted = TRUE;
	Mouse = LOCKED;		/* lock the mouse until the reply arrives */
	SendUndoRequest(PeerColor);
    }
}

/*
 * last play button event handler 
 */
/*ARGSUSED*/
lastPlayProc(item, event)
    Panel_item item;
    struct inputevent * event;
{
    ShowLastPlay();
}

/*
 * resign button event handler 
 */
/*ARGSUSED*/
resignProc(item, event)
    Panel_item item;
    struct inputevent * event;
{
    /* make sure this is what the user wants to do */
    if (Mouse == IDLE) {
	ConfirmResignation();
    }
}

/*
 * write a transcript 
 */
/*ARGSUSED*/
transcriptProc(item, event)
    Panel_item item;
    struct inputevent * event;
{
    WriteTranscript(TranscriptFileName, TranscriptType);
}

/*
 * save the game
 */
/*ARGSUSED*/
saveProc(item, event)
    Panel_item item;
    struct inputevent * event;
{
    /*
     * caveats: can't save games while we're still setting them up,
     * and cannot save a game involving machines after the game is over.
     */
    if (SetupMode 
    || GameOver && (IsMachine[WHITE] || IsMachine[BLACK]))
	return;
    if (IsMachine[WHITE] && IsMachine[BLACK]
    || IsMachine[PeerColor] && Turn != MyColor) {
	Message("Will save game when machine finishes move...");
	SaveWanted = TRUE;
    } else {
	SaveGame(SaveFileName);
    }
}

/*
 * set up the control subwindow
 */
void
InitControlSW()
{
    if ((ControlPanelSW = panel_create(NchessTool, 0)) == NULL) {
	fprintf(stderr, "Can't create control panel\n");
	exit(1);
    }
    ControlPanel = ControlPanelSW->ts_data;
    /* set up the buttons */
    if ( ! IsMachine[WHITE] || ! IsMachine[BLACK]) {
	ResignButton = panel_create_item(ControlPanel, PANEL_BUTTON,
	    PANEL_LABEL_STRING, "(Resign)",
	    PANEL_NOTIFY_PROC, resignProc,
	    0);
	UndoButton = panel_create_item(ControlPanel, PANEL_BUTTON,
	    PANEL_LABEL_STRING, "(Undo)",
	    PANEL_NOTIFY_PROC, undoProc,
	    0);
    }
    LastPlayButton = panel_create_item(ControlPanel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "(Last Play)",
	PANEL_NOTIFY_PROC, lastPlayProc,
	0);
    TranscriptButton = panel_create_item(ControlPanel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "(Transcript)",
	PANEL_NOTIFY_PROC, transcriptProc,
	0);
    SaveButton = panel_create_item(ControlPanel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "(Save)",
	PANEL_NOTIFY_PROC, saveProc,
	0);
    panel_fit_height(ControlPanel);
}
