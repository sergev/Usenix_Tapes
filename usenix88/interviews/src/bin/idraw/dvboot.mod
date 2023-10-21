implementation module dvboot;

import 
    picture,
    dvdefs,
    menu,
    dialog,
    scrollbar,
    vdi;

from system import
    Word;

from dvdefs import
    PIXPERINCH, PAGESIZEX, PAGESIZEY, OFFSETX, OFFSETY, DEFAULTFONT,
    LineType, Cursor, AlignProc, NULL;

(* export *)
from dvundo import
    UndoHandler;

(* export *)
from dvcmds import
    NewHandler, RevertHandler, OpenHandler, SaveHandler, SaveAsHandler, 
    PrintHandler, DirHandler, QuitHandler, 
    CutHandler, CopyHandler, PasteHandler, DeleteHandler,
    DupHandler, SelectAllHandler, GroupHandler, UngroupHandler, 
    ShowStructHandler, BringToFrontHandler, SendToBackHandler,
    HflipHandler, VflipHandler, CW90Handler, CCW90Handler,
    PrecMoveHandler, PrecScaleHandler, PrecRotHandler,
    FontHandler, LineStyleHandler, NonePatHandler, PatHandler, AlignHandler,
    AlignToGridHandler,
    TwoXHandler, HalfXHandler, NormalSizeHandler, ReduceToFitHandler,
    CenterPageHandler, GridHandler, OrientationHandler, RedrawHandler,
    CachingHandler;

from dvwindops import
    AdvCursor;

(* export *)
from dvoptools import
    SelectorHandler, MoveHandler, ScaleHandler, StretchHandler,
    RotateHandler, MagnifyHandler;
    
(* export *)
from dvobjtools import
    TextHandler, PerpLineHandler,
    LineHandler, CircleHandler, EllipseHandler, RectHandler,
    PolygonHandler, SplineHandler, ClosedSplineHandler;
    
from dvevents import
    MapKeyOp;

from pictmacros import
    ArrowHead;

from ps import
    MapFont, GetScreenFont;

(* export *)
from picture import
    XCoord,
    YCoord,
    Picture,
    RenderingStyle;

(* export *)
from menu import
    Menu,
    MenuEntryHandler;

(* export *)
from dialog import
    Dialog;

from grays import
    MakePattern;

from utilities import
    round;

from vdi import
    Font, GetFont;

(* export *)
from vdi import
    Pattern;

from input import
    arrowCursor, crossHairsCursor, ltextCursor;


const
    ORIGINX	     =   0;
    ORIGINY	     =   0;
    TOOLWIDTH        =  58;
    TOOLHEIGHT       =  35;
    CMDHEIGHT        =  25;
    CMDENTRYHEIGHT   =  16;
    DEFAULTCMDWIDTH  =  50;
    CMDMARGINS       =  14;
    SCROLLBARWIDTH   =  15;
    MENUMARGIN	     =  10;
    NGRAYS	     =  15;
    DIALOGWIDTH	     = 350;
    DIALOGHEIGHT     = 100;

(*
 *  Keyboard equivalents.
 *)
    SELECTTOOLOP 	= "s";
    MOVETOOLOP 		= "m";
    SCALETOOLOP 	= "j";
    STRETCHTOOLOP	= ";";
    ROTATETOOLOP 	= "k";
    MAGNIFYTOOLOP 	= "z";
    TEXTTOOLOP 		= "t";
    PERPLINETOOLOP 	= "p";
    LINETOOLOP 		= "l";
    CIRCLETOOLOP 	= "o";
    ELLIPSETOOLOP	= "q";
    RECTTOOLOP 		= "r";
    POLYGONTOOLOP 	= "w";
    SPLINETOOLOP	= "h";
    CLOSEDSPLINETOOLOP	= "y";
    
    NEWOP 	= 16C;	    NEWCODE	= "^n";
    REVERTOP	= 22C;	    REVERTCODE	= "^r";

