implementation module dvevents;

import
    picture,
    menu;

from utilities import
    PointInBox, IntersectBoxes;

(* export *)
from menu import
    Menu, MenuEntryHandler;

from menu import
    Destructive, Hide, Highlight, Expose, Execute, 
    GetEntryByCoord, GetUserData;

from vdi import
    AllWritable;

from dvboot import
    GetDialog;

from picture import
    Picture, ConvertToWorldCoord, ConvertToScreenCoord,
    GetOrigin, AlignCenters, GetBoundingBox;

from genlists import
    GenericList, Create, Iterator, BeginIteration, MoreElements,
    EndIteration, Prepend, Head, Append, Delete;

from drawingView import
    DrawingView, HandleMotion, HandleButton, HandleCharacter,
    HandleSelect, HandleUnSelect, HandleChange, HandleRedraw;

(* export *)
from dvdefs import
    DrawingView;

from dvdefs import
    Cursor, StateType,
    GRIDSIZE, LEFTBUTTON, MIDDLEBUTTON, RIGHTBUTTON,
    NULL, CARRIAGERTN, LINEFEED;

from dialog import
    Dialog, UpdateText, GetInputString, ResetInputString, GetPicture, Draw,
    AddWarning, RemoveWarning;

from io import
    SReadf;

from dvwindops import
    RedrawBanner, HandleRedraw;

from dvupict import
    ClipToClipBox;

from dvselect import
    ReleaseSelections;
    
(* export *)
from input import
    InputButton, InputEvent, EventType;

from input import
    SetCursor, GetCursor, QRead;
  
(* export *)
from picture import
    XCoord, YCoord;
    
from dvscroll import
    InScrollBar, ScrollHandler;

(* export *)
from system import
    Word;


(* export type EventPkg = pointer to EventPkgRecord; *)
(* export
type
    EventPkgRecord = record
        view  	: DrawingView;
	button  : InputButton;
	up	: boolean;
	x	: XCoord;
	y	: YCoord;
	c	: char;
	event	: EventType;
    end;
 *)

type
    KeyOpMapEntry = pointer to KeyOpMapRecord;
    KeyOpMapRecord = record
	c : char;
	hiliteMenu : Menu;
	handler : MenuEntryHandler;
	data : Word;
	tool : boolean;
    end;

const SREADARRAY = 100;
    

var
    keyOpMap : GenericList;
    accepted : boolean;
    freelist : GenericList;


(* export *)
procedure MapKeyOp(
    const c : char;
    const hiliteMenu : Menu;
    const handler : MenuEntryHandler;
    const data : Word;
    const tool : boolean
);
    var keyOpMapEntry : KeyOpMapEntry;
begin
    new(keyOpMapEntry);
    keyOpMapEntry^.c := c;
    keyOpMapEntry^.hiliteMenu := hiliteMenu;
    keyOpMapEntry^.handler := handler;
    keyOpMapEntry^.data := data;
    keyOpMapEntry^.tool := tool;
    Prepend(keyOpMapEntry, keyOpMap);
end MapKeyOp;


(* export *)
procedure GetKeyOp(
    const c : char;
    var   hiliteMenu : Menu;
    var   handler : MenuEntryHandler;
    var   data : Word;
    var   tool : boolean
) : boolean;
    var i : Iterator;
        keyOpMapEntry : KeyOpMapEntry;
begin
    i := BeginIteration(keyOpMap);
    while MoreElements(i, keyOpMapEntry) do
        if keyOpMapEntry^.c = c then
	    hiliteMenu := keyOpMapEntry^.hiliteMenu;
	    handler := keyOpMapEntry^.handler;
	    data := keyOpMapEntry^.data;
	    tool := keyOpMapEntry^.tool;
	    EndIteration(i);
	    return true;
	end;
    end;
    EndIteration(i);
    return false;
end GetKeyOp;


(* export *)
procedure ChangeCursor(const newCursor : integer);
begin
    if newCursor # GetCursor() then
        SetCursor(newCursor);
    end;
end ChangeCursor;


