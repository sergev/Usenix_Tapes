implementation module dvwindops;

import
    picture,
    rubband,
    scrollbar,
    dvboot,
    dialog,
    menu,
    genlists;

(* export *)
from drawing import
    Drawing;

(* export *)
from dvdefs import
    DrawingView;

from dvdefs import
    Box, LineType, UndoObj, StateType, MINHEIGHT, MINWIDTH,
    VERSION, DATE, INFOOP, NULL,
    LEFTBUTTON, MIDDLEBUTTON, RIGHTBUTTON, DEFAULTFONT;

from dvboot import
    NONE, LINE, LEFTARROW, RIGHTARROW, DEFAULTFONTUID;

from dvselect import
    HighlightSelectionsInBbox;

(* export *)
from picture import
    Picture, XCoord, YCoord, WHITE, BLACK;

from cursorset import
    MakeIVCursors, NumIVCursors, GetIVCursor;

from dirs import
    getwd;

from strings import
    Assign;

from vdi import
    SetColors, SetForegroundColor, SetPattern, clearPattern, stipplePattern1,
    GetHeight, StrWidth, CharStr, Font, GetFont, SetFont,
    FilledRectangle, Rectangle, Line, MoveTo, GetPosition,
    SetRendering, RenderingFunction, Writable, AllWritable,
    XMAXSCREEN, YMAXSCREEN, Sync;

from io import
    SWritef;

from dvupict import
    CalcPictClipBox, CalcClipBox, CenterPage, DrawPageInClipBox;

from dvevents import
    HandleEvent, MapKeyOp;

from dvscroll import
    UpdateScrollBars;

from ps import
    MapFont, GetScreenFont;

(* export *)
from input import
    InputButton;

from input import
    SetCursor, defaultCursor, hourglassCursor, KeyToChar,
    EventType, QButton, UnQButton;

from genlists import
    GenericList, Release;

from utilities import
    IntersectBoxes, PointInBox;

from system import
    Word;

var
    dummyX : XCoord;
    dummyY : YCoord;
    bannerHt : integer;
    infoHandler : menu.MenuEntryHandler;
    infoShown : boolean;
    cursorPos : integer;


(* public export *)
procedure OpenDrawingView (
    d : Drawing; var dv : DrawingView
);
begin
    new(dv);
    dv^.drawing := d;
    dv^.selected := false;
    dv^.quit := false;
    dv^.noFileSpecified := true;
    MapKeyOp(INFOOP, menu.Menu(nil), infoHandler, dv, false);
end OpenDrawingView;


(* export *)
procedure AdvCursor();
begin
    cursorPos := (cursorPos + 1) mod (NumIVCursors() - 1);
    SetCursor(GetIVCursor(cursorPos));
end AdvCursor;


procedure PlaceDrawingView (
    i : DrawingView; left : XCoord; top : YCoord; width, height : integer
);
    var right : XCoord;
        bottom : YCoord;
	font : integer;
