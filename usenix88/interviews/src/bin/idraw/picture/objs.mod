implementation module objs;

(* export *)
from main import
    Picture, XCoord, YCoord;

from main import
    MAXCOMPLEXITY, PictureObjectType;
    
from ops import
    Create, InitPictObj;

from bbox import
    LoadBbox, MergeBboxes, UpdateParentBboxes, CalcPolygonBbox, SizeTextBbox;

from redraw import
    IncrementComplexity;

import 
    genlists;


(* public export *)
procedure Point(
    const picture : Picture;
    const x	  : XCoord;
    const y       : YCoord
);
    var subpict : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.altered := true;
    Create(subpict);
    InitPictObj(picture, subpict, PointObj);
    subpict^.point.x := x;
    subpict^.point.y := y;
    LoadBbox(subpict^.bbox, x, y, x + 1, y + 1);
    genlists.Append(subpict, picture^.pictList);
    IncrementComplexity(picture);
    picture^.bbox := MergeBboxes(picture^.bbox, subpict^.bbox);
    UpdateParentBboxes(picture);
end Point;


(* public export *)
procedure Line(
    const picture : Picture;
    const x0      : XCoord;
    const y0 	  : YCoord;
    const x1	  : XCoord;
    const y1      : YCoord
);
    var subpict : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.altered := true;
    Create(subpict);
    InitPictObj(picture, subpict, LineObj);
    subpict^.line.p1.x := x0;
    subpict^.line.p1.y := y0;
    subpict^.line.p2.x := x1;
    subpict^.line.p2.y := y1;
    LoadBbox(subpict^.bbox, x0, y0, x1, y1);
    genlists.Append(subpict, picture^.pictList);
    IncrementComplexity(picture);
    picture^.bbox := MergeBboxes(picture^.bbox, subpict^.bbox);
    UpdateParentBboxes(picture);
end Line;


(* public export *)
procedure LineTo(
    const picture : Picture;
    const x : XCoord;
    const y : YCoord
);
begin
    if picture = Picture(nil) then
        return;
    end;

    Line(picture, picture^.attrib.curPos.x, picture^.attrib.curPos.y, x, y);
    picture^.attrib.curPos.x := x;
    picture^.attrib.curPos.y := y;
end LineTo;


(* public export *)
procedure Circle(
    const picture : Picture;
    const x       : XCoord;
    const y       : YCoord;
    const radius  : integer
);
    var subpict : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.altered := true;
    Create(subpict);
    InitPictObj(picture, subpict, CircleObj);
    subpict^.circle.center.x := x;
    subpict^.circle.center.y := y;
    subpict^.circle.radius := radius;
    LoadBbox(subpict^.bbox, x - radius, y - radius, x + radius, y + radius);
    genlists.Append(subpict, picture^.pictList);
    IncrementComplexity(picture);
    picture^.bbox := MergeBboxes(picture^.bbox, subpict^.bbox);
    UpdateParentBboxes(picture);
end Circle;


(* public export *)
procedure Ellipse(
    const picture : Picture;
    const x       : XCoord;
    const y       : YCoord;
    const r1, r2  : integer
);
    var subpict : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.altered := true;
    Create(subpict);
    InitPictObj(picture, subpict, EllipseObj);
    subpict^.ellipse.center.x := x;
    subpict^.ellipse.center.y := y;
    subpict^.ellipse.r1 := r1;
    subpict^.ellipse.r2 := r2;
    LoadBbox(subpict^.bbox, x - r1, y - r2, x + r1, y + r2);
    genlists.Append(subpict, picture^.pictList);
    IncrementComplexity(picture);
    picture^.bbox := MergeBboxes(picture^.bbox, subpict^.bbox);
    UpdateParentBboxes(picture);
end Ellipse;


(* public export *)
procedure Rectangle(
    const picture : Picture;
    const x0      : XCoord;
    const y0      : YCoord;
    const x1      : XCoord;
    const y1      : YCoord
);
    var subpict : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.altered := true;
    Create(subpict);
    InitPictObj(picture, subpict, RectObj);
    subpict^.rect.lowLeft.x := x0;
    subpict^.rect.lowLeft.y := y0;
    subpict^.rect.upRight.x := x1;
    subpict^.rect.upRight.y := y1;
    LoadBbox(subpict^.bbox, x0, y0, x1, y1);
    genlists.Append(subpict, picture^.pictList);
    IncrementComplexity(picture);
    picture^.bbox := MergeBboxes(picture^.bbox, subpict^.bbox);
    UpdateParentBboxes(picture);
end Rectangle;


procedure BuildVertexObj(
    const picture : Picture;
    const objType : PictureObjectType;
    const x : array of XCoord;
    const y : array of YCoord
);
    var subpict : Picture;
    	i : integer;
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.altered := true;
    Create(subpict);
    InitPictObj(picture, subpict, objType);
    new(subpict^.vertex.x, number(x));
    new(subpict^.vertex.y, number(y));
    
    for i := 0 to high(x) do
        subpict^.vertex.x^[i] := x[i];
	subpict^.vertex.y^[i] := y[i];
    end;
    subpict^.bbox := CalcPolygonBbox(x, y);
    genlists.Append(subpict, picture^.pictList);
    IncrementComplexity(picture);
    picture^.bbox := MergeBboxes(picture^.bbox, subpict^.bbox);
    UpdateParentBboxes(picture);
end BuildVertexObj;


(* public export *)
procedure Polygon(
    const picture : Picture;
    const x       : array of XCoord;
    const y       : array of YCoord
);
begin
    BuildVertexObj(picture, PolygonObj, x, y);
end Polygon;


(* public export *)
procedure BSpline(
    const picture : Picture;
    const x       : array of XCoord;
    const y       : array of YCoord
);
begin
    BuildVertexObj(picture, BSplineObj, x, y);
end BSpline;


(* public export *)
procedure ClosedBSpline(
    const picture : Picture;
    const x       : array of XCoord;
    const y       : array of YCoord
);
begin
    BuildVertexObj(picture, ClosedBSplineObj, x, y);
end ClosedBSpline;


(* public export *)
procedure Text(
    const picture     : Picture;
    const chars       : array of char
);
    var subpict : Picture;
    	i : integer;
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.altered := true;
    Create(subpict);
    InitPictObj(picture, subpict, TextObj);
    subpict^.text.start := picture^.attrib.curPos;
    
    new(subpict^.text.chars, number(chars));

    for i := 0 to high(chars) do
        subpict^.text.chars^[i] := chars[i];
    end;
    
    SizeTextBbox(subpict);
    picture^.attrib.curPos.x := subpict^.bbox.upRight.x;
    picture^.attrib.curPos.y := subpict^.bbox.lowLeft.y;
    genlists.Append(subpict, picture^.pictList);
    picture^.complexity := MAXCOMPLEXITY;
    picture^.bbox := MergeBboxes(picture^.bbox, subpict^.bbox);
    UpdateParentBboxes(picture);
end Text;



begin

end objs.
