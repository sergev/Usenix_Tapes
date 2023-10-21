(*
 *  DrawingView main program.
 *)
module dvmain;

import
    picture;

from drawing import
    Drawing;

from drawingView import
    DrawingView, OpenDrawingView, HandlePlace;

from dvdefs import
    LEFTBUTTON, MIDDLEBUTTON, RIGHTBUTTON, LARROW, RARROW, UARROW, DARROW,
    SMALLWIDTH, SMALLHEIGHT, LARGEWIDTH, LARGEHEIGHT, MAXFILENAMESIZE, NULL,
    PRINTSUFFIX, DEFAULTFONT;

from dvcmds import
    Print, OpenFile;

from dvboot import
    LoadFonts, CreateShades, OPENOP, AddDialogWarning, RemoveDialogWarning,
    OPENDIALOG, DEFAULTFONTUID;

from ps import
    MapFont;

from dverror import
    ReportError, READFILEERR, WRITEFILEERR, PRINTERR;

from dvevents import
    SplitEvent, KeyOpHandler;

from dvwindops import
    Destroy;

from vdi import
    Initialize, InitSize, InitPlace, Finishup, GetFont,
    XCoord, YCoord, XMAXSCREEN, YMAXSCREEN;

from parameters import
    NumParameters, GetParameter;

from strings import
    Assign, Append;

from cstrings import
    strcmp, strcmpn, strlen;

from io import
    Writef, terminal, SReadf;

from unix import
    uexit;

from input import
    InputEvent, QMotion, QCharacter, QButton, UnQButton, EventType, QRead;

from xstats import
    XRegister;

var
    initPrt : boolean;
    proceed : boolean;
    placed : boolean;
    shaped : boolean;
    left, top, width, height : integer;
    drawing : Drawing;
    v : DrawingView;
    ev : InputEvent;
    fileToOpen : array [1..MAXFILENAMESIZE] of char;


procedure SetDefaults ();
begin
    initPrt := false;
    proceed := true;
    placed := false;
    shaped := false;
    width := SMALLWIDTH;
    height := SMALLHEIGHT;
    fileToOpen[1] := NULL;
end SetDefaults;

procedure GetNextParam (
    var i : integer; 
    var p : array of char;
    var l : integer
);
begin
    if i >= NumParameters then
	Usage();
    end;
    GetParameter(i, p, l);
    i := i + 1;
end GetNextParam;


procedure Usage ();
begin
    Writef(terminal, "Usage: idraw <file>\n");
    Writef(terminal, "             [size=<width>,<height>]\n");
    Writef(terminal, "             [pos=<left>,<top>]\n");
    Writef(terminal, "             [-s] (small window)\n");
    Writef(terminal, "             [-l] (large window)\n");
    Writef(terminal, "             [-p <file>] ");
    Writef(terminal, "(convert to PostScript, portrait)\n");
    Writef(terminal, "             [-r <file>] ");
    Writef(terminal, "(convert to PostScript, landscape)\n");
    uexit(1);
end Usage;


procedure InitPrinting();
    var font : integer;
begin
    if not initPrt then
	initPrt := true;
	InitPlace(0, 0, 1, 1);
	(*
	 * This stuff is needed to get stuff initialized just as they
	 * do when a view is created.  Sigh.
	 *)
	font := GetFont();
	MapFont(font, DEFAULTFONT, 10, 0.0, 2.5);
	picture.MapFont(font, DEFAULTFONTUID);
	LoadFonts();
	CreateShades();
    end;
end InitPrinting;


procedure ProcessArgs ();
var i, len : integer;
    p : array [1..100] of char;
    fileName : array [1..MAXFILENAMESIZE] of char;
begin
    i := 1;
    while i < NumParameters do
	GetNextParam(i, p, len);
	if Equal(p, "-s") then
	    shaped := true;
	    (* default width/height *)
	elsif Equal(p, "-l") then
	    shaped := true;
	    width := LARGEWIDTH;
	    height := LARGEHEIGHT;
	elsif Prefix(p, "-p") then
	    InitPrinting();
	    GetNextParam(i, fileName, len);
	    if not Print(fileName, false) then
		Append(fileName, PRINTSUFFIX);
		ReportError("idraw", PRINTERR, fileName);
	    end;
	    proceed := false;
	elsif Prefix(p, "-r") then
	    InitPrinting();
	    GetNextParam(i, fileName, len);
	    if not Print(fileName, false) then
		Append(fileName, PRINTSUFFIX);
		ReportError("idraw", PRINTERR, fileName);
	    end;
	    proceed := false;
	elsif Prefix(p, "size=") then
	    shaped := true;
	    GetPair(p, 5, len, width, height, "size");
	elsif Prefix(p, "pos=") then
	    placed := true;
	    GetPair(p, 4, len, left, top, "pos");
	elsif p[1] = '-' then
	    Usage();
	else
	    Assign(fileToOpen, p);
	end;
    end;
end ProcessArgs;


procedure GetPair (
    const buf : array of char; start, len : integer;
    var x, y : integer; const name : array of char
);
var b : array [1..100] of char;
    v1, v2 : integer;
begin
    Substr(buf, start, len, b);
    if SReadf(b, "%d,%d", v1, v2) = 2 then
	x := v1;
	y := v2;
    else
	Writef(terminal, "bad value for %s\n", name);
	Usage();
    end;
end GetPair;


procedure Substr (
    const inString : array of char; start, len : integer;
    var outString : array of char
);
var i, j : integer;
begin
    j := 0;
    for i := start to len do
	outString[j] := inString[i];
	j := j + 1;
    end;
    outString[j] := '\0';
end Substr;


procedure Equal (const s1, s2 : array of char) : boolean;
begin
    return strcmp(s1, s2) = 0;
end Equal;


procedure Prefix (const s1, s2 : array of char) : boolean;
begin
    return strcmpn(s1, s2, strlen(s2)) = 0;
end Prefix;


begin	(* main *)
    SetDefaults();
    ProcessArgs();
    if not proceed then
        return;
    end;

    if placed then
	InitPlace(left, top, width + 1, height + 1);
    elsif shaped then
        InitSize(width + 1, height + 1);
    else
        Initialize();
	width := XMAXSCREEN - 1;
	height := YMAXSCREEN - 1;
    end;

    QMotion();
    QCharacter();
    UnQButton(LEFTBUTTON);
    UnQButton(MIDDLEBUTTON);
    UnQButton(RIGHTBUTTON);
    UnQButton(LARROW);
    UnQButton(RARROW);
    UnQButton(UARROW);
    UnQButton(DARROW);

    new(drawing);
    OpenDrawingView(drawing, v);
    HandlePlace(v, 0, 0, width, height);

    if (fileToOpen[1] # NULL) then
	 if not OpenFile(fileToOpen, v) then
	    AddDialogWarning(OPENDIALOG, "couldn't open ", fileToOpen);
	    KeyOpHandler(v, OPENOP);
	    RemoveDialogWarning(OPENDIALOG);
	end;
    end;

    QButton(LEFTBUTTON);
    QButton(MIDDLEBUTTON);
    QButton(RIGHTBUTTON);
    XRegister("idraw");
    repeat
        QRead(ev);
	SplitEvent(v, ev);
    until v^.quit;

    Destroy(v);
    Finishup();
end dvmain.