(* export *)
procedure EventPackage(
    const view   : DrawingView;
    const button : InputButton;
    const up	 : boolean;
    const x	 : XCoord;
    const y 	 : YCoord;
    const c	 : char;
    const event  : EventType
) : EventPkg;
    var eventPkg : EventPkg;
begin
    eventPkg := EventPkg(Head(freelist));
    if eventPkg = EventPkg(nil) then
        new(eventPkg);
    else
        Delete(eventPkg, freelist);
    end;
    eventPkg^.view := view;
    eventPkg^.button := button;
    eventPkg^.up := up;
    eventPkg^.x := x;
    eventPkg^.y := y;
    eventPkg^.c := c;
    eventPkg^.event := event;
    return eventPkg;
end EventPackage;


procedure HideMenu (view : DrawingView);
var l, r : XCoord;
    b, t : YCoord;
    m : Menu;
    p : Picture;
begin
    m := view^.stateInfo.exposedCmd;
    if Destructive(m) then
	p := menu.GetPicture(m);
	GetBoundingBox(p, l, b, r, t);
	ConvertToWorldCoord(p, l, b);
	ConvertToWorldCoord(p, r, t);
	HandleRedraw(view, l, b, r, t);
    else
	Hide(m);
    end;
end HideMenu;

(* export *)
procedure DisposeEventPackage (var e : EventPkg);
begin
    Prepend(e, freelist);
    e := EventPkg(nil);
end DisposeEventPackage;


(* export *)
procedure CommandHandler(
    const view   : DrawingView;
    const button : InputButton;
    const up	 : boolean;
          x	 : XCoord;
          y 	 : YCoord;
    const c	 : char;
    const event  : EventType
);
    var submenu : Menu;
