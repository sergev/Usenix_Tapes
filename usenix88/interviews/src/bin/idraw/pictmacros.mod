implementation module pictmacros;

import
    picture;

(* export *)
from picture import
    Picture, XCoord, YCoord;

from math import
    atan, sqrt;

from utilities import
    round, Deg, sign;

(* export *)
from dvdefs import
    LineType, VertexObjType;

from dvdefs import
    VertexObjProc;

from dvwindops import
    CopyIndicatorInfo;

from dvboot import
    ARROWWIDTH, ARROWHEIGHT;


(* export *)
procedure CreateSubpict(const pict : Picture) : Picture;
    var subpict : Picture;
        originX : XCoord;
	originY : YCoord;
begin
    picture.Create(subpict);
    picture.GetOrigin(pict, originX, originY);
    picture.SetOrigin(subpict, originX, originY);
    CopyIndicatorInfo(pict, subpict);
    picture.Nest(subpict, pict);
    return subpict;
end CreateSubpict;


(* export *)
procedure AddLine(
    const pict : Picture;
    const lineType : LineType;
    const x0 : XCoord;
    const y0 : YCoord;
    const x1 : XCoord;
    const y1 : YCoord
);
begin
    if lineType = None then
        return;
    end;

    picture.Line(pict, x0, y0, x1, y1);
    if lineType # Plain then
	case lineType of
	    LeftArrow:
		ArrowHead(pict, x0, y0, x1, y1, ARROWWIDTH, ARROWHEIGHT);
	        |
	    RightArrow:
		ArrowHead(pict, x1, y1, x0, y0, ARROWWIDTH, ARROWHEIGHT);
	    	|
	    DblArrow:
	    	ArrowHead(pict, x0, y0, x1, y1, ARROWWIDTH, ARROWHEIGHT);
		ArrowHead(pict, x1, y1, x0, y0, ARROWWIDTH, ARROWHEIGHT);
	end;
    end;
end AddLine;


(* export *)
procedure AddVertexObj(
    const p : Picture;
    const vertexObjType : VertexObjType;
    const lineType : LineType;
    const x : array of XCoord;
    const y : array of YCoord
);
    var vertexObjProc : VertexObjProc;
        rend : picture.RenderingStyle;
	a, b : integer;
begin
    rend := picture.GetRenderingStyle(p);
    if (rend = picture.Stroke) and (lineType = None) then
        return;
    end;

    case vertexObjType of
        Polygon:	vertexObjProc := picture.Polygon;|
	Spline:		vertexObjProc := picture.BSpline;|
	ClosedSpline:	vertexObjProc := picture.ClosedBSpline;
    end;
        
    if 	
        ((rend = picture.Fill) or (lineType # None)) and
	(
	    (vertexObjType # Spline) or	(lineType = Plain) or 
	    (lineType = None) or (rend = picture.Stroke)
	)
    then
	vertexObjProc(p, x, y);
    end;
    
    if (rend = picture.Fill) and (lineType # None) then
	picture.DefaultRenderingStyle(p, picture.Stroke);
	vertexObjProc(p, x, y);
    end;

    if 	
        (vertexObjType = Spline) and
        (lineType # None) and (lineType # Plain)
    then						(* add arrows *)
        a := high(x);
	b := a - 1;
	picture.DefaultRenderingStyle(p, rend);
	case lineType of
	    LeftArrow:
		ArrowHead(p, x[0], y[0], x[1], y[1], ARROWWIDTH, ARROWHEIGHT);
		|
	    RightArrow:
		ArrowHead(p, x[a], y[a], x[b], y[b], ARROWWIDTH, ARROWHEIGHT);
	    	|
	    DblArrow:
		ArrowHead(p, x[0], y[0], x[1], y[1], ARROWWIDTH, ARROWHEIGHT);
		ArrowHead(p, x[a], y[a], x[b], y[b], ARROWWIDTH, ARROWHEIGHT);

	end;
    end;
end AddVertexObj;


(* export *)
procedure ArrowHead(
    const pict : Picture;
    const tipX : XCoord;
    const tipY : YCoord;
    const tailX : XCoord;
    const tailY : YCoord;
    const width : integer;
    const height : integer
);
    var x : array [1..4] of XCoord;
        y : array [1..4] of YCoord;
        arrow : Picture;
	angle : real;
	dx : XCoord;
	dy : YCoord;
	rend : picture.RenderingStyle;
begin
    arrow := CreateSubpict(pict);
    x[1] := 0;
    y[1] := 0;
    x[2] := -height;
    y[2] := width div 2;
    x[3] := -height;
    y[3] := -width div 2;
    x[4] := x[1];
    y[4] := y[1];
    picture.Polygon(arrow, x, y);
    dx := tipX - tailX;
    dy := tipY - tailY;
    if dx = 0 then
        angle := float(sign(float(dy))) * 90.0;
    else
	angle := Deg(atan(float(dy) / float(dx)));
    end;
    if dx < 0 then
        angle := angle + float(sign(float(dy))) * 180.0;
    end;
    picture.RotateAboutPoint(arrow, angle, 0.0, 0.0);
    picture.Translate(arrow, float(tipX), float(tipY));

    if picture.GetRenderingStyle(pict) = picture.Fill then
	arrow := picture.Copy(arrow);
	picture.AlterRenderingStyle(arrow, picture.Stroke);
	picture.Nest(arrow, pict);
    end;
end ArrowHead;


begin
end pictmacros.
