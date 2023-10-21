implementation module dvdefs;

(* export *)
import 
    picture,
    menu,
    dialog,
    rubband,
    input;

(* export *)
from drawing import
    Drawing;

(* export *)
from picture import
    Picture, XCoord, YCoord;

(* export *)
from genlists import
    GenericList;

(* export *)
from system import
    Word;

(* export const VERSION = "$Revision: 4.12 $"; *)
(* export const DATE = "$Date: 87/11/19 16:30:38 $"; *)
(* export const INFOOP = "?"; *)

(* export const MINWIDTH = 250; *)
(* export const MINHEIGHT = 150; *)
(* export const SMALLWIDTH = 425; *)
(* export const SMALLHEIGHT = 540; *)
(* export const LARGEWIDTH = 652; *)
(* export const LARGEHEIGHT = 850; *)
   
(* export const LEFTBUTTON   = 0; *)
(* export const MIDDLEBUTTON = 1; *)
(* export const RIGHTBUTTON  = 2; *)
(* export const LARROW = 167; *)
(* export const RARROW = 168; *)
(* export const DARROW = 169; *)
(* export const UARROW = 170; *)

(* export const NULL = 0C; *)
(* export const BACKSPACE = 10C; *)
(* export const DELETE = 177C; *)
(* export const LINEFEED = 12C; *)
(* export const CARRIAGERTN = 15C; *)
(* export const BELL = 7C; *)
    
(* export const PRINTSUFFIX = ".ps"; *)
(* export const CLIPBOARD = ".clipboard"; *)
(* export const FONTFILE = ".fonts"; *)
(* export const DEFAULTFONT = "Courier"; *)

(* export const GRIDSIZE = 8; *)
(* export const SELECTAREA = 4; *)

(* export const PIXPERINCH = 72.0; *)
(* export const PAGESIZEX = 8.0; *)
(* export const PAGESIZEY = 10.9; *)
(* export const OFFSETX = 0.25; *)
(* export const OFFSETY = 0.05; *)

(* export const MAXPOLYPTS = 128; *)
(* export const MAXZOOMIN = 5; *)	(* powers of 2 *)
(* export const MAXZOOMOUT = -3; *)
(* export const MAXFILENAMESIZE = 100; *)
(* export const MAXPATHSIZE = 300; *)

(* export type Cursor = pointer to input.Cursor; *)
(* export type Font = pointer to picture.Font; *)
(* export type Pattern = pointer to picture.Pattern; *)
(* export type CharSet = set of char; *)
(* export type CharPtr = pointer to char; *)

(* export type AlignProc = pointer to procedure(Picture, Picture); *)
(* export type VertexObjProc =
    procedure (Picture, array of XCoord, array of YCoord);
 *)

(* export
type
    Box = record
        x0, x1 : XCoord;
	y0, y1 : YCoord;
    end;
 *)

(* export type LineType = (None, Plain, LeftArrow, RightArrow, DblArrow); *)
(* export type VertexObjType = (Polygon, Spline, ClosedSpline); *)

(* export
type
    StateType = (
        Idling, Rubberbanding, TextEntering, Menuing, Scrolling, Selecting,
	TempSelecting, TempRubberbanding, Dialoging
    );
 *)



(* export
type
    UndoOp = (
	Move, MoveVertex, Scale, Stretch, Rotate, 
	Insertion, Deletion, Alignment, Modification,
	SendToBack, BringToFront, Group, Ungroup, ShowStructure
    );
 *)
(* export type UndoOpSet = set of UndoOp; *)

(* export
type
    ModifOp = (
	FontChange, PatternChange
    );
 *)

(* export
type
    Relation = (
	Parent, Sibling
    );
 *)

(* export type UndoObj = pointer to UndoObjRecord; *)
(* export
type
    UndoObjRecord = record
	selecList : GenericList;    (* Same selecList as in drawingView *)
	zoom : integer;		    (* with no rubberbands.		*)
	undone : boolean;

	case undoOp : UndoOp of
	    Move:
		dx, dy : real;
		|
	    MoveVertex:
		selection : Picture;
		vertex : integer;
		vdx : XCoord;
		vdy : YCoord;
		|
	    Scale:
	    	sx, sy : real;
		|
	    Stretch:
		stx, sty : real;
		codex, codey : integer;
		|
	    Rotate:
		angle : real;
		|
	    Alignment, Modification, SendToBack, BringToFront, Group, Ungroup:
		infoList : GenericList;
	    else
	end;
    end;
 *)

(* export type SelecInfo = pointer to SelecInfoRecord; *)
(* export
type 
    SelecInfoRecord = record
	case undoOp : UndoOp of
	    Alignment:
		dx, dy : real;
		|
	    SendToBack, BringToFront, Group:
		relative : Picture;
		relation : Relation;
		|
	    Ungroup:
		numInGroup : integer;
	    else
	end;
    end;
 *)

(* export
type
    StateRecord = record
        activity     : StateType;
	rubberband   : rubband.Rubberband;
	selection    : Picture;
	exposedCmd   : menu.Menu;
	curCmdEntry  : menu.Menu;
	curTool	     : menu.Menu;
	curScroll    : Picture;
	curDialog    : dialog.Dialog;
	selecList    : GenericList;
	charList     : GenericList;
	charCount    : integer;
	textPict     : Picture;
	gridOn	     : boolean;
	redrawOn     : boolean;
	landscape    : boolean;
	selecSorted  : boolean;
	dirty        : boolean;
	lineType     : LineType;
	zoom	     : integer;
	count	     : integer;
	px	     : array [1..MAXPOLYPTS] of XCoord;
	py	     : array [1..MAXPOLYPTS] of YCoord;
	undoInfo     : UndoObj;
    end;
 *)
(* export
type
    ScrollBarRecord = record
	scrollBar	: Picture;
        autoDownButton	: Picture;
	autoUpButton 	: Picture;
	pageArea	: Picture;
	slider		: Picture;
    end;
 *)

(* public export type DrawingView = pointer to DrawingViewRecord; *)
(* export
type
    DrawingViewRecord = record
	drawing       : Drawing;
	selected      : boolean;
	quit	      : boolean;
	left, right   : XCoord;
	bottom, top   : YCoord;
	width, height : integer;
	stateInfo     : StateRecord;
    	world	      : Picture;
	indicator     : Picture;
	lineIndic     : Picture;
	patIndic      : Picture;
	commands      : menu.Menu;
	tools	      : menu.Menu;
	selectTool    : menu.Menu;
	pict          : Picture;
	pictClipBox   : Box;
	page	      : Picture;
	xscroll	      : ScrollBarRecord;
	yscroll	      : ScrollBarRecord;
	defaultCursor : integer;
	noFileSpecified : boolean;
	fileName      : array [1..MAXFILENAMESIZE] of char;
	bannermsg1    : array [1..MAXFILENAMESIZE] of char;
	bannermsg2    : array [1..MAXFILENAMESIZE] of char;
	curPath	      : array [1..MAXPATHSIZE] of char;
    end;
 *)

begin
end dvdefs.