(* export const  OPENOP	= 17C; *)
			    OPENCODE	= "^o";

    SAVEOP 	= 23C;	    SAVECODE	= "^s";
    SAVEASOP	= 1C;	    SAVEASCODE	= "^a";
    PRINTOP 	= 20C;	    PRINTCODE   = "^p";
    DIROP	= "D";
    QUITOP 	= 21C;	    QUITCODE	= "^q";
    
    UNDOOP	= "U";
    CUTOP 	= "x";
    COPYOP 	= "c";
    PASTEOP 	= "v";
    DUPOP	= "d";
    DELETEOP 	= 4C;	    DELETECODE	= "^d";
    SELECTALLOP = "a";
    GROUPOP 	= "g";
    UNGROUPOP 	= 25C;	    UNGROUPCODE = "^u";
    SHOWSTRUCTOP= "u";	    SHOWSTRUCTCODE = "u";
    BRINGTOFRONTOP = "f";
    SENDTOBACKOP = "b";
    HFLIPOP 	= "_";
    VFLIPOP 	= "|";
    CW90OP 	= "]";
    CCW90OP 	= "[";
    PRECMOVEOP  = "M";
    PRECSCALEOP = "J";
    PRECROTOP   = "K";
    
    ALNLEFTOP 	= "1";
    ALNRIGHTOP 	= "2";
    ALNBOTOP 	= "3";
    ALNTOPOP 	= "4";
    ALNVERTCTROP = "5";
    ALNHORIZCTROP = "6";
    ALNCTROP 	= "7";
    ALNLTOROP 	= "8";
    ALNRTOLOP 	= "9";
    ALNBTOTOP 	= "0";
    ALNTTOBOP 	= "-";
    ALNTOGRIDOP = ".";
    
    REDUCEOP 	= "i";
    ENLARGEOP 	= "e";
    NORMSIZEOP	 = "n";
    REDUCETOFITOP = "=";
    CENTERPAGEOP = "/";
    GRIDONOFFOP = ",";
    ORIENTATIONOP = "+";
    REDRAWOP = "R";
    CACHINGOP = "C";
    
    F6x10 = 37;		(* special font uid's to maintain compatibility	*)
    F6x13p = 25;	(* with existing idraw files			*)
(* export const DEFAULTFONTUID = 25; *)
    VTBOLD = 26;
    HELV10B = 27;
    HELV12 = 28;
    HELV12B = 29;
    HELV12I = 30;
    TIMROM10 = 32;
    TIMROM12 = 33;
    TIMROM12B = 34;
    TIMROM12I = 35;


(* export const NONE		=  1; *) (* indic subpict nesting levels *)
(* export const LINE		=  2; *)
(* export const LEFTARROW	=  3; *)
(* export const RIGHTARROW	=  4; *)

(* export const ARROWWIDTH	=  4; *) (* arrow dimensions used throughout *)
(* export const ARROWHEIGHT     =  8; *)

(* export const SAVEASDIALOG	=  1; *)
(* export const OPENDIALOG	=  2; *)
(* export const PRECMOVEDIALOG	=  3; *)
(* export const PRECSCALEDIALOG	=  4; *)
(* export const PRECROTDIALOG	=  5; *)
(* export const SAVECURDIALOG	=  6; *)
(* export const OVERWRITEDIALOG	=  7; *)
(* export const REVERTDIALOG	=  8; *)
(* export const PRINTERRDIALOG	=  9; *)
(* export const DIRDIALOG	= 10; *)
(* export const DIRERRDIALOG	= 11; *)


var
    viewright : XCoord;
    viewtop : YCoord;

    newHandler, revertHandler, openHandler, saveHandler, saveAsHandler,
    printHandler, dirHandler, quitHandler, undoHandler,
    cutHandler, copyHandler, pasteHandler, dupHandler, deleteHandler,
    selectAllHandler, groupHandler, ungroupHandler, showStructHandler,
    bringToFrontHandler, sendToBackHandler,
    hflipHandler, vflipHandler, cw90Handler, ccw90Handler,
    precMoveHandler, precScaleHandler, precRotHandler,
    lineStyleHandler, fontHandler, nonePatHandler, patHandler, alignHandler, 
    alignToGridHandler,
    twoXHandler, halfXHandler, normalSizeHandler, reduceToFitHandler,
    centerPageHandler, gridHandler, orientationHandler, redrawHandler,
    cachingHandler,
    selectorToolHandler, moveToolHandler, scaleToolHandler, 
    stretchToolHandler, rotateToolHandler,
    magnifyToolHandler, textToolHandler, perpLineToolHandler, 
    lineToolHandler, circleToolHandler, ellipseToolHandler, rectToolHandler, 
    polygonToolHandler, splineToolHandler, closedSplineToolHandler
    : MenuEntryHandler;

    saveAsDialog : Dialog;
    openDialog : Dialog;
    precMoveDialog : Dialog;
    precScaleDialog : Dialog;
    precRotDialog : Dialog;
    saveCurDialog : Dialog;
    overwriteDialog : Dialog;
    revertDialog : Dialog;
    dirDialog : Dialog;
    dirErrDialog : Dialog;

    defaultFont : Font;
    gray : array [1..NGRAYS] of integer;


(* export *)
procedure LoadFonts();
    var font : Font;
begin
    if not GetScreenFont(defaultFont, DEFAULTFONT, 10) then
	defaultFont := GetFont();
    end;

    font := picture.LoadFont("6x10");
    MapFont(font, "Courier", 8, 0.0, 2.5);
    picture.MapFont(font, F6x10);

    font := picture.LoadFont("vtbold");
    MapFont(font, "Courier-Bold", 13, 0.0, 2.5);
    picture.MapFont(font, VTBOLD);

    font := picture.LoadFont("helv10b");
    MapFont(font, "Helvetica", 12, 0.0, 2.5);
    picture.MapFont(font, HELV10B);

    font := picture.LoadFont("helv12");
    MapFont(font, "Helvetica", 14, 0.0, 2.5);
    picture.MapFont(font, HELV12);

    font := picture.LoadFont("helv12b");
    MapFont(font, "Helvetica-Bold", 14, 0.0, 2.5);
    picture.MapFont(font, HELV12B);

    font := picture.LoadFont("helv12i");
    MapFont(font, "Helvetica-Oblique", 14, 0.0, 2.5);
    picture.MapFont(font, HELV12I);

    font := picture.LoadFont("timrom10");
    MapFont(font, "Times-Roman", 12, 0.0, 2.5);
    picture.MapFont(font, TIMROM10);

    font := picture.LoadFont("timrom12");
    MapFont(font, "Times-Roman", 14, 0.0, 2.5);
    picture.MapFont(font, TIMROM12);

    font := picture.LoadFont("timrom12b");
    MapFont(font, "Times-Bold", 15, 0.0, 2.5);
    picture.MapFont(font, TIMROM12B);

    font := picture.LoadFont("timrom12i");
    MapFont(font, "Times-Italic", 15, 0.0, 2.5);
    picture.MapFont(font, TIMROM12I);
end LoadFonts;


(* export *)
procedure CreateWorld(
    const width  : XCoord;
    const height : YCoord
) : Picture;
    var world : Picture;
  	frame : Picture;
	clipBox : Picture;
begin
    viewright := width;
    viewtop := height;
    picture.Create(world);
    picture.DisableRedraw(world);
    picture.Rectangle(world, ORIGINX, ORIGINY, width, height);
    picture.SetOrigin(world, ORIGINX, ORIGINY);
    frame := picture.Copy(world);
    picture.AlterRenderingStyle(world, picture.Fill);
    picture.AlterColor(world, picture.WHITE);
    picture.Nest(frame, world);

    picture.Create(clipBox);
    picture.DisableRedraw(clipBox);
    picture.Rectangle(
        clipBox, ORIGINX + TOOLWIDTH, ORIGINY + SCROLLBARWIDTH, 
	width - SCROLLBARWIDTH, height - CMDHEIGHT
    );
    picture.Nest(clipBox, world);
    return world;
end CreateWorld;


(* export *)
procedure CreateIndicator(
    var lineIndic : Picture;
    var patIndic  : Picture
) : Picture;
    var outline, indicator, nonePat : Picture;
        noneLine, plainLine, leftArrow, rightArrow : Picture;
        horizCenter : XCoord;
begin
    horizCenter := TOOLWIDTH div 2;
    picture.Create(outline);
    picture.Rectangle(outline, 0, 0, TOOLWIDTH, CMDHEIGHT);
    indicator := picture.Copy(outline);
    picture.Line(outline, horizCenter, 0, horizCenter, CMDHEIGHT);
    picture.AlterRenderingStyle(indicator, picture.Fill);
    picture.AlterColor(indicator, picture.WHITE);
    picture.Nest(outline, indicator);

    picture.Create(lineIndic);
    picture.Create(noneLine);
    picture.Create(plainLine);
    picture.Create(leftArrow);
    picture.Create(rightArrow);
    picture.DefaultRenderingStyle(leftArrow, picture.Fill);
    picture.DefaultRenderingStyle(rightArrow, picture.Fill);
    picture.DefaultFont(noneLine, defaultFont);
    picture.Text(noneLine, "N");
    picture.Line(plainLine, 1, 0, horizCenter - 1, 0);
    ArrowHead(leftArrow, 1, 0, 2, 0, 2 * ARROWWIDTH, ARROWHEIGHT);
    ArrowHead(
        rightArrow, horizCenter - 1, 0, horizCenter - 2, 0, 
	2 * ARROWWIDTH, ARROWHEIGHT
    );
    picture.Nest(noneLine, lineIndic);
    picture.Nest(plainLine, lineIndic);
    picture.Nest(leftArrow, lineIndic);
    picture.Nest(rightArrow, lineIndic);
    picture.AlignCenters(lineIndic, noneLine);
    picture.AlignVertCenters(lineIndic, plainLine);
    picture.AlignVertCenters(lineIndic, leftArrow);
    picture.AlignVertCenters(lineIndic, rightArrow);
    picture.AlignVertCenters(indicator, lineIndic);
    picture.AlterPattern(lineIndic, vdi.solidPattern);
    picture.Nest(lineIndic, indicator);

    picture.Create(patIndic);
    picture.DefaultRenderingStyle(patIndic, picture.Fill);
    picture.DefaultPattern(patIndic, vdi.solidPattern);
    picture.Rectangle(
         patIndic, horizCenter + 1, 1, TOOLWIDTH - 1, CMDHEIGHT - 1
    );
    nonePat := picture.Copy(noneLine);
    picture.AlignCenters(patIndic, nonePat);
    picture.Disable(nonePat);
    picture.Nest(nonePat, patIndic);
    picture.Nest(patIndic, indicator);
    picture.Translate(indicator, float(ORIGINX), float(viewtop - CMDHEIGHT));

    return indicator;
end CreateIndicator;

 
procedure TextEntry(
    const string1, string2 : array of char;
    const height : integer
) : Picture;
    var text, background : Picture;
        left, right : XCoord;
	bottom, top : YCoord;
	buf : dynarray of char;
	bufsize, i, size1, size2 : integer;
begin
    picture.Create(text);
    picture.DefaultFont(text, defaultFont);
    picture.DisableRedraw(text);
    if string2[0] = NULL then
	picture.Text(text, string1);
    else
        size1 := number(string1);
	size2 := number(string2);
        bufsize := size1 + size2 + 1;
	new(buf, bufsize);
	for i := 0 to size1 - 2 do
	    buf^[i] := string1[i];
	end;
	buf^[size1 - 1] := " ";
	for i := size1 to bufsize - 3 do
	    buf^[i] := string2[i - size1];
	end;
	buf^[bufsize - 2] := " ";
	buf^[bufsize - 1] := NULL;
        picture.Text(text, buf^[0:bufsize]);
	dispose(buf);
    end;
    picture.GetBoundingBox(text, left, bottom, right, top);

    picture.Create(background);
    picture.DefaultFont(background, defaultFont);
    picture.DisableRedraw(background);
    picture.Rectangle(background, left, 0, right, height);
    picture.AlterRenderingStyle(background, picture.Fill);
    picture.AlterColor(background, picture.WHITE);
    picture.AlignCenters(background, text);
    picture.Nest(text, background);
    return background;
end TextEntry;


procedure PatternEntry(
    const patMenu : Menu;
    const pattern : picture.Pattern;
    const width : integer
);
    var patPict : Picture;
        patId : pointer to picture.Pattern;
	dummy : Menu;
begin
    new(patId);
    patId^ := pattern;
    picture.Create(patPict);
    picture.DisableRedraw(patPict);
    picture.DefaultRenderingStyle(patPict, picture.Fill);
    picture.DefaultPattern(patPict, pattern);
    picture.Rectangle(patPict, 0, 0, width, CMDENTRYHEIGHT);
    picture.DefaultRenderingStyle(patPict, picture.Stroke);
    picture.Rectangle(patPict, 0, 0, width, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
	patMenu, patPict, menu.Outline, patHandler, patId
    );
end PatternEntry;


(* export *)
procedure GetGrayLevel(const pat : Pattern) : real;
    var i : integer;
	n : real;
begin
    n := float(NGRAYS + 1);
    for i := 1 to NGRAYS do
        if gray[i] = pat then
	    return (1.1 * (n - float(i)) - n / 20.0) / n;
	end;
    end;
    if pat = picture.clearPattern then
	return 1.0;
    else
	return 0.0;
    end;
end GetGrayLevel;


procedure PictureEntry(
    const pict : Picture;
    const width  : integer;
    const height : integer
) : Picture;
    var background : Picture;
begin
    picture.Create(background);
    picture.Rectangle(background, 0, 0, XCoord(width), YCoord(height));
    picture.AlterRenderingStyle(background, picture.Fill);
    picture.AlterColor(background, picture.WHITE);
    picture.AlignCenters(background, pict);
    picture.Nest(pict, background);
    return background;
end PictureEntry;


procedure AddToolKeyOp(
    const pict : Picture;
    const op : array of char
) : Picture;
    var left, right : XCoord;
        bottom, top : YCoord;
begin
    picture.GetBoundingBox(pict, left, bottom, right, top);
    picture.MoveTo(pict, right - picture.StrWidth(pict, op) - 1, bottom + 1);
    picture.Text(pict, op);
    return pict;
end AddToolKeyOp;


(* export *)
procedure CreateShades();
    var i : integer;
begin
    for i := 1 to 15 do
        gray[i] := MakePattern(i);
    end;
end CreateShades;


(* export *)
procedure CreateCommands() : Menu;
    var cmds, submenu, dummy : Menu;
        fileMenu, editMenu, structMenu, fontMenu : Menu;
	linesMenu, patMenu, arrangeMenu, optMenu : Menu;
        pict, subpict : Picture;
	i, width : integer;
	fontNo : pointer to integer;
	lineType : pointer to LineType;
	none : array [1..20] of char;
	alignProc : AlignProc;
begin
    menu.Create(cmds);
    menu.DisableRedraw(cmds);
    menu.SetLayout(cmds, menu.RightSide, menu.VertCentered);

    pict := TextEntry(" File ", NULL, CMDHEIGHT - 2);
    picture.Translate(
        pict, float(ORIGINX + TOOLWIDTH + 1), float(viewtop - CMDHEIGHT + 1)
    );
    fileMenu := menu.AppendEntry(cmds, pict, menu.Invert, nil, nil);
    menu.SetLayout(fileMenu, menu.Below, menu.FlushLeft);

    pict := TextEntry(" Edit ", NULL, CMDHEIGHT - 2);
    editMenu := menu.AppendEntry(cmds, pict, menu.Invert, nil, nil);
    menu.SetLayout(editMenu, menu.Below, menu.FlushLeft);

    pict := TextEntry(" Structure ", NULL, CMDHEIGHT - 2);
    structMenu := menu.AppendEntry(cmds, pict, menu.Invert, nil, nil);
    menu.SetLayout(structMenu, menu.Below, menu.FlushLeft);

    pict := TextEntry(" Font ", NULL, CMDHEIGHT - 2);
    fontMenu := menu.AppendEntry(cmds, pict, menu.Invert, nil, nil);
    menu.SetLayout(fontMenu, menu.Below, menu.FlushLeft);

    pict := TextEntry(" Line ", NULL, CMDHEIGHT - 2);
    linesMenu := menu.AppendEntry(cmds, pict, menu.Invert, nil, nil);
    menu.SetLayout(linesMenu, menu.Below, menu.FlushLeft);

    pict := TextEntry(" Shade ", NULL, CMDHEIGHT - 2);
    patMenu := menu.AppendEntry(cmds, pict, menu.Invert, nil, nil);
    menu.SetLayout(patMenu, menu.Below, menu.FlushLeft);

    pict := TextEntry(" Align ", NULL, CMDHEIGHT - 2);
    arrangeMenu := menu.AppendEntry(cmds, pict, menu.Invert, nil, nil);
    menu.SetLayout(arrangeMenu, menu.Below, menu.HorizCentered);

    pict := TextEntry(" Option ", NULL, CMDHEIGHT - 2);
    optMenu := menu.AppendEntry(cmds, pict, menu.Invert, nil, nil);
    menu.SetLayout(optMenu, menu.Below, menu.FlushRight);
 
    AdvCursor();
    pict := TextEntry(" New           ", NEWCODE, CMDENTRYHEIGHT);
    MapKeyOp(NEWOP, fileMenu, newHandler, nil, false);
    dummy := menu.AppendEntry(
        fileMenu, pict, menu.Invert, newHandler, nil
    );
    pict := TextEntry(" Revert        ", REVERTCODE, CMDENTRYHEIGHT);
    MapKeyOp(REVERTOP, fileMenu, revertHandler, nil, false);
    dummy := menu.AppendEntry(
        fileMenu, pict, menu.Invert, revertHandler, nil
    );
    pict := TextEntry("-------------------", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(fileMenu, pict, menu.None, nil, nil);

    pict := TextEntry(" Open...       ", OPENCODE, CMDENTRYHEIGHT);
    MapKeyOp(OPENOP, fileMenu, openHandler, nil, false);
    dummy := menu.AppendEntry(
        fileMenu, pict, menu.Invert, openHandler, nil
    );
    pict := TextEntry(" Save          ", SAVECODE, CMDENTRYHEIGHT);
    MapKeyOp(SAVEOP, fileMenu, saveHandler, nil, false);
    dummy := menu.AppendEntry(
        fileMenu, pict, menu.Invert, saveHandler, nil
    );
    pict := TextEntry(" Save As...    ", SAVEASCODE, CMDENTRYHEIGHT);
    MapKeyOp(SAVEASOP, fileMenu, saveAsHandler, nil, false);
    dummy := menu.AppendEntry(
        fileMenu, pict, menu.Invert, saveAsHandler, nil
    );
    pict := TextEntry(" Print         ", PRINTCODE, CMDENTRYHEIGHT);
    MapKeyOp(PRINTOP, fileMenu, printHandler, nil, false);
    dummy := menu.AppendEntry(
        fileMenu, pict, menu.Invert, printHandler, nil
    );
    pict := TextEntry(" Directory...   ", DIROP, CMDENTRYHEIGHT);
    MapKeyOp(DIROP, fileMenu, dirHandler, nil, false);
    dummy := menu.AppendEntry(
        fileMenu, pict, menu.Invert, dirHandler, nil
    );
    pict := TextEntry("-------------------", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(fileMenu, pict, menu.None, nil, nil);

    pict := TextEntry(" Quit          ", QUITCODE, CMDENTRYHEIGHT);
    MapKeyOp(QUITOP, fileMenu, quitHandler, nil, false);
    dummy := menu.AppendEntry(
        fileMenu, pict, menu.Invert, quitHandler, nil
    );
    menu.AddBorder(fileMenu);
    menu.AddShadow(fileMenu);

    AdvCursor();
    pict := TextEntry(" Undo               ", UNDOOP, CMDENTRYHEIGHT);
    MapKeyOp(UNDOOP, editMenu, undoHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, undoHandler, nil);
    pict := TextEntry(" Cut                ", CUTOP, CMDENTRYHEIGHT);
    MapKeyOp(CUTOP, editMenu, cutHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, cutHandler, nil);
    pict := TextEntry(" Copy               ", COPYOP, CMDENTRYHEIGHT);
    MapKeyOp(COPYOP, editMenu, copyHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, copyHandler, nil);
    pict := TextEntry(" Paste              ", PASTEOP, CMDENTRYHEIGHT);
    MapKeyOp(PASTEOP, editMenu, pasteHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, pasteHandler, nil);
    pict := TextEntry(" Duplicate          ", DUPOP, CMDENTRYHEIGHT);
    MapKeyOp(DUPOP, editMenu, dupHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, dupHandler, nil);
    pict := TextEntry(" Delete            ", DELETECODE, CMDENTRYHEIGHT);
    MapKeyOp(DELETEOP, editMenu, deleteHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, deleteHandler, nil);
    pict := TextEntry(" Select All         ", SELECTALLOP, CMDENTRYHEIGHT);
    MapKeyOp(SELECTALLOP, editMenu, selectAllHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, selectAllHandler, nil);
    pict := TextEntry("-----------------------", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(editMenu, pict, menu.None, nil, nil);
    pict := TextEntry(" Flip Horizontal    ", HFLIPOP, CMDENTRYHEIGHT);
    MapKeyOp(HFLIPOP, editMenu, hflipHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, hflipHandler, nil);
    pict := TextEntry(" Flip Vertical      ", VFLIPOP, CMDENTRYHEIGHT);
    MapKeyOp(VFLIPOP, editMenu, vflipHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, vflipHandler, nil);
    pict := TextEntry(" 90 Clockwise       ", CW90OP, CMDENTRYHEIGHT);
    MapKeyOp(CW90OP, editMenu, cw90Handler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, cw90Handler, nil);
    pict := TextEntry(" 90 CounterCW       ", CCW90OP, CMDENTRYHEIGHT);
    MapKeyOp(CCW90OP, editMenu, ccw90Handler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, ccw90Handler, nil);

    pict := TextEntry("-----------------------", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(editMenu, pict, menu.None, nil, nil);
    pict := TextEntry(" Precise Move...    ", PRECMOVEOP, CMDENTRYHEIGHT);
    MapKeyOp(PRECMOVEOP, editMenu, precMoveHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, precMoveHandler, nil);
    pict := TextEntry(" Precise Scale...   ", PRECSCALEOP, CMDENTRYHEIGHT);
    MapKeyOp(PRECSCALEOP, editMenu, precScaleHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, precScaleHandler, nil);
    pict := TextEntry(" Precise Rotate...  ", PRECROTOP, CMDENTRYHEIGHT);
    MapKeyOp(PRECROTOP, editMenu, precRotHandler, nil, false);
    dummy := menu.AppendEntry(editMenu, pict, menu.Invert, precRotHandler, nil);

    menu.AddBorder(editMenu);
    menu.AddShadow(editMenu);

    AdvCursor();
    pict := TextEntry(" Group             ", GROUPOP, CMDENTRYHEIGHT);
    MapKeyOp(GROUPOP, structMenu, groupHandler, nil, false);
    dummy := menu.AppendEntry(structMenu, pict, menu.Invert, groupHandler, nil);
    pict := TextEntry(" Ungroup          ", UNGROUPCODE, CMDENTRYHEIGHT);
    MapKeyOp(UNGROUPOP, structMenu, ungroupHandler, nil, false);
    dummy := menu.AppendEntry(structMenu, pict, menu.Invert, ungroupHandler, nil);
    pict := TextEntry(" Show Structure    ", SHOWSTRUCTCODE, CMDENTRYHEIGHT);
    MapKeyOp(SHOWSTRUCTOP, structMenu, showStructHandler, nil, false);
    dummy := menu.AppendEntry(structMenu, pict, menu.Invert, showStructHandler, nil);
    pict := TextEntry("----------------------", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(structMenu, pict, menu.None, nil, nil);
    pict := TextEntry(" Bring to Front    ", BRINGTOFRONTOP, CMDENTRYHEIGHT);
    MapKeyOp(BRINGTOFRONTOP, structMenu, bringToFrontHandler, nil, false);
    dummy := menu.AppendEntry(structMenu, pict, menu.Invert, bringToFrontHandler, nil);
    pict := TextEntry(" Send to Back      ", SENDTOBACKOP, CMDENTRYHEIGHT);
    MapKeyOp(SENDTOBACKOP, structMenu, sendToBackHandler, nil, false);
    dummy := menu.AppendEntry(structMenu, pict, menu.Invert, sendToBackHandler, nil);

    menu.AddBorder(structMenu);
    menu.AddShadow(structMenu);

    AdvCursor();
    new(fontNo);
    if not GetScreenFont(fontNo^, "Courier", 8) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Courier 8            ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );
    new(fontNo);
    if not GetScreenFont(fontNo^, "Courier", 10) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Courier 10           ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );
    new(fontNo);
    if not GetScreenFont(fontNo^, "Courier-Bold", 13) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Courier Bold 13      ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );

    new(fontNo);
    if not GetScreenFont(fontNo^, "Helvetica", 12) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Helvetica 12         ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );
    new(fontNo);
    if not GetScreenFont(fontNo^, "Helvetica", 14) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Helvetica 14         ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );
    new(fontNo);
    if not GetScreenFont(fontNo^, "Helvetica-Bold", 14) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Helvetica Bold 14    ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );
    new(fontNo);
    if not GetScreenFont(fontNo^, "Helvetica-Oblique", 14) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Helvetica Oblique 14 ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );

    new(fontNo);
    if not GetScreenFont(fontNo^, "Times-Roman", 12) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Times Roman 12       ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );
    new(fontNo);
    if not GetScreenFont(fontNo^, "Times-Roman", 14) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Times Roman 14       ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );
    new(fontNo);
    if not GetScreenFont(fontNo^, "Times-Bold", 15) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Times Bold 15        ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );
    new(fontNo);
    if not GetScreenFont(fontNo^, "Times-Italic", 15) then
	fontNo^ := GetFont();
    end;
    pict := TextEntry(" Times Italic 15      ", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        fontMenu, pict, menu.Invert, fontHandler, fontNo
    );

    menu.AddBorder(fontMenu);
    menu.AddShadow(fontMenu);

    AdvCursor();
    none := "  None  ";
    width := picture.StrWidth(pict, none);

    new(lineType);
    lineType^ := None;
    pict := TextEntry(none, NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
	linesMenu, pict, menu.Invert, lineStyleHandler, lineType
    );
    new(lineType);
    lineType^ := Plain;
    picture.Create(subpict);
    picture.Line(subpict, MENUMARGIN, 0, width - MENUMARGIN, 0);
    pict := PictureEntry(subpict, width, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
	linesMenu, pict, menu.Invert, lineStyleHandler, lineType
    );
    new(lineType);
    lineType^ := LeftArrow;
    picture.Create(subpict);
    picture.DefaultRenderingStyle(subpict, picture.Fill);
    picture.DefaultPattern(subpict, vdi.solidPattern);
    picture.Line(subpict, MENUMARGIN, 0, width - MENUMARGIN, 0);
    ArrowHead(subpict, MENUMARGIN, 0, width - MENUMARGIN, 0, 2 * ARROWWIDTH, ARROWHEIGHT);
    pict := PictureEntry(subpict, width, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
	linesMenu, pict, menu.Invert, lineStyleHandler, lineType
    );
    new(lineType);
    lineType^ := RightArrow;
    picture.Create(subpict);
    picture.DefaultRenderingStyle(subpict, picture.Fill);
    picture.DefaultPattern(subpict, vdi.solidPattern);
    picture.Line(subpict, MENUMARGIN, 0, width - MENUMARGIN, 0);
    ArrowHead(subpict, width - MENUMARGIN, 0, MENUMARGIN, 0, 2 * ARROWWIDTH, ARROWHEIGHT);
    pict := PictureEntry(subpict, width, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
	linesMenu, pict, menu.Invert, lineStyleHandler, lineType
    );
    new(lineType);
    lineType^ := DblArrow;
    picture.Create(subpict);
    picture.DefaultRenderingStyle(subpict, picture.Fill);
    picture.DefaultPattern(subpict, vdi.solidPattern);
    picture.Line(subpict, MENUMARGIN, 0, width - MENUMARGIN, 0);
    ArrowHead(subpict, MENUMARGIN, 0, width - MENUMARGIN, 0, 2 * ARROWWIDTH, ARROWHEIGHT);
    ArrowHead(subpict, width - MENUMARGIN, 0, MENUMARGIN, 0, 2 * ARROWWIDTH, ARROWHEIGHT);
    pict := PictureEntry(subpict, width, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
	linesMenu, pict, menu.Invert, lineStyleHandler, lineType
    );
    menu.AddBorder(linesMenu);
    menu.AddShadow(linesMenu);

    AdvCursor();
    pict := TextEntry(none, NULL, CMDENTRYHEIGHT);
    picture.DefaultRenderingStyle(pict, picture.Stroke);
    picture.Rectangle(pict, 0, 0, width, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(
        patMenu, pict, menu.Outline, nonePatHandler, nil
    );
    PatternEntry(patMenu, vdi.solidPattern, width);
    PatternEntry(patMenu, vdi.clearPattern, width);
    for i := 1 to 15 do
        PatternEntry(patMenu, gray[i], width);
    end;
    menu.AddBorder(patMenu);
    menu.AddShadow(patMenu);

    AdvCursor();
    new(alignProc);
    alignProc^ := picture.AlignLeftSides;
    pict := TextEntry(" Left Sides    ", ALNLEFTOP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNLEFTOP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignRightSides;
    pict := TextEntry(" Right Sides   ", ALNRIGHTOP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNRIGHTOP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignBottoms;
    pict := TextEntry(" Bottoms       ", ALNBOTOP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNBOTOP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignTops;
    pict := TextEntry(" Tops          ", ALNTOPOP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNTOPOP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignVertCenters;
    pict := TextEntry(" Vert Centers  ", ALNVERTCTROP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNVERTCTROP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignHorizCenters;
    pict := TextEntry(" Horiz Centers ", ALNHORIZCTROP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNHORIZCTROP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignCenters;
    pict := TextEntry(" Centers       ", ALNCTROP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNCTROP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignLeftToRight;
    pict := TextEntry(" Left to Right ", ALNLTOROP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNLTOROP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignRightToLeft;
    pict := TextEntry(" Right to Left ", ALNRTOLOP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNRTOLOP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignBottomToTop;
    pict := TextEntry(" Bottom to Top ", ALNBTOTOP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNBTOTOP, arrangeMenu, alignHandler, dummy, false);
    new(alignProc);
    alignProc^ := picture.AlignTopToBottom;
    pict := TextEntry(" Top to Bottom ", ALNTTOBOP, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignHandler, alignProc);
    MapKeyOp(ALNTTOBOP, arrangeMenu, alignHandler, dummy, false);
    pict := TextEntry(" Align to Grid ", ALNTOGRIDOP, CMDENTRYHEIGHT);
    MapKeyOp(ALNTOGRIDOP, arrangeMenu, alignToGridHandler, nil, false);
    dummy := menu.AppendEntry(arrangeMenu, pict, menu.Invert, alignToGridHandler, nil);
    menu.AddBorder(arrangeMenu);
    menu.AddShadow(arrangeMenu);

    pict := TextEntry(" Reduce         ", REDUCEOP, CMDENTRYHEIGHT);
    MapKeyOp(REDUCEOP, optMenu, halfXHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, halfXHandler, nil);
    pict := TextEntry(" Enlarge        ", ENLARGEOP, CMDENTRYHEIGHT);
    MapKeyOp(ENLARGEOP, optMenu, twoXHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, twoXHandler, nil);
    pict := TextEntry(" Normal Size    ", NORMSIZEOP, CMDENTRYHEIGHT);
    MapKeyOp(NORMSIZEOP, optMenu, normalSizeHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, normalSizeHandler, nil);
    pict := TextEntry(" Reduce to Fit  ", REDUCETOFITOP, CMDENTRYHEIGHT);
    MapKeyOp(REDUCETOFITOP, optMenu, reduceToFitHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, reduceToFitHandler, nil);
    pict := TextEntry(" Center Page    ", CENTERPAGEOP, CMDENTRYHEIGHT);
    MapKeyOp(CENTERPAGEOP, optMenu, centerPageHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, centerPageHandler, nil);
    pict := TextEntry("-------------------", NULL, CMDENTRYHEIGHT);
    dummy := menu.AppendEntry(optMenu, pict, menu.None, nil, nil);
    pict := TextEntry(" Grid on/off    ", GRIDONOFFOP, CMDENTRYHEIGHT);
    MapKeyOp(GRIDONOFFOP, optMenu, gridHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, gridHandler, nil);
    pict := TextEntry(" Orientation    ", ORIENTATIONOP, CMDENTRYHEIGHT);
    MapKeyOp(ORIENTATIONOP, optMenu, orientationHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, orientationHandler, nil);
    pict := TextEntry(" Redraw on/off  ", REDRAWOP, CMDENTRYHEIGHT);
    MapKeyOp(REDRAWOP, optMenu, redrawHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, redrawHandler, nil);

    pict := TextEntry(" Caching on/off ", CACHINGOP, CMDENTRYHEIGHT);
    MapKeyOp(CACHINGOP, optMenu, cachingHandler, nil, false);
    dummy := menu.AppendEntry(optMenu, pict, menu.Invert, cachingHandler, nil);

    menu.AddBorder(optMenu);
    menu.AddShadow(optMenu);

    AdvCursor();
    return cmds;        
end CreateCommands;


(* export *)
procedure CreateDialogs();
    var msg : Picture;
begin
    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "Enter a name for this drawing (cancel):");
    dialog.Create(saveAsDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.9);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "Enter name of drawing to open (cancel):");
    dialog.Create(openDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.9);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "Enter X and Y movement (cancel):");
    dialog.Create(precMoveDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.5);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "Enter X and Y scaling (cancel):");
    dialog.Create(precScaleDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.5);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "Enter rotation in degrees (cancel):");
    dialog.Create(precRotDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.3);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "Save current drawing? [yn] (cancel):");
    dialog.Create(saveCurDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.05);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "already exists; overwrite? [yn] (cancel):");
    dialog.Create(overwriteDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.05);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "really want to revert to original? [yn] (cancel):");
    dialog.Create(revertDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.05);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "is the current directory.  Change to (cancel):");
    dialog.Create(dirDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.9);

    picture.Create(msg);
    picture.DefaultFont(msg, defaultFont);
    picture.Text(msg, "Change current directory to (cancel):");
    dialog.Create(dirErrDialog, DIALOGWIDTH, DIALOGHEIGHT, msg, 0.9);
end CreateDialogs;


(* export *)
procedure SetDialogOrigins(
    const x : XCoord;
    const y : YCoord
);
begin
    dialog.SetOrigin(saveAsDialog, x, y);
    dialog.SetOrigin(openDialog, x, y);
    dialog.SetOrigin(precMoveDialog, x, y);
    dialog.SetOrigin(precScaleDialog, x, y);
    dialog.SetOrigin(precRotDialog, x, y);
    dialog.SetOrigin(saveCurDialog, x, y);
    dialog.SetOrigin(overwriteDialog, x, y);
    dialog.SetOrigin(revertDialog, x, y);
    dialog.SetOrigin(dirDialog, x, y);
    dialog.SetOrigin(dirErrDialog, x, y);
end SetDialogOrigins;


(* export *)
procedure AlignDialogCenters(const p : Picture);
begin
    picture.AlignCenters(p, dialog.GetPicture(saveAsDialog));
    picture.AlignCenters(p, dialog.GetPicture(openDialog));
    picture.AlignCenters(p, dialog.GetPicture(precMoveDialog));
    picture.AlignCenters(p, dialog.GetPicture(precScaleDialog));
    picture.AlignCenters(p, dialog.GetPicture(precRotDialog));
    picture.AlignCenters(p, dialog.GetPicture(saveCurDialog));
    picture.AlignCenters(p, dialog.GetPicture(overwriteDialog));
    picture.AlignCenters(p, dialog.GetPicture(revertDialog));
    picture.AlignCenters(p, dialog.GetPicture(dirDialog));
    picture.AlignCenters(p, dialog.GetPicture(dirErrDialog));
end AlignDialogCenters;


(* export *)
procedure GetDialog(const id : integer) : Dialog;
begin
    case id of
	SAVEASDIALOG:	    return saveAsDialog;    |
	OPENDIALOG:	    return openDialog;	    |
	PRECMOVEDIALOG:	    return precMoveDialog;  |
	PRECSCALEDIALOG:    return precScaleDialog; |
	PRECROTDIALOG:	    return precRotDialog;   |
	SAVECURDIALOG:	    return saveCurDialog;   |
	OVERWRITEDIALOG:    return overwriteDialog; |
	REVERTDIALOG:	    return revertDialog;    |
	DIRDIALOG:	    return dirDialog;	    |
	DIRERRDIALOG:	    return dirErrDialog;
	else
	    return Dialog(nil);
    end;
end GetDialog;


(* export *)
procedure AddDialogWarning(
    const id : integer;
    const s1, s2 : array of char
);
    var warning : Picture;
begin
    picture.Create(warning);
    picture.DefaultFont(warning, defaultFont);
    picture.Text(warning, s1);
    picture.Text(warning, s2);
    dialog.AddWarning(GetDialog(id), warning);
end AddDialogWarning;


(* export *)
procedure RemoveDialogWarning(const id : integer);
begin
    dialog.RemoveWarning(GetDialog(id));
end RemoveDialogWarning;


procedure ToolEntry(var tool : Picture) : Picture;
    var background : Picture;
begin
    picture.Create(background);
    picture.DefaultFont(background, defaultFont);
    picture.DisableRedraw(background);
    picture.Rectangle(background, 1, 1, TOOLWIDTH - 1, TOOLHEIGHT - 1);
    picture.AlterRenderingStyle(background, picture.Fill);
    picture.AlterColor(background, picture.WHITE);
    picture.AlignCenters(background, tool);
    picture.Nest(tool, background);
    return background;
end ToolEntry;


(* export *)
procedure CreateTools() : Menu;
    var tools, dummy : Menu;
	pict : Picture;
	x : array [1..5] of XCoord;
	y : array [1..5] of YCoord;
	cursor : Cursor;
begin
    new(cursor);
    cursor^ := arrowCursor;
  
    menu.Create(tools);
    menu.DisableRedraw(tools);
    menu.SetLayout(tools, menu.Below, menu.FlushLeft);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.DefaultFont(pict, defaultFont);
    picture.Text(pict, "Select");
    pict := AddToolKeyOp(ToolEntry(pict), SELECTTOOLOP);
    picture.Translate(
        pict, float(ORIGINX), float(viewtop - CMDHEIGHT - TOOLHEIGHT)
    );
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, selectorToolHandler, cursor
    );
    MapKeyOp(SELECTTOOLOP, dummy, selectorToolHandler, nil, true);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.DefaultFont(pict, defaultFont);
    picture.Text(pict, "Move");
    pict := AddToolKeyOp(ToolEntry(pict), MOVETOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, moveToolHandler, cursor
    );
    MapKeyOp(MOVETOOLOP, dummy, moveToolHandler, nil, true);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.DefaultFont(pict, defaultFont);
    picture.Text(pict, "Scale");
    pict := AddToolKeyOp(ToolEntry(pict), SCALETOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, scaleToolHandler, cursor
    );
    MapKeyOp(SCALETOOLOP, dummy, scaleToolHandler, nil, true);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.DefaultFont(pict, defaultFont);
    picture.Text(pict, "Stretch");
    pict := AddToolKeyOp(ToolEntry(pict), STRETCHTOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, stretchToolHandler, cursor
    );
    MapKeyOp(STRETCHTOOLOP, dummy, stretchToolHandler, nil, true);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.DefaultFont(pict, defaultFont);
    picture.Text(pict, "Rotate");
    pict := AddToolKeyOp(ToolEntry(pict), ROTATETOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, rotateToolHandler, cursor
    );
    MapKeyOp(ROTATETOOLOP, dummy, rotateToolHandler, nil, true);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.DefaultFont(pict, defaultFont);
    picture.Text(pict, "Magnify");
    pict := AddToolKeyOp(ToolEntry(pict), MAGNIFYTOOLOP);
    new(cursor);
    cursor^ := crossHairsCursor;
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, magnifyToolHandler, cursor
    );
    MapKeyOp(MAGNIFYTOOLOP, dummy, magnifyToolHandler, nil, true);
    
    AdvCursor();
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.DefaultFont(pict, defaultFont);
    picture.Text(pict, "Text");
    pict := AddToolKeyOp(ToolEntry(pict), TEXTTOOLOP);
    new(cursor);
    cursor^ := ltextCursor;
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, textToolHandler, cursor
    );
    MapKeyOp(TEXTTOOLOP, dummy, textToolHandler, nil, true);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.Line(pict, 0, 15, 20, 15);
    picture.Line(pict, 25, 0, 25, 20);
    pict := AddToolKeyOp(ToolEntry(pict), PERPLINETOOLOP);
    new(cursor);
    cursor^ := crossHairsCursor;
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, perpLineToolHandler, cursor
    );
    MapKeyOp(PERPLINETOOLOP, dummy, perpLineToolHandler, nil, true);

    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.Line(pict, 0, 0, 20, 20);
    pict := AddToolKeyOp(ToolEntry(pict), LINETOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, lineToolHandler, cursor
    );
    MapKeyOp(LINETOOLOP, dummy, lineToolHandler, nil, true);

    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.Circle(pict, 10, 10, 10);
    pict := AddToolKeyOp(ToolEntry(pict), CIRCLETOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, circleToolHandler, cursor
    );
    MapKeyOp(CIRCLETOOLOP, dummy, circleToolHandler, nil, true);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.Ellipse(pict, 10, 10, 12, 8);
    pict := AddToolKeyOp(ToolEntry(pict), ELLIPSETOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, ellipseToolHandler, cursor
    );
    MapKeyOp(ELLIPSETOOLOP, dummy, ellipseToolHandler, nil, true);
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.Rectangle(pict, 0, 0, 20, 20);
    pict := AddToolKeyOp(ToolEntry(pict), RECTTOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, rectToolHandler, cursor
    );
    MapKeyOp(RECTTOOLOP, dummy, rectToolHandler, nil, true);
        
    x[1] := 5;
    y[1] := 0;
    x[2] := 0;
    y[2] := 10;
    x[3] := 10;
    y[3] := 20;
    x[4] := 23;
    y[4] := 15;
    x[5] := 20;
    y[5] := 5;
    
    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.Polygon(pict, x, y);
    pict := AddToolKeyOp(ToolEntry(pict), POLYGONTOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, polygonToolHandler, cursor
    );
    MapKeyOp(POLYGONTOOLOP, dummy, polygonToolHandler, nil, true);

    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.BSpline(pict, x, y);
    pict := AddToolKeyOp(ToolEntry(pict), SPLINETOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, splineToolHandler, cursor
    );
    MapKeyOp(SPLINETOOLOP, dummy, splineToolHandler, nil, true);

    picture.Create(pict);
    picture.DisableRedraw(pict);
    picture.ClosedBSpline(pict, x, y);
    pict := AddToolKeyOp(ToolEntry(pict), CLOSEDSPLINETOOLOP);
    dummy := menu.AppendEntry(
        tools, pict, menu.Invert, closedSplineToolHandler, cursor
    );
    MapKeyOp(CLOSEDSPLINETOOLOP, dummy, closedSplineToolHandler, nil, true);

    return tools;