begin
    i^.width := width;
    i^.height := height;
    i^.left := left;
    i^.top := top;

    right := left + width;
    bottom := top - height;

    bannerHt := GetHeight() + 1;
    i^.world := dvboot.CreateWorld(width, height - bannerHt);
    picture.SetOrigin(i^.world, left, bottom);
    font := GetFont();
    MapFont(font, DEFAULTFONT, 10, 0.0, 2.5);
    picture.MapFont(font, DEFAULTFONTUID);
    i^.stateInfo.dirty := false;
    i^.stateInfo.redrawOn := true;
    RedrawBanner(i, left, bottom, right, top);
    picture.Draw(i^.world);

    dvboot.LoadFonts();
    dvboot.CreateShades();
    MakeIVCursors();
    SetCursor(GetIVCursor(NumIVCursors() - 1));

    i^.indicator := dvboot.CreateIndicator(i^.lineIndic, i^.patIndic);
    AdvCursor();
    i^.commands := dvboot.CreateCommands();
    AdvCursor();
    dvboot.CreateDialogs();
    dvboot.SetDialogOrigins(left, bottom);
    dvboot.AlignDialogCenters(i^.world);
    AdvCursor();
    i^.tools := dvboot.CreateTools();
    AdvCursor();
    i^.selectTool := menu.GetEntry(i^.tools, 1);
    picture.SetOrigin(i^.indicator, left, bottom);
    menu.SetOrigin(i^.commands, left, bottom);
    menu.SetOrigin(i^.tools, left, bottom);

    i^.pict := dvboot.CreateUserPicture(
	picture.GetLineStyle(i^.lineIndic),
	picture.GetPattern(i^.patIndic),
	picture.Fill
    );
    AdvCursor();
    i^.page := dvboot.CreatePage();
    i^.pictClipBox := CalcClipBox(i);

    AdvCursor();
    dvboot.CreateScrollBars(
        i^.xscroll.scrollBar, i^.xscroll.autoDownButton,
	i^.xscroll.autoUpButton, i^.xscroll.pageArea, i^.xscroll.slider,
        i^.yscroll.scrollBar, i^.yscroll.autoDownButton,
	i^.yscroll.autoUpButton, i^.yscroll.pageArea, i^.yscroll.slider
    );
    AdvCursor();
    picture.Translate(i^.xscroll.scrollBar, float(i^.pictClipBox.x0), 0.0);
    picture.Translate(
        i^.yscroll.scrollBar, 
	float(i^.pictClipBox.x1), float(i^.pictClipBox.y0)
    );
    scrollbar.ChangeLength(
        i^.xscroll.scrollBar, i^.pictClipBox.x1 - i^.pictClipBox.x0
    );
    scrollbar.ChangeLength(
        i^.yscroll.scrollBar, i^.pictClipBox.y1 - i^.pictClipBox.y0
    );

    picture.SetOrigin(i^.pict, i^.left, i^.bottom);
    picture.SetOrigin(i^.page, i^.left, i^.bottom);
    picture.SetOrigin(i^.xscroll.scrollBar, i^.left, i^.bottom);
    picture.SetOrigin(i^.yscroll.scrollBar, i^.left, i^.bottom);

    AdvCursor();
    menu.Draw(i^.tools);
    AdvCursor();
    InitState(i);
    picture.Draw(i^.indicator);
    AdvCursor();
    menu.Draw(i^.commands);
    AdvCursor();
    CenterPage(i);
    UpdateScrollBars(i);
    picture.Draw(i^.xscroll.scrollBar);
    picture.Draw(i^.yscroll.scrollBar);
    AdvCursor();
    DrawPageInClipBox(i);
end PlaceDrawingView;


procedure ChangeWorld(
    const view : DrawingView;
    const left : XCoord;
    const bottom : YCoord;
    const right : XCoord;
    const top : YCoord
);
    var dxLeft, dxRight, x0, x1 : XCoord;
        dyBottom, dyTop, y0, y1 : YCoord;
	frame, clipBox : Picture;
begin
    clipBox := picture.GetNestedPicture(view^.world, -1);
    picture.GetBoundingBox(clipBox, dxLeft, dyBottom, dxRight, dyTop);
    picture.GetBoundingBox(view^.world, x0, y0, x1, y1);
    dxLeft := dxLeft - x0;
    dyBottom := dyBottom - y0;
    dxRight := dxRight - x1;
    dyTop := dyTop - y1;

    picture.Destroy(view^.world);
    picture.Create(view^.world);
    picture.DisableRedraw(view^.world);
    picture.Rectangle(view^.world, 0, 0, right - left, top - bottom);
    frame := picture.Copy(view^.world);
    picture.AlterRenderingStyle(view^.world, picture.Fill);
    picture.AlterColor(view^.world, picture.WHITE);
    picture.Nest(frame, view^.world);

    picture.Create(clipBox);
    picture.DisableRedraw(clipBox);
    picture.Rectangle(
        clipBox, dxLeft, dyBottom, right - left + dxRight, top - bottom + dyTop
    );
    picture.Nest(clipBox, view^.world);
    picture.SetOrigin(view^.world, left, bottom);
    dvboot.SetDialogOrigins(left, bottom);
    picture.SetOrigin(view^.xscroll.scrollBar, left, bottom);
    picture.SetOrigin(view^.yscroll.scrollBar, left, bottom);

    view^.pictClipBox := CalcClipBox(view);
    picture.GetBoundingBox(view^.yscroll.scrollBar, x0, y0, x1, y1);
    picture.Translate(
        view^.yscroll.scrollBar, 
	float(view^.pictClipBox.x1 - x0), 0.0
    );
    scrollbar.ChangeLength(
        view^.xscroll.scrollBar, view^.pictClipBox.x1 - view^.pictClipBox.x0
    );
    scrollbar.ChangeLength(
        view^.yscroll.scrollBar, view^.pictClipBox.y1 - view^.pictClipBox.y0
    );
