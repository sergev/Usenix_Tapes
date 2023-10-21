implementation module dvcmds;

import
    picture,
    menu,
    dialog,
    rubband,
    genlists,
    dvboot,
    strings;

(* export *)
from picture import
    Picture, XCoord, YCoord;

(* export *)
from dvdefs import
    DrawingView;

from dvdefs import
    LineType, StateType, Font, Pattern, AlignProc, Box, ModifOp,
    PRINTSUFFIX, OFFSETX, OFFSETY, PIXPERINCH, PAGESIZEX, MAXFILENAMESIZE,
    MAXPATHSIZE, CLIPBOARD, LEFTBUTTON, GRIDSIZE, MAXZOOMIN, MAXZOOMOUT,
    NULL, LINEFEED, CARRIAGERTN;

from dvselect import
    CreateRubberbands, HighlightSelections, DeleteRubberbands,
    RecalcRubberbands, QuickReleaseSelections, SelectAll,
    CutSelections, CopySelections, PasteSelections, DisableSelections,
    BringSelectionsToFront, SendSelectionsToBack,
    CleanupSelectionsForGrouping, Group, Ungroup, ShowStructure,
    TranslateSelections, ScaleSelections, RotateSelections;

from genlists import
    GenericList, Iterator, BeginIteration, MoreElements,
    EndIteration, GetElement, Head, Tail, Append, Insert, DeleteCur, Release;

from dvundo import
    ResetLog, LogMove, LogScale, LogRotate,
    LogInsertion, LogDeletion, LogAlignment, LogModification,
    LogSendToBack, LogBringToFront, LogGroup, LogUngroup, LogShowStructure;

from dvwindops import
    CopyIndicatorInfo, UpdateLineIndicator,
    DrawBanner, MakeDirty, MakeClean, BannerMsg, EraseBannerMsg;

(* export *)
from input import
    InputButton;

from input import
    defaultCursor, arrowCursor, hourglassCursor,
    GetCursor, SetCursor, CursorOn, CursorOff;

from rubband import
    Rubberband;

from dirs import
    chdir, getwd;

from dvscroll import
    UpdateScrollBars;

from dvupict import
    DrawInClipBox, RedrawInClipBox, DrawPageInClipBox, ClearAndDrawInClipBox,
    ClipToClipBox, CenterPage, CenterPointOnPage, CalcCenterPictClipBox,
    GetNearestGridPoint;

from dverror import
    ReportErrorBanner, PRINTERR;

from utilities import
    round;

from dvevents import
    GetDialogInput, GetFloatInputs, EraseDialog, QuickEraseDialog;

from vdi import
    AllWritable, WHITE, BLACK;

from ps import
    PictToPS;

from io import
    File, Open, Close;

(* export *)
from system import
    Word;

from math import
    log, ldexp;


type
    LineTypePtr = pointer to LineType;

var
    cancelled : boolean;


(* export *)
procedure NewHandler(const w : Word);
    var view : DrawingView;
        newpict : Picture;
	response : array [1..MAXFILENAMESIZE] of char;
begin
    view := DrawingView(w);
    if view^.stateInfo.dirty then
	GetDialogInput(view, dvboot.SAVECURDIALOG, response, false);
	if response[1] = "y" then
	    cancelled := false;
	    SaveHandler(w);
	    if cancelled then
		return;
	    end;
	elsif response[1] # "n" then
	    EraseDialog(view);
	    return;
	end;
	QuickEraseDialog(view);
    end;
    
    view^.stateInfo.dirty := false;
    view^.noFileSpecified := true;
    ResetLog(view);
    picture.Create(newpict);
    CopyIndicatorInfo(view^.pict, newpict);
    QuickReleaseSelections(view);
    picture.Destroy(view^.pict);
    view^.pict := newpict;
    picture.SetOrigin(view^.pict, view^.left, view^.bottom);
    picture.Destroy(view^.page);
    view^.page := dvboot.CreatePage();
    picture.SetOrigin(view^.page, view^.left, view^.bottom);
    view^.stateInfo.zoom := 0;
    view^.stateInfo.landscape := false;
    CenterPage(view);
    UpdateScrollBars(view);
    picture.Redraw(view^.xscroll.scrollBar);
    picture.Redraw(view^.yscroll.scrollBar);
    ClearAndDrawInClipBox(view^.pict, view);
    DrawBanner(view);
end NewHandler;


(* export *)
procedure OpenFile(
    const fileName : array of char;
    const view : DrawingView
) : boolean;
    var newpict : Picture;
	origCursor : integer;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
