implementation module xforms;

(* export *)
from main import
    Picture, XCoord, YCoord, TransformMatrix, FloatPointType, BoundingBox,
    ConcatOrder, PointType, RectPolyX, RectPolyY;

from main import
    identityMatrix, PictureObjectType;
    
from bbox import
    BboxToPolygon, PolygonToBbox, CalcBbox;

from math import
    sin, cos;

from util import
    round;

import
    genlists;

const
    PI 	= 3.1415927;


(* export *)
procedure TranslateMatrix(const dx, dy : real) : TransformMatrix;
    var xform : TransformMatrix;
begin
    xform := identityMatrix;
    xform[3, 1] := dx;
    xform[3, 2] := dy;
    return xform;
end TranslateMatrix;


(* export *)
procedure ScaleMatrix(const sx, sy : real) : TransformMatrix;
    var xform : TransformMatrix;
begin
    xform := identityMatrix;
    xform[1, 1] := sx;
    xform[2, 2] := sy;
    return xform;
end ScaleMatrix;


(* export *)
procedure RotateMatrix(const angle : real) : TransformMatrix;
    var xform : TransformMatrix;
        radAngle : real;
begin
    radAngle := angle * PI / 180.0;
    xform := identityMatrix;
    xform[1, 1] := cos(radAngle);
    xform[1, 2] := sin(radAngle);
    xform[2, 1] := -xform[1, 2];
    xform[2, 2] := xform[1, 1];
    return xform;
end RotateMatrix;
    

(* export *)
procedure FloatTransformX(
    const x, y : real;
    const xform : TransformMatrix
) : real;
begin
   return (
        x * xform[1, 1] +
	y * xform[2, 1] + xform[3, 1]
    );
end FloatTransformX;


(* export *)
procedure FloatTransformY(
    const x, y : real;
    const xform : TransformMatrix
) : real;
begin
    return (
        x * xform[1, 2] + 
	y * xform[2, 2] + xform[3, 2]
    );
end FloatTransformY;


(* export *)
procedure FloatTransformPoint(
    const p : FloatPointType;
    const xform : TransformMatrix
) : FloatPointType;
    var pNew : FloatPointType;
begin
    pNew.x := p.x * xform[1, 1] + p.y * xform[2, 1] + xform[3, 1];
    pNew.y := p.x * xform[1, 2] + p.y * xform[2, 2] + xform[3, 2];
    return pNew;
end FloatTransformPoint;


(* export *)
procedure TransformX(
    const x : XCoord;
    const y : YCoord;
    const xform : TransformMatrix
) : XCoord;
begin
    return round(
        float(x) * xform[1, 1] + float(y) * xform[2, 1] + xform[3, 1]
    );
end TransformX;


(* export *)
procedure TransformY(
    const x : XCoord;
    const y : YCoord;
    const xform : TransformMatrix
) : YCoord;
begin
    return round(
        float(x) * xform[1, 2] + float(y) * xform[2, 2] + xform[3, 2]
    );
end TransformY;


(* export *)
procedure TransformPoint(
    const p : PointType;
    const xform : TransformMatrix
) : PointType;
    var pNew : PointType;
begin
    pNew.x := round(
        float(p.x) * xform[1, 1] + float(p.y) * xform[2, 1] + xform[3, 1]
    );
    pNew.y := round(
        float(p.x) * xform[1, 2] + float(p.y) * xform[2, 2] + xform[3, 2]
    );
    return pNew;
end TransformPoint;


(* export *)
procedure TransformVerteces(
    var x : array of XCoord;
    var y : array of YCoord;
    const xform : TransformMatrix
);
    var i : integer;
        px : XCoord;
	py : YCoord;
begin
    for i := 0 to min(high(x), high(y)) do
        px := round(
	    float(x[i])*xform[1, 1] + float(y[i])*xform[2, 1] + xform[3, 1]
	);
        py := round(
	    float(x[i])*xform[1, 2] + float(y[i])*xform[2, 2] + xform[3, 2]
	);
	x[i] := px;
	y[i] := py;
    end;
end TransformVerteces;
        

(* export *)
procedure TransformBbox(
    const bbox : BoundingBox;
    const xform : TransformMatrix
) : BoundingBox;
    var newBbox : BoundingBox;
	rectX : RectPolyX;
	rectY : RectPolyY;
	lowLeft, upRight : PointType;
begin
    if NoRotation(xform) then
        lowLeft := TransformPoint(bbox.lowLeft, xform);
	upRight := TransformPoint(bbox.upRight, xform);
	newBbox.lowLeft.x := min(lowLeft.x, upRight.x);
	newBbox.lowLeft.y := min(lowLeft.y, upRight.y);
	newBbox.upRight.x := max(lowLeft.x, upRight.x);
	newBbox.upRight.y := max(lowLeft.y, upRight.y);
    else
        BboxToPolygon(bbox, xform, rectX, rectY);
	newBbox := PolygonToBbox(rectX, rectY);
    end;
    newBbox.center := FloatTransformPoint(bbox.center, xform);
    return newBbox;
end TransformBbox;


(* export *)
procedure MatricesIdentical(
    const matrix1, matrix2 : TransformMatrix
) : boolean;
    var i, j : integer;
begin
    for i := 1 to 3 do
 	for j := 1 to 2 do
	    if matrix1[i, j] <> matrix2[i, j] then
	        return false;
	    end;
	end;
    end;
    return true;
end MatricesIdentical;


(* export *)
procedure ConcatMatrices(
    const xform1, xform2 : TransformMatrix
) : TransformMatrix;
    var product : TransformMatrix;
        i, j    : integer;
begin
    for j := 1 to 2 do
	for i := 1 to 3 do
	    product[i, j] := 
	        xform1[i, 1] * xform2[1, j] + xform1[i, 2] * xform2[2, j];
	end;
    end;
    product[3, 1] := product[3, 1] + xform2[3, 1];
    product[3, 2] := product[3, 2] + xform2[3, 2];
    return product;
end ConcatMatrices;


(*
 * GlobalTransform recursively applies a global transformation
 * matrix to all elements of a picture's display list.
 *)
(* export *)
procedure GlobalTransform(
    const picture : Picture;
    const xform : TransformMatrix;
    const concatOrder : ConcatOrder
);
    var k : genlists.Iterator;
        subpict : Picture;
	i, j : integer;
	xform1, xform2 : TransformMatrix;
begin
    if MatricesIdentical(xform, identityMatrix) then
        return;
    end;

    picture^.altered := true;
    if picture^.objType = PictureList then
        k := genlists.BeginIteration(picture^.pictList);
        while genlists.MoreElements(k, subpict) do
            GlobalTransform(subpict, xform, concatOrder);
        end;
        genlists.EndIteration(k);
        picture^.bbox := CalcBbox(picture);
	return;
    end;

    if concatOrder = After then
	xform1 := picture^.xform^;
	xform2 := xform;
    else
        xform1 := xform;
	xform2 := picture^.xform^;
    end;
    for j := 1 to 2 do
        for i := 1 to 3 do
	    picture^.xform^[i, j] := 
	        xform1[i, 1] * xform2[1, j] + xform1[i, 2] * xform2[2, j];
        end;
    end;
    picture^.xform^[3, 1] := picture^.xform^[3, 1] + xform2[3, 1];
    picture^.xform^[3, 2] := picture^.xform^[3, 2] + xform2[3, 2];
end GlobalTransform;


(* export *)
procedure NoRotation(const xform : TransformMatrix) : boolean;
begin
    return (xform[1, 2] = 0.0) and (xform[2, 1] = 0.0);
end NoRotation;


begin

end xforms.