end ChangeWorld;


procedure InitState(const view : DrawingView);
    var dummy : integer;
begin
    view^.stateInfo.activity := Idling;
    view^.stateInfo.rubberband := rubband.Rubberband(nil);
    view^.stateInfo.exposedCmd := menu.Menu(nil);
    view^.stateInfo.curCmdEntry := menu.Menu(nil);
    view^.stateInfo.curTool := menu.GetEntry(view^.tools, 1);
    view^.stateInfo.curScroll := Picture(nil);
    view^.stateInfo.curDialog := dialog.Dialog(nil);
    genlists.Create(view^.stateInfo.selecList);
    genlists.Create(view^.stateInfo.charList);
    view^.stateInfo.textPict := Picture(nil);
    view^.stateInfo.gridOn := false;
    view^.stateInfo.selecSorted := false;
    view^.stateInfo.landscape := false;
    view^.stateInfo.lineType := Plain;
    view^.stateInfo.zoom := 0;
    view^.stateInfo.undoInfo := UndoObj(nil);
    UpdateLineIndicator(view);
    menu.Highlight(view^.stateInfo.curTool);
    view^.bannermsg1[1] := NULL;
    view^.bannermsg2[1] := NULL;
    dummy := getwd(view^.curPath);
end InitState;


(* export *)
procedure CopyIndicatorInfo(const source, dest : Picture);
begin
    picture.DefaultFont(dest, picture.GetFont(source));
    picture.DefaultColor(dest, picture.GetColor(source));
    picture.DefaultPattern(dest, picture.GetPattern(source));
    picture.DefaultRenderingStyle(dest, picture.GetRenderingStyle(source));
end CopyIndicatorInfo;


(* export *)
procedure UpdateLineIndicator(const view : DrawingView);
    var none, line, leftArrow, rightArrow : Picture;
begin
    none       := picture.GetNestedPicture(view^.lineIndic, NONE);
    line       := picture.GetNestedPicture(view^.lineIndic, LINE);
    leftArrow  := picture.GetNestedPicture(view^.lineIndic, LEFTARROW);
    rightArrow := picture.GetNestedPicture(view^.lineIndic, RIGHTARROW);
    
    case view^.stateInfo.lineType of
        None:
	    picture.Enable(none);
	    picture.Disable(line);
	    picture.Disable(leftArrow);
	    picture.Disable(rightArrow);
	|
        Plain:
	    picture.Disable(none);
	    picture.Enable(line);
	    picture.Disable(leftArrow);
	    picture.Disable(rightArrow);
	|
	LeftArrow:
	    picture.Disable(none);
	    picture.Enable(line);
	    picture.Enable(leftArrow);
	    picture.Disable(rightArrow);
	|
	RightArrow:
	    picture.Disable(none);
	    picture.Enable(line);
	    picture.Disable(leftArrow);
	    picture.Enable(rightArrow);
	|
	DblArrow:
	    picture.Disable(none);
	    picture.Enable(line);
	    picture.Enable(leftArrow);
	    picture.Enable(rightArrow);
    end;
end UpdateLineIndicator;