begin
    if fileName[0] = NULL then
	EraseDialog(view);
	return true;
    end;
    origCursor := GetCursor();
    SetCursor(hourglassCursor);
    if not picture.Read(newpict, fileName) or (newpict = Picture(nil)) then
	SetCursor(origCursor);
	return false;
    end;
    QuickEraseDialog(view);
    ResetLog(view);
    strings.Assign(view^.fileName, fileName);
    view^.noFileSpecified := false;
    MakeClean(view);
    CopyIndicatorInfo(view^.pict, newpict);
    QuickReleaseSelections(view);
    picture.Destroy(view^.pict);
    view^.pict := newpict;
    picture.SetOrigin(view^.pict, view^.left, view^.bottom);
    picture.Destroy(view^.page);
    view^.page := dvboot.CreatePage();
    picture.SetOrigin(view^.page, view^.left, view^.bottom);
    view^.stateInfo.zoom := 0;
    view^.stateInfo.landscape := false;
    picture.GetBoundingBox(view^.pict, x0, y0, x1, y1);
    if
        (abs(view^.pictClipBox.x1 - view^.pictClipBox.x0) < abs(x1 - x0)) or
	(abs(view^.pictClipBox.y1 - view^.pictClipBox.y0) < abs(y1 - y0))
    then
	ReduceToFitHandler(view);
    else
        CenterPage(view);
	UpdateScrollBars(view);
	picture.Redraw(view^.xscroll.scrollBar);
	picture.Redraw(view^.yscroll.scrollBar);
	ClearAndDrawInClipBox(view^.pict, view);
    end;
    SetCursor(origCursor);
    DrawBanner(view);
    return true;
end OpenFile;


(* export *)
procedure RevertHandler(const w : Word);
    var view : DrawingView;
	testName, response : array [1..MAXFILENAMESIZE] of char;
begin
    view := DrawingView(w);
    if not view^.noFileSpecified and view^.stateInfo.dirty then
	GetDialogInput(view, dvboot.REVERTDIALOG, response, false);
	if response[1] = "y" then
	    dvboot.AddDialogWarning(
		dvboot.OPENDIALOG, "couldn't revert!", " (file nonexistent?)"
	    );
	    strings.Assign(testName, view^.fileName);
	    loop
		if OpenFile(testName, view) then
		    dvboot.RemoveDialogWarning(dvboot.OPENDIALOG);
		    exit;
		else
		    GetDialogInput(view, dvboot.OPENDIALOG, testName, false);
		    dvboot.RemoveDialogWarning(dvboot.OPENDIALOG);
		    dvboot.AddDialogWarning(
			dvboot.OPENDIALOG, "couldn't open ", testName
		    );
		end;
	    end;
	else
	    EraseDialog(view);
	    return;
	end;
    end;
end RevertHandler;


(* export *)
procedure OpenHandler(const w : Word);
    var view : DrawingView;
	testName, response : array [1..MAXFILENAMESIZE] of char;
	dummy : boolean;
begin
    view := DrawingView(w);
    if view^.stateInfo.dirty then
	GetDialogInput(view, dvboot.SAVECURDIALOG, response, false);
	if response[1] = "y" then
	    cancelled := false;
	    SaveHandler(w);
	    if cancelled then
		return;
	    end;
	elsif response[1] # "n" then
	    EraseDialog(view);
	    return;
	end;
    end;
    loop
	GetDialogInput(view, dvboot.OPENDIALOG, testName, false);
	if OpenFile(testName, view) then
	    dvboot.RemoveDialogWarning(dvboot.OPENDIALOG);
	    exit;
	else
	    dvboot.AddDialogWarning(
		dvboot.OPENDIALOG, "couldn't open ", testName
	    );
	end;
    end;
end OpenHandler;


procedure SaveFile(
    const view : DrawingView;
    const fileName : array of char
) : boolean;
    var	pgx0, x0, x1 : XCoord;
	pgy0, y0, y1 : YCoord;
	zoomFactor, dx, dy : real;
	origCursor : integer;
	successful : boolean;