begin
    ConvertToScreenCoord(view^.world, x, y);
    if
        (view^.stateInfo.activity # Menuing) and 
	(event = BUTTON) and (button = LEFTBUTTON) and not up 
    then
        view^.stateInfo.exposedCmd := GetEntryByCoord(view^.commands, x, y);
	Highlight(view^.stateInfo.exposedCmd);
	Expose(view^.stateInfo.exposedCmd);
	view^.stateInfo.activity := Menuing;

    elsif view^.stateInfo.activity = Menuing then
        submenu := GetEntryByCoord(view^.commands, x, y);
	if submenu # view^.stateInfo.exposedCmd then
	    if submenu # Menu(nil) then
  	        HideMenu(view);
	        Highlight(view^.stateInfo.exposedCmd);
		Highlight(submenu);
                Expose(submenu);
	        view^.stateInfo.exposedCmd := submenu;
                view^.stateInfo.curCmdEntry := Menu(nil);
	    else
	        submenu := 
		    GetEntryByCoord(view^.stateInfo.exposedCmd, x, y);
		if
		    (
		        (submenu = Menu(nil)) or
		        (submenu = view^.stateInfo.curCmdEntry)
		    ) and 
		    (event = BUTTON) and (button = LEFTBUTTON) and up 
		then
(*
 * Upclicked outside exposed menu or on preselected entry.
 *)
	            HideMenu(view);
		    ConvertToWorldCoord(view^.world, x, y);
	            Execute(submenu, view);
		    if not view^.quit then
	    	    	Highlight(view^.stateInfo.exposedCmd);
	    	    	view^.stateInfo.exposedCmd := Menu(nil);
	                view^.stateInfo.curCmdEntry := Menu(nil);
	            	view^.stateInfo.activity := Idling;
		    end;
		elsif submenu # view^.stateInfo.curCmdEntry then
		    Highlight(view^.stateInfo.curCmdEntry);
		    Highlight(submenu);
	            view^.stateInfo.curCmdEntry := submenu;
		end;
	    end;
	elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    HideMenu(view);
	    Highlight(view^.stateInfo.exposedCmd);
	    view^.stateInfo.exposedCmd := Menu(nil);
	    view^.stateInfo.activity := Idling;
	elsif view^.stateInfo.curCmdEntry # Menu(nil) then
	    Highlight(view^.stateInfo.curCmdEntry);
	    view^.stateInfo.curCmdEntry := Menu(nil);
	end;
    end;
end CommandHandler;


procedure HighlightTool(
    const view : DrawingView;
    const tool : Menu
);
begin
    if view^.stateInfo.curTool = tool then
        ClipToClipBox(view);
        ReleaseSelections(view);
        AllWritable();
    else
        Highlight(view^.stateInfo.curTool);
        view^.stateInfo.curTool := tool;
        Highlight(view^.stateInfo.curTool);
    end;
end HighlightTool;


(* export *)
procedure ToolHandler(
    const view   : DrawingView;
    const button : InputButton;
    const up 	 : boolean;
          x	 : XCoord;
          y 	 : YCoord;
    const c	 : char;
    const event  : EventType
);
    var tool : Menu;
begin
    if (event = BUTTON) and (button = LEFTBUTTON) and not up then
	ConvertToScreenCoord(view^.world, x, y);
	tool := GetEntryByCoord(view^.tools, x, y);
	HighlightTool(view, tool);
    end;
end ToolHandler;


(* export *)
procedure KeyOpHandler(
    const view : DrawingView;
    const c : char
);
    var hiliteMenu : Menu;
        handler : MenuEntryHandler;
	data : Word;
	tool : boolean;
begin
    if GetKeyOp(c, hiliteMenu, handler, data, tool) then
	if tool then
	    HighlightTool(view, hiliteMenu);
	else
	    Highlight(hiliteMenu);
	    view^.stateInfo.curCmdEntry := Menu(data);
	    view^.stateInfo.exposedCmd := hiliteMenu;
	    handler^(view);
	    view^.stateInfo.exposedCmd := Menu(nil);
	    view^.stateInfo.curCmdEntry := Menu(nil);
	    Highlight(hiliteMenu);
	end;
    end;
end KeyOpHandler;	


(* export *)
procedure SplitEvent(
    const v : DrawingView;
    const ev : InputEvent
);
begin
    case ev.eventType of
	BUTTON:
	    accepted := HandleButton(v, ev.key, ev.up, ev.kx, ev.ky);
	    |
	CHARACTER:
	    accepted := HandleCharacter(v, ev.ch, false, ev.cx, ev.cy);
	    |
	MOTION:
	    accepted := HandleMotion(v, ev.mx, ev.my);
	    |
	REDRAW:
	    HandleRedraw(v, ev.left, ev.bottom, ev.right, ev.top);
	    menu.Uncache(v^.commands);
	    |
	RESIZE:
	    HandleChange(v, 0, 0, ev.width - 1, ev.height - 1);
	    |
	SELECT:
	    HandleSelect(v);
	    |
	UNSELECT:
	    HandleUnSelect(v);
	else
    end;
end SplitEvent;


procedure ConvertActivity(const state : StateType) : StateType;
begin
    if state = Selecting then
        return TempSelecting;
    elsif state = Rubberbanding then
 	return TempRubberbanding;
    elsif state = TempSelecting then
	return Selecting;
    elsif state = TempRubberbanding then
	return Rubberbanding;
    else
	return state;
    end;
end ConvertActivity;


(* export *)
procedure HandleEvent(
    const view   : DrawingView;
          button : InputButton;
    const up	 : boolean;
    	  x	 : XCoord;
    	  y 	 : YCoord;
    const c	 : char;
    const event  : EventType
);
    var cursor : Cursor;
        origTool : Menu;
        screenX, origx : XCoord;
	screenY, origy : YCoord;
begin
    screenX := x;
    screenY := y;
    ConvertToScreenCoord(view^.world, screenX, screenY);
    if
        (event = BUTTON) and (button = MIDDLEBUTTON) and
	(view^.stateInfo.activity = Idling)
    then
	if not up then	    (* otherwise absorb event to avoid confusion *)
	    ClipToClipBox(view);
	    ReleaseSelections(view);
	    AllWritable();
	end;

(*
 * Begin momentary select stuff.
 *)
    elsif
	(event = BUTTON) and (button = RIGHTBUTTON) and not up and
        (view^.stateInfo.activity = Idling) and
	PointInBox(
	    screenX, screenY,
	    view^.pictClipBox.x0, view^.pictClipBox.y0,
	    view^.pictClipBox.x1, view^.pictClipBox.y1
	)
    then
        if view^.stateInfo.curTool = view^.selectTool then
	    HandleEvent(view, LEFTBUTTON, up, x, y, c, event);
	else
            origTool := view^.stateInfo.curTool;
	    HighlightTool(view, view^.selectTool);
	    HandleEvent(view, LEFTBUTTON, up, x, y, c, event);
	    view^.stateInfo.curTool := origTool;
	    view^.stateInfo.activity := ConvertActivity(view^.stateInfo.activity);
	end;
    elsif 
        (view^.stateInfo.activity = TempSelecting) or
	(view^.stateInfo.activity = TempRubberbanding)
    then
	if (event # BUTTON) or (button # MIDDLEBUTTON) then
	    view^.stateInfo.activity := 
		ConvertActivity(view^.stateInfo.activity);
	    origTool := view^.stateInfo.curTool;
	    view^.stateInfo.curTool := view^.selectTool;
	    HandleEvent(view, LEFTBUTTON, up, x, y, c, event);
	    if view^.stateInfo.activity = Idling then
		HighlightTool(view, origTool);
	    else
		view^.stateInfo.curTool := origTool;
		view^.stateInfo.activity := 
		    ConvertActivity(view^.stateInfo.activity);
	    end;
	end;
    
    elsif
        (event = BUTTON) and (button = RIGHTBUTTON) and up and
	(
	    (view^.stateInfo.activity = Selecting) or
	    (view^.stateInfo.activity = Rubberbanding)
        )
    then
	HandleEvent(view, LEFTBUTTON, up, x, y, c, event);
(*
 * End momentary select stuff.
 *)

    elsif (view^.stateInfo.activity = Idling) and (event = CHARACTER) then
        KeyOpHandler(view, c);
    elsif view^.stateInfo.activity = TextEntering then
	Execute(
	    view^.stateInfo.curTool, 
	    EventPackage(view, button, up, x, y, c, event)
	);
    elsif
        (view^.stateInfo.activity = Menuing) or
        (
	    (event = BUTTON) and (button = LEFTBUTTON) and not up and
	    (view^.stateInfo.activity = Idling) and
            (GetEntryByCoord(view^.commands, screenX, screenY) # Menu(nil))
	)
    then
        ChangeCursor(view^.defaultCursor);
	CommandHandler(view, button, up, x, y, c, event);

    elsif 
        (view^.stateInfo.activity = Scrolling) or
	(
	    (view^.stateInfo.activity = Idling) and 
	    InScrollBar(view, screenX, screenY) and 
	    			(* sets curScroll as side effect *)
	    (event = BUTTON) and (button = LEFTBUTTON) and not up
	)
    then
        ScrollHandler(view, button, up, screenX, screenY, c, event);

    elsif
        (event = BUTTON) and (button = LEFTBUTTON) and not up and
	(view^.stateInfo.activity = Idling) and
        (GetEntryByCoord(view^.tools, screenX, screenY) # Menu(nil))
    then
        ToolHandler(view, button, up, x, y, c, event);
	
    else
        if
	    PointInBox(
		screenX, screenY,
		view^.pictClipBox.x0, view^.pictClipBox.y0,
		view^.pictClipBox.x1, view^.pictClipBox.y1
	    )
	then
	    cursor := Cursor(GetUserData(view^.stateInfo.curTool));
	    ChangeCursor(cursor^);
	else
	    ChangeCursor(view^.defaultCursor);
	end;
	
	if view^.stateInfo.gridOn then
	    GetOrigin(view^.pict, origx, origy);
	    x := GRIDSIZE * (x div GRIDSIZE) + origx mod GRIDSIZE;
	    y := GRIDSIZE * (y div GRIDSIZE) + origy mod GRIDSIZE;
	end;

        if
	    (
	        (view^.stateInfo.activity = Idling) and
		(event = BUTTON) and (button = LEFTBUTTON) and not up
	    ) or 
	    (view^.stateInfo.activity = Rubberbanding) or
	    (view^.stateInfo.activity = TextEntering) or
	    (view^.stateInfo.activity = Selecting)
	then
	    Execute(
	        view^.stateInfo.curTool, 
		EventPackage(view, button, up, x, y, c, event)
	    );
	end;
    end;        
end HandleEvent;


(* export *)
procedure EraseDialog(const view : DrawingView);
    var dialogPict : Picture;
	left, right : XCoord;
	bottom, top : YCoord;
	oldState : StateType;
begin
    if view^.stateInfo.curDialog # Dialog(nil) then
	dialogPict := GetPicture(view^.stateInfo.curDialog);
	GetBoundingBox(dialogPict, left, bottom, right, top);
	oldState := view^.stateInfo.activity;
	view^.stateInfo.activity := Idling;
	HandleRedraw(view, left, bottom, right, top);
	view^.stateInfo.activity := oldState;
    end;
end EraseDialog;


(* export *)
procedure QuickEraseDialog(const view : DrawingView);
    var left, right : XCoord;
	bottom, top : YCoord;
begin
    if view^.stateInfo.curDialog # Dialog(nil) then
	GetBoundingBox(
	    GetPicture(view^.stateInfo.curDialog), left, bottom, right, top
	);
	RedrawBanner(view, left, bottom, right, top);
	picture.DrawInBoundingBox(view^.world, left, bottom, right, top);
	picture.DrawInBoundingBox(view^.indicator, left, bottom, right, top);
	menu.DrawInBoundingBox(view^.commands, left, bottom, right, top);
	menu.DrawInBoundingBox(view^.tools, left, bottom, right, top);
	menu.HighlightInBoundingBox(
	    view^.stateInfo.curTool, left, bottom, right, top
	);
	if (view^.stateInfo.exposedCmd # menu.Menu(nil)) then
	    menu.HighlightInBoundingBox(
		view^.stateInfo.exposedCmd, left, bottom, right, top
	    );
	end;
	picture.DrawInBoundingBox(
	    view^.xscroll.scrollBar, left, bottom, right, top
	);
	picture.DrawInBoundingBox(
	    view^.yscroll.scrollBar, left, bottom, right, top
	);
    end;
end QuickEraseDialog;	


(* export *)
procedure GetDialogInput(
    const view : DrawingView;
    const dialogId : integer;
    var name : array of char;
    const erase : boolean
);
    var e : InputEvent;
	oldState : StateType;
begin
    oldState := view^.stateInfo.activity;
    view^.stateInfo.activity := Dialoging;
    view^.stateInfo.curDialog := GetDialog(dialogId);
    Draw(view^.stateInfo.curDialog);

    loop
	QRead(e);
	case e.eventType of
	    MOTION:
		accepted := HandleMotion(view, e.mx, e.my);
		|
	    REDRAW:
		HandleRedraw(view, e.left, e.bottom, e.right, e.top);
		|
	    RESIZE:
		HandleChange(view, 0, 0, e.width - 1, e.height - 1);
		|
	    SELECT:
		HandleSelect(view);
		|
	    UNSELECT:
		HandleUnSelect(view);
		|
	    BUTTON:
		if e.up then
		    exit
		else
		    case e.key of
			LEFTBUTTON:
			    UpdateText(view^.stateInfo.curDialog, 'y');
			    |
			MIDDLEBUTTON:
			    UpdateText(view^.stateInfo.curDialog, 'n');
			else
		    end;
		end;
		|
	    CHARACTER:
		if (e.ch = LINEFEED) or (e.ch = CARRIAGERTN) then
		    exit;
		else
		    UpdateText(view^.stateInfo.curDialog, e.ch);
		end;
	    else
	end;
    end;
    if erase then
	EraseDialog(view);
    end;
    GetInputString(view^.stateInfo.curDialog, name);
    ResetInputString(view^.stateInfo.curDialog);
    view^.stateInfo.activity := oldState;
end GetDialogInput;


(* export *)
procedure GetFloatInputs(
    const view : DrawingView;
    const dialogId : integer;
    var f1, f2 : real;
    const erase : boolean
) : integer;
    var buf : array [1..SREADARRAY] of char;
begin
    GetDialogInput(view, dialogId, buf, erase);
    if buf[1] = NULL then
        return 0;
    else
	return SReadf(buf, "%f %f", f1, f2);
    end;
end GetFloatInputs;


begin
    Create(keyOpMap);
    Create(freelist);
end dvevents.