(* export *)
procedure Destroy(var view : DrawingView);
begin
(*
 *  No need to clean up when running externally.
 *
    Release(view^.stateInfo.selecList);
    picture.Destroy(view^.world);
    picture.Destroy(view^.indicator);
    menu.Destroy(view^.commands);
    menu.Destroy(view^.tools);
    picture.Destroy(view^.pict);
    picture.Destroy(view^.page);
    picture.Destroy(view^.xscroll.scrollBar);
    picture.Destroy(view^.yscroll.scrollBar);
    dispose(view);
 *)
end Destroy;


(* public export *)
procedure HandleMotion(
    const view : DrawingView;
    const x    : XCoord;
    const y    : YCoord
) : boolean;
begin
    if PointInBox(x, y, 0, 0, XMAXSCREEN, YMAXSCREEN - bannerHt) then
	QButton(MIDDLEBUTTON);
	QButton(RIGHTBUTTON);
	QButton(LEFTBUTTON);
    else
	if 
	    (view^.stateInfo.activity = Idling) or
	    (view^.stateInfo.activity = Dialoging)
	then
	    UnQButton(MIDDLEBUTTON);
	    UnQButton(RIGHTBUTTON);
	    if PointInBox(x, y, 0, 0, XMAXSCREEN, YMAXSCREEN) then
		UnQButton(LEFTBUTTON);
	    else
	    	QButton(LEFTBUTTON);
	    end;
	else
	    QButton(RIGHTBUTTON);
	    QButton(LEFTBUTTON);
	end;
    end;
    HandleEvent(view, 0, false,  x, y, ' ', MOTION);
    return true;
end HandleMotion;


(* public export *)
procedure HandleButton(
    const view   : DrawingView;
    const button : InputButton;
    const up	 : boolean;
    const x      : XCoord;
    const y      : YCoord
) : boolean;
    var c : integer;
begin
    HandleEvent(view, button, up, x, y, ' ', BUTTON);
    return true;
end HandleButton;


(* public export *)
procedure HandleCharacter(
    const view   : DrawingView;
    const ch	 : char;
    const up	 : boolean;
    const x      : XCoord;
    const y      : YCoord
) : boolean;
    var dummy : integer;
begin
    if not up then
        HandleEvent(view, dummy, up, x, y, ch, CHARACTER);
    end;       
    return true;
end HandleCharacter;


procedure Translate(
    const view : DrawingView;
    const x0 : XCoord;
    const y0 : YCoord
);
begin
    picture.SetOrigin(view^.world, x0, y0);
    dvboot.SetDialogOrigins(x0, y0);
    picture.SetOrigin(view^.indicator, x0, y0);
    menu.SetOrigin(view^.commands, x0, y0);
    menu.SetOrigin(view^.tools, x0, y0);
    picture.SetOrigin(view^.xscroll.scrollBar, x0, y0);
    picture.SetOrigin(view^.yscroll.scrollBar, x0, y0);
    view^.pictClipBox := CalcClipBox(view);
    (* reset clipBox *)
end Translate;


(* public export *)
procedure HandlePlace (
    i : DrawingView; 
    left : XCoord; bottom : YCoord; right : XCoord; top : YCoord
);
begin
    i^.left := left;
    i^.bottom := bottom;
    i^.right := right;
    i^.top := top;

    i^.defaultCursor := defaultCursor;
    PlaceDrawingView(i, left, top, right - left, top - bottom);
    SetCursor(i^.defaultCursor);
end HandlePlace;


(*
 *  HandleChange has been hacked to work with a particular menu/tool
 *  layout.  A general algorithm should be developed to handle arbitrary
 *  layouts.
 *)
(* public export *)
procedure HandleChange (
    i : DrawingView; 
    left : XCoord; bottom : YCoord; right : XCoord; top : YCoord
);
    var x0, dx : XCoord;
        y0, dy : YCoord;