begin
    successful := true;
    if not view^.stateInfo.dirty then
        return successful;
    end;
    origCursor := GetCursor();
    SetCursor(hourglassCursor);
    pgx0 := round(OFFSETX * PIXPERINCH);
    pgy0 := round(OFFSETY * PIXPERINCH);
    picture.GetBoundingBox(view^.page, x0, y0, x1, y1);

    if (x0 # pgx0) or (y0 # pgy0) then
        zoomFactor := 1.0 / ldexp(1.0, view^.stateInfo.zoom);
	if zoomFactor = 1.0 then
	    dx := float(pgx0 - x0);
	    dy := float(pgy0 - y0);
	    picture.Translate(view^.pict, dx, dy);
	else
	    dx := (float(x0)*zoomFactor - float(pgx0)) / (zoomFactor - 1.0);
	    dy := (float(y0)*zoomFactor - float(pgy0)) / (zoomFactor - 1.0);
            picture.ScaleAboutPoint(view^.pict, zoomFactor, zoomFactor, dx, dy);
	end;
        if not picture.Write(view^.pict, fileName) then
	    successful := false;
	end;
	if zoomFactor = 1.0 then
	    picture.Translate(view^.pict, -dx, -dy);
	else
	    zoomFactor := 1.0 / zoomFactor;
            picture.ScaleAboutPoint(view^.pict, zoomFactor, zoomFactor, dx, dy);
	end;
    else
        if not picture.Write(view^.pict, fileName) then
	    successful := false;
	end;
    end;
    SetCursor(origCursor);
    if successful then
	MakeClean(view);
    else
	picture.Touch(view^.pict);
    end;
    return successful;
end SaveFile;


(* export *)
procedure SaveHandler(const w : Word);
    var view : DrawingView;
        dummy : boolean;
begin
    view := DrawingView(w);
    if view^.noFileSpecified then
        SaveAsHandler(w);
    else
	BannerMsg(view, "writing...", " ");
	if not SaveFile(view, view^.fileName) then
	    EraseBannerMsg(view);
	    dvboot.AddDialogWarning(
		dvboot.SAVEASDIALOG,
		"couldn't save this drawing as ", view^.fileName
	    );
	    SaveAsHandler(w);
	else
	    BannerMsg(view, "writing...", " done!");
	end;
    end;
end SaveHandler;


(* export *)
procedure SaveAsHandler(const w : Word);
    var view : DrawingView;
	testFile : File;
	testName, response : array [1..MAXFILENAMESIZE] of char;
	prevFileSpecStatus : boolean;
	warning : Picture;
begin
    loop
	view := DrawingView(w);
	GetDialogInput(view, dvboot.SAVEASDIALOG, testName, false);
	dvboot.RemoveDialogWarning(dvboot.SAVEASDIALOG);
	if testName[1] = NULL then
	    cancelled := true;
	    EraseDialog(view);
	    return;
	end;
	testFile := Open(testName, "r");
	if testFile # nil then
	    Close(testFile);
	    dvboot.AddDialogWarning(
		dvboot.OVERWRITEDIALOG, "a drawing named ", testName
	    );
	    GetDialogInput(view, dvboot.OVERWRITEDIALOG, response, false);
	    if response[1] # "y" then
		cancelled := true;
		EraseDialog(view);
		dvboot.RemoveDialogWarning(dvboot.OVERWRITEDIALOG);
		return;
	    else
		dvboot.RemoveDialogWarning(dvboot.OVERWRITEDIALOG);
	    end;
	end;
	prevFileSpecStatus := view^.noFileSpecified;
	view^.noFileSpecified := false;
	view^.stateInfo.dirty := true;
	strings.Assign(response, "writing ");
	strings.Append(response, testName);
	strings.Append(response, "...");
	BannerMsg(view, response, " ");
	if SaveFile(view, testName) then
	    strings.Assign(view^.fileName, testName);
	    BannerMsg(view, response, " done!");
	    EraseDialog(view);
	    exit;
	else
	    EraseBannerMsg(view);
	    view^.noFileSpecified := prevFileSpecStatus;
	    dvboot.AddDialogWarning(
		dvboot.SAVEASDIALOG, "couldn't save this drawing as ", testName
	    );
	end;
    end;
end SaveAsHandler;


(* export *)
procedure Print(
    const pictFile : array of char;
    const landscape : boolean
) : boolean;
    var offsetx, offsety, angle : real;
	printFile : array [1..MAXFILENAMESIZE + 3] of char;
begin
    strings.Assign(printFile, pictFile);
    strings.Append(printFile, ".ps");
    if landscape then
	offsetx := PIXPERINCH * (PAGESIZEX + OFFSETX + OFFSETY);
	offsety := PIXPERINCH * (OFFSETY - OFFSETX);
	angle := 90.0;
    else
        offsetx := 0.0;
	offsety := 0.0;
	angle := 0.0;
    end;
    
    return PictToPS(pictFile, printFile, offsetx, offsety, 1.0, 1.0, angle);
end Print;


(* export *)
procedure PrintHandler(const w : Word);
    var view : DrawingView;
	origCursor : integer;
	psfile : array [1..MAXFILENAMESIZE] of char;
	msg : array [1..MAXFILENAMESIZE] of char;
begin
    view := DrawingView(w);
    cancelled := false;
    if view^.stateInfo.dirty or view^.noFileSpecified then
        SaveHandler(w);
	if cancelled then
	    return;
	end;
    end;
    origCursor := GetCursor();
    SetCursor(hourglassCursor);
    strings.Assign(psfile, view^.fileName);
    strings.Append(psfile, PRINTSUFFIX);
    strings.Assign(msg, psfile);
    strings.Append(msg, "...");
    BannerMsg(view, "writing ", msg);
    if not Print(view^.fileName, view^.stateInfo.landscape) then
	ReportErrorBanner(view, PRINTERR, psfile);
    else
	strings.Append(msg, " done!");
	BannerMsg(view, "writing ", msg);
    end;
    SetCursor(origCursor);
end PrintHandler;


(* export *)
procedure DirHandler(const w : Word);
    var view : DrawingView;
	response: array [1..MAXPATHSIZE] of char;
begin
    view := DrawingView(w);
    dvboot.AddDialogWarning(dvboot.DIRDIALOG, view^.curPath, NULL);
    GetDialogInput(view, dvboot.DIRDIALOG, response, false);
    loop
	if response[1] = NULL then
	    exit;
	elsif (chdir(response) = 0) and (getwd(response) # 0) then
	    strings.Assign(view^.curPath, response);
	    BannerMsg(view, "pwd = ", response);
	    exit;
	else
	    dvboot.AddDialogWarning(
		dvboot.DIRERRDIALOG, "couldn't chdir to ", response
	    );
	    GetDialogInput(view, dvboot.DIRERRDIALOG, response, false);
	end;
    end;
    EraseDialog(view);
    dvboot.RemoveDialogWarning(dvboot.DIRERRDIALOG);
    dvboot.RemoveDialogWarning(dvboot.DIRDIALOG);
end DirHandler;


(* export *)
procedure QuitHandler(const w : Word);
    var view : DrawingView;
        newpict : Picture;
	response : array [1..MAXFILENAMESIZE] of char;
begin
    view := DrawingView(w);
    if view^.stateInfo.dirty then
	GetDialogInput(view, dvboot.SAVECURDIALOG, response, false);
	if response[1] = "y" then
	    cancelled := false;
	    SaveHandler(w);
	    if cancelled then
		return;
	    end;
	elsif response[1] # "n" then
	    EraseDialog(view);
	    return;
	end;
    end;
    view^.quit := true;
end QuitHandler;


(* export *)
procedure CutHandler(const w : Word);
    var view : DrawingView;
        origCursor : integer;
begin
    view := DrawingView(w);
    origCursor := GetCursor();
    SetCursor(hourglassCursor);
    ResetLog(view);
    if CutSelections(view, CLIPBOARD) then
	if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	    MakeDirty(view);
	end;
	LogDeletion(view);
	genlists.Create(view^.stateInfo.selecList);
	RedrawInClipBox(view^.pict, view);
	DrawPageInClipBox(view);
    end;
    SetCursor(origCursor);
end CutHandler;


(* export *)
procedure CopyHandler(const w : Word);
    var view : DrawingView;
        origCursor : integer;
	dummy : boolean;
begin
    view := DrawingView(w);
    origCursor := GetCursor();
    SetCursor(hourglassCursor);
    ResetLog(view);
    dummy := CopySelections(view, CLIPBOARD);
    SetCursor(origCursor);
end CopyHandler;


(* export *)
procedure PasteHandler(const w : Word);
    var view : DrawingView;
        origCursor : integer;
	i : Iterator;
	s : Picture;
begin
    view := DrawingView(w);
    origCursor := GetCursor();
    SetCursor(hourglassCursor);
    if PasteSelections(view, CLIPBOARD) then
	if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	    MakeDirty(view);
	end;
	i := BeginIteration(view^.stateInfo.selecList);
	while MoreElements(i, s) do
	    DrawInClipBox(s, view);
	end;
	EndIteration(i);
	DrawPageInClipBox(view);
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
	LogInsertion(view);
    end;
    SetCursor(origCursor);
end PasteHandler;


(* export *)
procedure DeleteHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
        MakeDirty(view);
        ClipToClipBox(view);
	DeleteRubberbands(view);
        AllWritable();
	DisableSelections(view);
	LogDeletion(view);
	genlists.Create(view^.stateInfo.selecList);
        RedrawInClipBox(view^.pict, view);
        DrawPageInClipBox(view);
    end;
end DeleteHandler;


(* export *)
procedure DupHandler(const w : Word);
    var view : DrawingView;
        copy, s : Picture;
	i : Iterator;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
	newSelecList : GenericList;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
        MakeDirty(view);
	ResetLog(view);
	genlists.Create(newSelecList);
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
	if not view^.stateInfo.selecSorted then
	    picture.OrderSelections(view^.stateInfo.selecList, view^.pict);
	    view^.stateInfo.selecSorted := true;
	end;
	i := BeginIteration(view^.stateInfo.selecList);
	while MoreElements(i, s) do
	    DeleteCur(i);
	    copy := picture.Copy(s);
	    picture.Translate(copy, float(GRIDSIZE), float(GRIDSIZE));
	    picture.Nest(copy, view^.pict);	
	    DrawInClipBox(copy, view);
	    Append(copy, newSelecList);
	end;
	EndIteration(i);
	Release(view^.stateInfo.selecList);
	view^.stateInfo.selecList := newSelecList;
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
	LogInsertion(view);
	DrawPageInClipBox(view);
    end;
end DupHandler;


(* export *)
procedure SelectAllHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    ClipToClipBox(view);
    SelectAll(view);
    AllWritable();
    view^.stateInfo.selecSorted := true;
end SelectAllHandler;


(* export *)
procedure GroupHandler(const w : Word);
    var view : DrawingView;
	headSelecList, head, parent : Picture;
	i : Iterator;
	found : boolean;
begin
    view := DrawingView(w);
    ClipToClipBox(view);
    DeleteRubberbands(view);
    AllWritable();
    ResetLog(view);
    picture.Create(parent);
    CleanupSelectionsForGrouping(view, parent);
    if parent # Picture(nil) then
	MakeDirty(view);
	LogGroup(view);
	Group(view, parent);
    end;
    CreateRubberbands(view);
    ClipToClipBox(view);
    HighlightSelections(view);
    AllWritable();
end GroupHandler;


(* export *)
procedure UngroupHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);
        ClipToClipBox(view);
        DeleteRubberbands(view);
	LogUngroup(view);
	Ungroup(view);
        CreateRubberbands(view);
        HighlightSelections(view);
        AllWritable();
    end;
end UngroupHandler;


(* export *)
procedure ShowStructHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
        ClipToClipBox(view);
        DeleteRubberbands(view);
	LogShowStructure(view);
	ShowStructure(view);	(* creates its own rubberbands *)
        HighlightSelections(view);
        AllWritable();
    end;
end ShowStructHandler;


(* export *)
procedure BringToFrontHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
	ResetLog(view);
	if not view^.stateInfo.selecSorted then
	    picture.OrderSelections(view^.stateInfo.selecList, view^.pict);
	    view^.stateInfo.selecSorted := true;
	end;
	LogBringToFront(view);
	BringSelectionsToFront(view);
	DrawPageInClipBox(view);
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
    end;
end BringToFrontHandler;


(* export *)
procedure SendToBackHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
	ResetLog(view);
	if not view^.stateInfo.selecSorted then
	    picture.OrderSelections(view^.stateInfo.selecList, view^.pict);
	    view^.stateInfo.selecSorted := true;
	end;
	LogSendToBack(view);
	SendSelectionsToBack(view);
	DrawPageInClipBox(view);
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
    end;
end SendToBackHandler;


(* export *)
procedure HflipHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
	ScaleSelections(view, -1.0, 1.0);
	RedrawInClipBox(view^.pict, view);
	DrawPageInClipBox(view);
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
	LogScale(view, -1.0, 1.0);
    end;
end HflipHandler;


(* export *)
procedure VflipHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
	ScaleSelections(view, 1.0, -1.0);
	RedrawInClipBox(view^.pict, view);
	DrawPageInClipBox(view);
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
	LogScale(view, 1.0, -1.0);
    end;
end VflipHandler;


(* export *)
procedure CW90Handler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
	RotateSelections(view, -90.0);
	RedrawInClipBox(view^.pict, view);
	DrawPageInClipBox(view);
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
	LogRotate(view, -90.0);
    end;
end CW90Handler;


(* export *)
procedure CCW90Handler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
	RotateSelections(view, 90.0);
	RedrawInClipBox(view^.pict, view);
	DrawPageInClipBox(view);
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
	LogRotate(view, 90.0);
    end;
end CCW90Handler;


(* export *)
procedure PrecMoveHandler(const w : Word);
    var view : DrawingView;
        x, y : real;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	if
	    (GetFloatInputs(view, dvboot.PRECMOVEDIALOG, x, y, true) > 1) and
	    ((x # 0.0) or (y # 0.0))
	then
	    MakeDirty(view);
	    TranslateSelections(view, x, y);
	    ClipToClipBox(view);
	    DeleteRubberbands(view);
	    AllWritable();
	    RedrawInClipBox(view^.pict, view);
	    DrawPageInClipBox(view);
	    ClipToClipBox(view);
	    CreateRubberbands(view);
	    HighlightSelections(view);
	    AllWritable();
	    LogMove(view, x, y);
	end;
    end;
end PrecMoveHandler;


(* export *)
procedure PrecScaleHandler(const w : Word);
    var view : DrawingView;
        x, y : real;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	if
	    (GetFloatInputs(view, dvboot.PRECSCALEDIALOG, x, y, true) > 1) and
	    ((x # 1.0) or (y # 1.0)) and (x*y # 0.0)
	then
	    MakeDirty(view);
	    ScaleSelections(view, x, y);
	    ClipToClipBox(view);
	    DeleteRubberbands(view);
	    AllWritable();
	    RedrawInClipBox(view^.pict, view);
	    DrawPageInClipBox(view);
	    ClipToClipBox(view);
	    CreateRubberbands(view);
	    HighlightSelections(view);
	    AllWritable();
	    LogScale(view, x, y);
	end;
    end;
end PrecScaleHandler;


(* export *)
procedure PrecRotHandler(const w : Word);
    var view : DrawingView;
        rotation, dummy : real;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	if
	    (
		GetFloatInputs(
		    view, dvboot.PRECROTDIALOG, rotation, dummy, true
		) # 0
	    ) and (rotation # 0.0)
	then
	    MakeDirty(view);
	    RotateSelections(view, rotation);
	    ClipToClipBox(view);
	    DeleteRubberbands(view);
	    AllWritable();
	    RedrawInClipBox(view^.pict, view);
	    DrawPageInClipBox(view);
	    ClipToClipBox(view);
	    CreateRubberbands(view);
	    HighlightSelections(view);
	    AllWritable();
	    LogRotate(view, rotation);
	end;
    end;
end PrecRotHandler;


(* export *)
procedure FontHandler(const w : Word);
    var view : DrawingView;
        font : Font;
	i : Iterator;
	s : Picture;
	alteredAny : boolean;
begin
    view := DrawingView(w);
    font := Font(menu.GetUserData(view^.stateInfo.curCmdEntry));
    picture.DefaultFont(view^.pict, font^);

    alteredAny := false;
    ClipToClipBox(view);
    HighlightSelections(view);	(* erase rubberbands *)
    AllWritable();
    ResetLog(view);
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        alteredAny := true;
	LogModification(view, FontChange, s);
        picture.AlterFont(s, font^);
    end;
    EndIteration(i);
    if alteredAny then
	MakeDirty(view);
	RedrawInClipBox(view^.pict, view);
	DrawPageInClipBox(view);
    end;
    ClipToClipBox(view);
    RecalcRubberbands(view);
    HighlightSelections(view);
    AllWritable();
end FontHandler;


(* export *)
procedure LineStyleHandler(const w : Word);
    var view : DrawingView;
	lineType : LineTypePtr;
begin
    view := DrawingView(w);
    lineType := LineTypePtr(
        menu.GetUserData(view^.stateInfo.curCmdEntry)
    );
    view^.stateInfo.lineType := lineType^;
    UpdateLineIndicator(view);
    picture.Draw(view^.indicator);
end LineStyleHandler;


(* export *)
procedure NonePatHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    picture.AlterRenderingStyle(view^.patIndic, picture.Stroke);
    picture.Enable(picture.GetNestedPicture(view^.patIndic, -1));
    picture.Draw(view^.indicator);
    picture.DefaultRenderingStyle(view^.pict, picture.Stroke);
end NonePatHandler;


(* export *)
procedure PatHandler(const w : Word);
    var view : DrawingView;
        pattern : Pattern;
	i : Iterator;
	s : Picture;
	alteredAny : boolean;
begin
    view := DrawingView(w);
    pattern := Pattern(
        menu.GetUserData(view^.stateInfo.curCmdEntry)
    );
    picture.AlterRenderingStyle(view^.patIndic, picture.Fill);
    picture.AlterPattern(view^.patIndic, pattern^);
    picture.Disable(picture.GetNestedPicture(view^.patIndic, -1));
    picture.Draw(view^.patIndic);
    picture.DefaultRenderingStyle(view^.pict, picture.Fill);
    picture.DefaultPattern(view^.pict, pattern^);
    
    alteredAny := false;
    ClipToClipBox(view);
    HighlightSelections(view);	(* erase rubberbands *)
    AllWritable();
    ResetLog(view);
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        alteredAny := true;
	LogModification(view, PatternChange, s);
	picture.AlterPattern(s, pattern^);
    end;
    EndIteration(i);
    if alteredAny then
	MakeDirty(view);
	RedrawInClipBox(view^.pict, view);
	DrawPageInClipBox(view);
    end;
    ClipToClipBox(view);
    HighlightSelections(view);
    AllWritable();
end PatHandler;


(* export *)
procedure AlignHandler(const w : Word);
    var view : DrawingView;
        i : Iterator;
	fixedPict, s : Picture;
	alignProc : AlignProc;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);
	ResetLog(view);
	i := BeginIteration(view^.stateInfo.selecList);
        if MoreElements(i, fixedPict) then
	    LogAlignment(view, 0.0, 0.0);
	    while MoreElements(i, s) do
	        alignProc := AlignProc(
	            menu.GetUserData(view^.stateInfo.curCmdEntry)
	        );
	        alignProc^(fixedPict, s);
	        fixedPict := s;
		LogAlignment(view, picture.aligndx, picture.aligndy);
		    (* gross hack; see picture "align" module *)
	    end;
	    ClipToClipBox(view);
	    DeleteRubberbands(view);
	    CreateRubberbands(view);	
	    AllWritable();
	    RedrawInClipBox(view^.pict, view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
        end;
        EndIteration(i);
    end;
end AlignHandler;


(* export *)
procedure AlignToGridHandler(const w : Word);
    var view : DrawingView;
	i : Iterator;
        s : Picture;
	x0, x1, dx, origx : XCoord;
	y0, y1, dy, origy : YCoord;
begin
    view := DrawingView(w);
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	MakeDirty(view);    
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();

	ResetLog(view);
	picture.GetOrigin(view^.pict, origx, origy);

        i := BeginIteration(view^.stateInfo.selecList);
        while MoreElements(i, s) do
            picture.GetBoundingBox(s, x0, y0, x1, y1);
	    GetNearestGridPoint(view, float(x0), float(y0), dx, dy);
	    dx := (dx - x0) mod GRIDSIZE;
	    dy := (dy - y0) mod GRIDSIZE;
            picture.Translate(s, float(dx), float(dy));
	    LogAlignment(view, float(dx), float(dy));
        end;
        EndIteration(i);

	RedrawInClipBox(view^.pict, view);
	DrawPageInClipBox(view);
	ClipToClipBox(view);
	CreateRubberbands(view);
	HighlightSelections(view);
	AllWritable();
    end;
end AlignToGridHandler;


(* export *)
procedure Zoom(
    const view : DrawingView;
    const factor : real
);
    var x, y : real;
        gpx : XCoord;
	gpy : YCoord;
begin
    CalcCenterPictClipBox(view, x, y);
    GetNearestGridPoint(view, x, y, gpx, gpy);
    picture.ScaleAboutPoint(view^.pict, factor, factor, float(gpx), float(gpy));
    picture.ScaleAboutPoint(view^.page, factor, factor, float(gpx), float(gpy));
    UpdateScrollBars(view);
    picture.Redraw(view^.xscroll.scrollBar);
    picture.Redraw(view^.yscroll.scrollBar);
    RecalcRubberbands(view);
    ClearAndDrawInClipBox(view^.pict, view);
end Zoom;


(* export *)
procedure TwoXHandler(const w : Word);
    var view : DrawingView;
        origCursor : integer;
begin
    view := DrawingView(w);
    if view^.stateInfo.zoom < MAXZOOMIN then
        origCursor := GetCursor();
        SetCursor(hourglassCursor);
        Zoom(view, 2.0);
        view^.stateInfo.zoom := view^.stateInfo.zoom + 1;
        SetCursor(origCursor);
    end;
end TwoXHandler;


(* export *)
procedure HalfXHandler(const w : Word);
    var view : DrawingView;
        origCursor : integer;
begin
    view := DrawingView(w);
    if view^.stateInfo.zoom > MAXZOOMOUT then
        origCursor := GetCursor();
        SetCursor(hourglassCursor);
        Zoom(view, 0.5);
        view^.stateInfo.zoom := view^.stateInfo.zoom - 1;
        SetCursor(origCursor);
    end;
end HalfXHandler;


(* export *)
procedure NormalSizeHandler(const w : Word);
    var view : DrawingView;
        origCursor : integer;
begin
    view := DrawingView(w);
    origCursor := GetCursor();
    if view^.stateInfo.zoom # 0 then
        SetCursor(hourglassCursor);
        Zoom(view, ldexp(1.0, -view^.stateInfo.zoom));
        view^.stateInfo.zoom := 0;
        SetCursor(origCursor);
    end;
end NormalSizeHandler;


(* export *)
procedure ReduceToFitHandler(const w : Word);
    var view : DrawingView;
        origCursor, power2 : integer;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
	reducFactor : real;
begin
    view := DrawingView(w);
    if view^.stateInfo.zoom > MAXZOOMOUT then
        origCursor := GetCursor();
        SetCursor(hourglassCursor);
	picture.GetBoundingBox(view^.page, x0, y0, x1, y1);
	reducFactor := min(
	    float(abs(view^.pictClipBox.x1 - view^.pictClipBox.x0)) / 
	    float(abs(x1 - x0)),
	    float(abs(view^.pictClipBox.y1 - view^.pictClipBox.y0)) / 
	    float(abs(y1 - y0))
	);	    
	if reducFactor < 1.0 then
	    power2 := trunc(log(reducFactor) / log(2.0) - 1.0);
	    power2 := max(MAXZOOMOUT - view^.stateInfo.zoom, power2);
	    CenterPage(view);
	    Zoom(view, ldexp(1.0, power2));
	    view^.stateInfo.zoom := view^.stateInfo.zoom + power2;
	else
            CenterPage(view);
	    UpdateScrollBars(view);
	    picture.Redraw(view^.xscroll.scrollBar);
	    picture.Redraw(view^.yscroll.scrollBar);
	    ClearAndDrawInClipBox(view^.pict, view);
	end;
        SetCursor(origCursor);
    end;
end ReduceToFitHandler;


(* export *)
procedure CenterPageHandler(const w : Word);
    var view : DrawingView;
        origCursor : integer;
begin
    view := DrawingView(w);
    origCursor := GetCursor();
    SetCursor(hourglassCursor);
    CenterPage(view);
    UpdateScrollBars(view);
    picture.Redraw(view^.xscroll.scrollBar);
    picture.Redraw(view^.yscroll.scrollBar);
    ClearAndDrawInClipBox(view^.pict, view);
    SetCursor(origCursor);
end CenterPageHandler;


(* export *)
procedure GridHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    view^.stateInfo.gridOn := not view^.stateInfo.gridOn;
    DrawBanner(view);
end GridHandler;


(* export *)
procedure RedrawHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    if view^.stateInfo.redrawOn then
        picture.DisableRedraw(view^.pict);
    else
        picture.EnableRedraw(view^.pict);
	ClearAndDrawInClipBox(view^.pict, view);
    end;
    view^.stateInfo.redrawOn := not view^.stateInfo.redrawOn;
    DrawBanner(view);
end RedrawHandler;


(* export *)
procedure OrientationHandler(const w : Word);
    var view : DrawingView;
        x0, x1 : XCoord;
	y0, y1 : YCoord;
begin
    view := DrawingView(w);
    MakeDirty(view);
    picture.GetBoundingBox(view^.page, x0, y0, x1, y1);
    if view^.stateInfo.landscape then
	picture.Translate(view^.page, 0.0, float(y0 - y1));
        picture.RotateAboutPoint(view^.page, 90.0, float(x0), float(y0));
    else
        picture.Translate(view^.page, float(x0 - x1), 0.0);
	picture.RotateAboutPoint(view^.page, -90.0, float(x0), float(y0));
    end;
    view^.stateInfo.landscape := not view^.stateInfo.landscape;
    UpdateScrollBars(view);
    picture.Redraw(view^.xscroll.scrollBar);
    picture.Redraw(view^.yscroll.scrollBar);
    ClearAndDrawInClipBox(view^.pict, view);
end OrientationHandler;


(* export *)
procedure CachingHandler(const w : Word);
    var view : DrawingView;
begin
    view := DrawingView(w);
    menu.SetDestructive(view^.commands, not menu.Destructive(view^.commands));
end CachingHandler;

begin
    cancelled := false;
end dvcmds.