end CreateTools;


(* export *)
procedure CreateUserPicture(
    const lineStyle : integer;
    const pattern   : integer;
    const rendering : RenderingStyle
) : Picture;
    var userPicture : Picture;
begin
    picture.Create(userPicture);
    picture.DefaultFont(userPicture, defaultFont);
    picture.DefaultLineStyle(userPicture, lineStyle);
    picture.DefaultPattern(userPicture, pattern);
    picture.DefaultRenderingStyle(userPicture, rendering);
    return userPicture;
end CreateUserPicture;


(* export *)
procedure CreatePage() : Picture;
    var page : Picture;
        x0, x1 : XCoord;
	y0, y1 : YCoord;
begin
    picture.Create(page);
    x0 := round(OFFSETX * PIXPERINCH);
    y0 := round(OFFSETY * PIXPERINCH);
    x1 := round((PAGESIZEX + OFFSETX) * PIXPERINCH);
    y1 := round((PAGESIZEY + OFFSETY) * PIXPERINCH);
    picture.DefaultRenderingStyle(page, picture.Stroke);
    picture.DefaultColor(page, picture.BLACK);
    picture.Rectangle(page, x0, y0, x1, y1);
    picture.Rectangle(page, x0 - 1, y0 - 1, x1 + 1, y1 + 1);
    picture.Rectangle(page, x0 + 1, y0 + 1, x1 - 1, y1 - 1);
    return page;