begin
    Translate(i, left, bottom);
    if
        (right - left # i^.width) or
	(top - bottom # i^.height)
    then
        i^.width := right - left;
	i^.height := top - bottom;
	dy := top - i^.top + i^.bottom - bottom;
	ChangeWorld(i, left, bottom, right, top - GetHeight() - 1);
	picture.Translate(i^.indicator, 0.0, float(dy));
	menu.Translate(i^.commands, 0, dy);
	menu.Translate(i^.tools, 0, dy);
	picture.DisableRedraw(i^.xscroll.scrollBar);
	picture.DisableRedraw(i^.yscroll.scrollBar);
	UpdateScrollBars(i);
    else
        picture.GetOrigin(i^.pict, x0, y0);
	dx := left - i^.left;
	dy := bottom - i^.bottom;
        picture.SetOrigin(i^.pict, x0 + dx, y0 + dy);
	picture.SetOrigin(i^.page, x0 + dx, y0 + dy);
    end;	

    i^.left := left;
    i^.bottom := bottom;
    i^.right := right;
    i^.top := top;
    
    dvboot.AlignDialogCenters(i^.world);
    HandleRedraw(i, left, bottom, right, top);    
    menu.Uncache(i^.commands);
    picture.EnableRedraw(i^.xscroll.scrollBar);
    picture.EnableRedraw(i^.yscroll.scrollBar);
end HandleChange;


(* public export *)
procedure HandleRedraw (
    i : DrawingView; 
    left : XCoord; bottom : YCoord; right : XCoord; top : YCoord
);
    var box, clip : Box;
begin
    RedrawBanner(i, left, bottom, right, top);
    picture.DrawInBoundingBox(i^.world, left, bottom, right, top);
    picture.DrawInBoundingBox(i^.indicator, left, bottom, right, top);
    menu.DrawInBoundingBox(i^.commands, left, bottom, right, top);
    menu.DrawInBoundingBox(i^.tools, left, bottom, right, top);
    menu.HighlightInBoundingBox(
	i^.stateInfo.curTool, left, bottom, right, top
    );
    if (i^.stateInfo.exposedCmd # menu.Menu(nil)) then
	menu.HighlightInBoundingBox(
	    i^.stateInfo.exposedCmd, left, bottom, right, top
	);
    end;
    picture.DrawInBoundingBox(i^.xscroll.scrollBar, left, bottom, right, top);
    picture.DrawInBoundingBox(i^.yscroll.scrollBar, left, bottom, right, top);

    if i^.pict # Picture(nil) then
	clip.x0 := left;
	clip.y0 := bottom;
	clip.x1 := right;
	clip.y1 := top;
        picture.ConvertToScreenCoord(i^.pict, clip.x0, clip.y0);
        picture.ConvertToScreenCoord(i^.pict, clip.x1, clip.y1);
	box := CalcPictClipBox(i^.pict, i);
	if 
	    IntersectBoxes(
	        box.x0, box.y0, box.x1, box.y1, 
		clip.x0, clip.y0, clip.x1, clip.y1
	    )
	then
            picture.DrawInBoundingBox(i^.pict, box.x0, box.y0, box.x1, box.y1);
            picture.DrawInBoundingBox(i^.page, box.x0, box.y0, box.x1, box.y1);
	    if i^.stateInfo.activity # TextEntering then
	        HighlightSelectionsInBbox(i, box.x0, box.y0, box.x1, box.y1);
 	    end;
	end;
    end;
    if i^.stateInfo.activity = Dialoging then
	dialog.DrawInBoundingBox(
	    i^.stateInfo.curDialog, left, bottom, right, top
	);
    end;
end HandleRedraw;


(* public export *)
procedure HandleSelect (i : DrawingView);
begin
    if not i^.selected then
	i^.selected := true;
	DrawBanner(i);
    end;
end HandleSelect;


(* public export *)
procedure HandleUnSelect (i : DrawingView);
begin
    if i^.selected then
	i^.selected := false;
	DrawBanner(i);
        UnQButton(MIDDLEBUTTON);
	UnQButton(RIGHTBUTTON);
    end;
end HandleUnSelect;


(* export *)
procedure MakeDirty(const i : DrawingView);
begin
    if not i^.stateInfo.dirty or infoShown then
        i^.stateInfo.dirty := true;
	DrawBanner(i);
    end;
end MakeDirty;


(* export *)
procedure MakeClean(const i : DrawingView);
begin
    if i^.stateInfo.dirty or infoShown then
	i^.stateInfo.dirty := false;
	DrawBanner(i);
    end;
end MakeClean;


(* export *)
procedure RedrawBanner (
    i : DrawingView;
    left : XCoord;
    bottom : YCoord;
    right : XCoord;
    top : YCoord
);
    var x0, x1 : XCoord;
        y0, y1 : YCoord;
begin
    picture.GetBoundingBox(i^.world, x0, y0, x1, y1);
    picture.ConvertToWorldCoord(i^.world, x1, y1);
    if IntersectBoxes(
         left, bottom, right, top, i^.left, y1, i^.right, i^.top
    ) then
	if infoShown then
	    DrawBanner(i);
	else
	    Writable(left, bottom, right, top);
	    DrawBanner(i);
	    AllWritable();
	end;
    end;
end RedrawBanner;


procedure InfoHandler(const w : Word);
    var defaultFont : Font;
        botBanner : YCoord;
	i : DrawingView;
begin
    i := DrawingView(w);
    SetPattern(clearPattern);
    SetRendering(PAINTFGBG);
    botBanner := i^.top - bannerHt;
    if i^.selected then
        SetColors(WHITE, BLACK);
        FilledRectangle(i^.left, botBanner, i^.right, i^.top);
    else
        SetColors(BLACK, WHITE);
        FilledRectangle(i^.left, botBanner, i^.right, i^.top);
        Rectangle(i^.left, botBanner, i^.right, i^.top);
    end;
    MoveTo(i^.left + 4, botBanner + 1);
    if not GetScreenFont(defaultFont, DEFAULTFONT, 10) then
        defaultFont := GetFont();
    end;
    SetFont(defaultFont);
    CharStr("idraw by John Vlissides.  ");
    CharStr(VERSION);
    CharStr(DATE);
    Sync();
    infoShown := true;
end InfoHandler;


(* export *)
procedure DrawBanner(const i : DrawingView);
    var defaultFont : Font;
        botBanner : YCoord;
begin
    SetPattern(clearPattern);
    SetRendering(PAINTFGBG);
    botBanner := i^.top - bannerHt;
    if i^.selected then
        SetColors(WHITE, BLACK);
        FilledRectangle(i^.left, botBanner, i^.right, i^.top);
    else
        SetColors(BLACK, WHITE);
        FilledRectangle(i^.left, botBanner, i^.right, i^.top);
        Rectangle(i^.left, botBanner, i^.right, i^.top);
    end;
    MoveTo(i^.left + 4, botBanner + 1);
    if not GetScreenFont(defaultFont, DEFAULTFONT, 10) then
        defaultFont := GetFont();
    end;
    SetFont(defaultFont);
    CharStr("drawing view   ");
    if i^.noFileSpecified then
        CharStr("[unnamed drawing]");
    else
	CharStr(i^.fileName);
    end;
    if i^.stateInfo.dirty then
        CharStr(" *");
    end;
    if i^.stateInfo.gridOn then
        CharStr("   grid on");
    end;
    if not i^.stateInfo.redrawOn then
        CharStr("   redraw off");
    end;
    if i^.bannermsg1[1] # NULL then
	CharStr("   ");
	CharStr(i^.bannermsg1);
    end;
    if i^.bannermsg2[1] # NULL then
	CharStr(i^.bannermsg2);
    end;
    infoShown := false;
    Sync();
end DrawBanner;


(* export *)
procedure BannerMsg(
    const view : DrawingView;
    const msg1 : array of char;
    const msg2 : array of char    
);
begin
    Assign(view^.bannermsg1, msg1);
    Assign(view^.bannermsg2, msg2);
    DrawBanner(view);
    view^.bannermsg1[1] := NULL;
    view^.bannermsg2[1] := NULL;
    infoShown := true;
end BannerMsg;


(* export *)
procedure EraseBannerMsg(const view : DrawingView);
begin
    view^.bannermsg1[1] := NULL;
    view^.bannermsg2[1] := NULL;
    DrawBanner(view);
end EraseBannerMsg;


begin
    infoShown := false;
    new(infoHandler);
    infoHandler^ := InfoHandler;
    cursorPos := 0;
end dvwindops.
