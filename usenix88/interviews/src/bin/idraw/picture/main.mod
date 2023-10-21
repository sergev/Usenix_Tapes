implementation module main;

(* export *)
import 
    vdi,
    genlists,
    system;

(* export const MAXCOMPLEXITY = 5; *)

(* public export const BLACK = vdi.BLACK; *)
(* public export const WHITE = vdi.WHITE; *)

(* public export type GenericList = genlists.GenericList; *)
(* public export type Word = system.Word; *)

(* public export type XCoord = vdi.XCoord; *)
(* public export type YCoord = vdi.YCoord; *)
(* public export type ColorValue = vdi.ColorValue; *)
(* public export type RenderingFunction = vdi.RenderingFunction; *)
(* public export type PatternData = vdi.PatternData; *)
(* public export type Pattern = vdi.Pattern; *)
(* public export type LineStyle = vdi.LineStyle; *)
(* public export type Font = vdi.Font; *)

(* public export type
    PictureObjectType = (PointObj, CircleObj, EllipseObj, RectObj, 
    			 PolygonObj, BSplineObj, ClosedBSplineObj,
			 LineObj, TextObj, PictureList, None);
 *)
(* export type
    AttribType = (
        FontAttrib, Color, BkgndColor, Paint, PatternAttrib, 
	Rendering, LineStyleAttrib, CurPos
    );
 *)
			 
(* public export type
    RenderingStyle = (Fill, Stroke);
 *)
(* public export type
    PaintType = (Opaque, Transparent, Xor);
 *)
(* export type
    ConcatOrder = (Before, After);
 *)

(* export
type
    PointType = record
        x : XCoord;
	y : YCoord;
    end;
 *)

(* export
type
    Attributes = record
	font	   : integer;
	color      : ColorValue;
	bkgndColor : ColorValue;
	paintType  : PaintType;
	pattern    : integer;
	rendering  : RenderingStyle;
	lineStyle  : integer;
	curPos     : PointType;
	origin	   : PointType;
	userData   : Word;
    end;
 *)
(* export
type
    LineType = record
	p1, p2  : PointType;
    end;
 *)
(* export
type
    CircleType = record
	center  : PointType;
     	radius	: integer;
    end;
 *)
(* export
type
    EllipseType = record
        center : PointType;
	r1, r2 : integer;
    end;
 *)
(* export type EllipseSplineX = array [1..8] of XCoord; *)
(* export type EllipseSplineY = array [1..8] of YCoord; *)
(* export
type
    RectType = record
	lowLeft, upRight : PointType;
    end;
 *)
(* export type RectPolyX = array [1..5] of XCoord; *)
(* export type RectPolyY = array [1..5] of YCoord; *)
(* export
type
    VertexArray = record
        x : dynarray of XCoord;
	y : dynarray of YCoord;
    end;
 *)
(* export
type
    TextType = record
	start	    : PointType;
	chars       : dynarray of char;
    end;			        
 *)
(* export
type
    FloatPointType = record
        x, y : real;
    end;
 *)
(* export
type
    BoundingBox = record
        lowLeft, upRight : PointType;
	center : FloatPointType;
    end;
 *)
(* export type BboxPtr = pointer to BoundingBox; *)
(* export type TransformMatrix = array [1..3], [1..2] of real; *)
(* export type XformPtr = pointer to TransformMatrix; *)

(* public export type Picture = pointer to PictureRecord; *)
(* export
type
    PictureRecord = record
        altered    : boolean;
	enabled	   : boolean;
	destroyed  : boolean;
	redraw     : boolean;
	attrib	   : Attributes;
	bbox	   : BoundingBox;
	xform      : XformPtr;
	parent     : Picture;
	eraseMask  : Picture;
	complexity : [0..MAXCOMPLEXITY];

        case objType : PictureObjectType of
	    PointObj:    point    : PointType    |
	    LineObj:	 line	  : LineType     |
	    CircleObj:	 circle	  : CircleType   |
	    EllipseObj:  ellipse  : EllipseType  |
	    RectObj:	 rect	  : RectType     |
	    PolygonObj,
	    BSplineObj,
	    ClosedBSplineObj:
	    		 vertex	  : VertexArray  |
 	    TextObj:	 text	  : TextType     |
	    PictureList: pictList : GenericList
        end;
    end;
 *)

(* public export var XMAXSCREEN : XCoord; *)
(* public export var YMAXSCREEN : YCoord; *)

(* public export var solidPattern : Pattern; *)
(* public export var clearPattern : Pattern; *)
(* public export var vertPattern : Pattern; *)
(* public export var horizPattern : Pattern; *)
(* public export var majorDiagPattern : Pattern; *)
(* public export var minorDiagPattern : Pattern; *)
(* public export var thickVertPattern : Pattern; *)
(* public export var thickHorizPattern : Pattern; *)
(* public export var thickMajorDiagPattern : Pattern; *)
(* public export var thickMinorDiagPattern : Pattern; *)
(* public export var stipplePattern1 : Pattern; *)
(* public export var stipplePattern2 : Pattern; *)
(* public export var stipplePattern3 : Pattern; *)
(* public export var stipplePattern4 : Pattern; *)
(* public export var stipplePattern5 : Pattern; *)
(* public export var stipplePattern6 : Pattern; *)

(* export var identityMatrix : TransformMatrix; *)
(* export var originX : XCoord; *)
(* export var originY : YCoord; *)


(* public export *)
procedure InitGraphics();
begin
    vdi.InitPlace(0, YMAXSCREEN, XMAXSCREEN, YMAXSCREEN);
end InitGraphics;


(* public export *)
procedure ExitGraphics();
begin
    vdi.Finishup();
end ExitGraphics;


(* public export *)
procedure DefPattern(var p : PatternData) : Pattern;
begin
    return vdi.DefPattern(p);
end DefPattern;


(* public export *)
procedure DefLineStyle(const i : integer) : LineStyle;
begin
    return vdi.DefLineStyle(i);
end DefLineStyle;


(* public export *)
procedure LoadFont(const aoc : array of char) : Font;
begin
    return vdi.LoadFont(aoc);
end LoadFont;


(* public export *)
procedure GetFontHeight(n : integer) : integer;
begin
    return vdi.GetFontHeight(n);
end GetFontHeight;



begin
    XMAXSCREEN := vdi.XMAXSCREEN;
    YMAXSCREEN := vdi.YMAXSCREEN;

    solidPattern := 1;
    clearPattern := 2;
    vertPattern := vdi.vertPattern;
    horizPattern := vdi.horizPattern;
    majorDiagPattern := vdi.majorDiagPattern;
    minorDiagPattern := vdi.minorDiagPattern;
    thickVertPattern := vdi.thickVertPattern;
    thickHorizPattern := vdi.thickHorizPattern;
    thickMajorDiagPattern := vdi.thickMajorDiagPattern;
    thickMinorDiagPattern := vdi.thickMinorDiagPattern;
    stipplePattern1 := vdi.stipplePattern1;
    stipplePattern2 := vdi.stipplePattern2;
    stipplePattern3 := vdi.stipplePattern3;
    stipplePattern4 := vdi.stipplePattern4;
    stipplePattern5 := vdi.stipplePattern5;
    stipplePattern6 := vdi.stipplePattern6;

    originX := 0;
    originY := 0;

    identityMatrix[1, 1] := 1.0;
    identityMatrix[1, 2] := 0.0;
    identityMatrix[2, 1] := 0.0;
    identityMatrix[2, 2] := 1.0;
    identityMatrix[3, 1] := 0.0;
    identityMatrix[3, 2] := 0.0;
end main.