end CreatePage;


(* export *)
procedure CreateScrollBars(
    var sbX, autoDnX, autoUpX, pageAreaX, sliderX : Picture;
    var sbY, autoDnY, autoUpY, pageAreaY, sliderY : Picture
);
begin
    sbX := scrollbar.Create(
        SCROLLBARWIDTH * 5, SCROLLBARWIDTH, 
	autoDnX, autoUpX, pageAreaX, sliderX
    );
    sbY := scrollbar.Create(
	SCROLLBARWIDTH, SCROLLBARWIDTH * 5,
	autoDnY, autoUpY, pageAreaY, sliderY
    );
end CreateScrollBars;


begin
    new(newHandler);
    new(revertHandler);
    new(openHandler);
    new(saveHandler);
    new(saveAsHandler);
    new(printHandler);
    new(dirHandler);
    new(quitHandler);
    new(undoHandler);
    new(cutHandler);
    new(copyHandler);
    new(pasteHandler);
    new(deleteHandler);
    new(dupHandler);
    new(selectAllHandler);
    new(groupHandler);
    new(ungroupHandler);
    new(showStructHandler);
    new(bringToFrontHandler);
    new(sendToBackHandler);
    new(hflipHandler);
    new(vflipHandler);
    new(cw90Handler);
    new(ccw90Handler);
    new(precMoveHandler);
    new(precScaleHandler);
    new(precRotHandler);
    new(fontHandler);
    new(lineStyleHandler);
    new(nonePatHandler);
    new(patHandler);
    new(alignHandler);
    new(alignToGridHandler);
    new(twoXHandler);
    new(halfXHandler);
    new(normalSizeHandler);
    new(reduceToFitHandler);
    new(centerPageHandler);
    new(gridHandler);
    new(orientationHandler);
    new(redrawHandler);
    new(cachingHandler);

    newHandler^ := NewHandler;
    revertHandler^ := RevertHandler;
    openHandler^ := OpenHandler;
    saveHandler^ := SaveHandler;
    saveAsHandler^ := SaveAsHandler;
    printHandler^ := PrintHandler;
    dirHandler^ := DirHandler;
    quitHandler^ := QuitHandler;
    undoHandler^ := UndoHandler;
    cutHandler^ := CutHandler;
    copyHandler^ := CopyHandler;
    pasteHandler^ := PasteHandler;
    deleteHandler^ := DeleteHandler;
    dupHandler^ := DupHandler;
    selectAllHandler^ := SelectAllHandler;
    groupHandler^ := GroupHandler;
    ungroupHandler^ := UngroupHandler;
    showStructHandler^ := ShowStructHandler;
    bringToFrontHandler^ := BringToFrontHandler;
    sendToBackHandler^ := SendToBackHandler;
    hflipHandler^ := HflipHandler;
    vflipHandler^ := VflipHandler;
    cw90Handler^ := CW90Handler;
    ccw90Handler^ := CCW90Handler;
    precMoveHandler^ := PrecMoveHandler;
    precScaleHandler^ := PrecScaleHandler;
    precRotHandler^ := PrecRotHandler;
    fontHandler^ := FontHandler;
    lineStyleHandler^ := LineStyleHandler;
    nonePatHandler^ := NonePatHandler;
    patHandler^ := PatHandler;
    alignHandler^ := AlignHandler;
    alignToGridHandler^ := AlignToGridHandler;
    twoXHandler^ := TwoXHandler;
    halfXHandler^ := HalfXHandler;
    normalSizeHandler^ := NormalSizeHandler;
    reduceToFitHandler^ := ReduceToFitHandler;
    centerPageHandler^ := CenterPageHandler;
    gridHandler^ := GridHandler;
    orientationHandler^ := OrientationHandler;
    redrawHandler^ := RedrawHandler;
    cachingHandler^ := CachingHandler;

    new(selectorToolHandler);
    new(moveToolHandler);
    new(scaleToolHandler);
    new(stretchToolHandler);
    new(rotateToolHandler);
    new(magnifyToolHandler);
    new(textToolHandler);
    new(perpLineToolHandler);
    new(lineToolHandler);
    new(circleToolHandler);
    new(ellipseToolHandler);
    new(rectToolHandler);
    new(polygonToolHandler);
    new(splineToolHandler);
    new(closedSplineToolHandler);
    selectorToolHandler^ := SelectorHandler;
    moveToolHandler^ := MoveHandler;
    scaleToolHandler^ := ScaleHandler;
    stretchToolHandler^ := StretchHandler;
    rotateToolHandler^ := RotateHandler;
    magnifyToolHandler^ := MagnifyHandler;
    textToolHandler^ := TextHandler;
    perpLineToolHandler^ := PerpLineHandler;
    lineToolHandler^ := LineHandler;
    circleToolHandler^ := CircleHandler;
    ellipseToolHandler^ := EllipseHandler;
    rectToolHandler^ := RectHandler;
    polygonToolHandler^ := PolygonHandler;
    splineToolHandler^ := SplineHandler;
    closedSplineToolHandler^ := ClosedSplineHandler;
end dvboot.
