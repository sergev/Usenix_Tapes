implementation module util;

(* export *)
from main import
    Picture, XCoord, YCoord, PointType, FloatPointType, RectPolyX, 
    RectPolyY, RectType, EllipseSplineX, EllipseSplineY, EllipseType,
    GenericList, TransformMatrix;

from main import
    PictureObjectType;

from xforms import
    TransformPoint, TransformX, TransformY, TransformVerteces;

from math import
    sqrt;

import
    genlists;


const ELLIPSEAXISFRAC = 0.42;	(* fraction of axis used to obtain cntrl pt *)
const ELLIPSEENLARGE = 1.025;	(* enlargement to make spline ~= ellipse *)


(* export *)
procedure round(const x : real) : integer;
begin
    if x < 0.0 then
	return -trunc(-x + 0.5);
    else
        return trunc(x + 0.5);
    end;
end round;


(* export *)
procedure PointsIdentical(const p1, p2 : PointType) : boolean;
begin
    return ((p1.x = p2.x) and (p1.y = p2.y));
end PointsIdentical;


(* export *)
procedure FloatPointsIdentical(const p1, p2 : FloatPointType) : boolean;
begin
    return ((p1.x = p2.x) and (p1.y = p2.y));
end FloatPointsIdentical;


(* export *)
procedure Distance(const p1, p2 : PointType) : integer;
    var deltaX : XCoord;
        deltaY : YCoord;
begin
    deltaX := p1.x - p2.x;
    deltaY := p1.y - p2.y;
    return round(sqrt(float(deltaX*deltaX + deltaY*deltaY)));
end Distance;


(* export *)
procedure FloatDistance(const p1, p2 : FloatPointType) : real;
    var deltaX, deltaY : real;
begin
    deltaX := p1.x - p2.x;
    deltaY := p1.y - p2.y;
    return sqrt(float(deltaX*deltaX + deltaY*deltaY));
end FloatDistance;


(* export *)
procedure RectToPolygon(
    const rect : RectType;
    const xform : TransformMatrix;
    var   rectX : RectPolyX;
    var   rectY : RectPolyY;
    const offsetX : XCoord;
    const offsetY : YCoord
);
    var lowLeft, upRight : PointType;
begin
    lowLeft := TransformPoint(rect.lowLeft, xform);
    upRight := TransformPoint(rect.upRight, xform);

    rectX[1] := lowLeft.x + offsetX;
    rectX[2] := TransformX(rect.lowLeft.x, rect.upRight.y, xform) + offsetX;
    rectX[3] := upRight.x + offsetX;
    rectX[4] := TransformX(rect.upRight.x, rect.lowLeft.y, xform) + offsetX;
    rectX[5] := rectX[1];
	
    rectY[1] := lowLeft.y + offsetY;
    rectY[2] := TransformY(rect.lowLeft.x, rect.upRight.y, xform) + offsetY;
    rectY[3] := upRight.y + offsetY;
    rectY[4] := TransformY(rect.upRight.x, rect.lowLeft.y, xform) + offsetY;
    rectY[5] := rectY[1];
end RectToPolygon;


(* export *)
procedure EllipseToSpline(
    const ellipse : EllipseType;
    const xform : TransformMatrix;
    var   x : EllipseSplineX;
    var   y : EllipseSplineY;
    const offsetX : XCoord;
    const offsetY : YCoord
);
    var i, cpx1, cpx2, cpy1, cpy2 : integer;
begin
    cpx1 := round(float(ellipse.r1) * ELLIPSEAXISFRAC);
    cpx2 := round(float(ellipse.r1) * ELLIPSEENLARGE);
    cpy1 := round(float(ellipse.r2) * ELLIPSEAXISFRAC);
    cpy2 := round(float(ellipse.r2) * ELLIPSEENLARGE);
    x[1] := ellipse.center.x + cpx1;
    x[6] := x[1];
    x[2] := ellipse.center.x - cpx1;
    x[5] := x[2];
    x[3] := ellipse.center.x - cpx2;
    x[4] := x[3];
    x[7] := ellipse.center.x + cpx2;
    x[8] := x[7];
    y[1] := ellipse.center.y + cpy2;
    y[2] := y[1];
    y[5] := ellipse.center.y - cpy2;
    y[6] := y[5];
    y[3] := ellipse.center.y + cpy1;
    y[8] := y[3];
    y[4] := ellipse.center.y - cpy1;
    y[7] := y[4];

    TransformVerteces(x, y, xform);
    for i := 1 to 8 do
	x[i] := x[i] + offsetX;
	y[i] := y[i] + offsetY;
    end;
end EllipseToSpline;


(* export *)
procedure OrthoRect(
    const x0 : XCoord; const y0 : YCoord;
    const x1 : XCoord; const y1 : YCoord;
    const x2 : XCoord; const y2 : YCoord;
    const x3 : XCoord; const y3 : YCoord
) : boolean;
begin
    return (
        ((x0 = x3) and (x1 = x2) and (y0 = y1) and (y2 = y3)) or
	((x0 = x1) and (x2 = x3) and (y0 = y3) and (y1 = y2))
    );
end OrthoRect;


(* export *)
procedure Root(const picture : Picture) : Picture;
begin
    if (picture = Picture(nil)) or (picture^.parent = Picture(nil)) then
        return picture;
    else
        return Root(picture^.parent);
    end;
end Root;


(*
 * GetPictList returns the pictList in the given picture that contains the
 * given subpicture.
 *)
(* export *)
procedure GetPictList(
    const picture : Picture;
    const subPicture : Picture;
    var   found : boolean
) : GenericList;
    var i : genlists.Iterator;
        pictList : GenericList;
	subpict : Picture;
begin
    i := genlists.BeginIteration(picture^.pictList);
    while genlists.MoreElements(i, subpict) do
        if subpict^.objType = PictureList then
	    if subpict = subPicture then
		genlists.EndIteration(i);
	        found := true;
	        return picture^.pictList;
	    else
	        pictList := GetPictList(subpict, subPicture, found);
	    end;
    	end;
    end;
    genlists.EndIteration(i);
    return GenericList(nil);
end GetPictList;
	    

(* export *)
procedure StringsEqual(const a, b : array of char) : boolean;
    var k, n : integer;
begin
    n := number(a);
    if n # number(b) then
        return false;
    end;
    k := 1;
    while k < n do
 	if a[k] # b[k] then
	    return false;
	else
	    k := k + 1;
        end;
    end;
    return true;
end StringsEqual;


begin

end util.
