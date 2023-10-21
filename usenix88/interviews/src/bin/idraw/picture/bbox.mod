implementation module bbox;

(* export *)
from main import
    Picture, XCoord, YCoord, BoundingBox, RectPolyX, RectPolyY,
    TransformMatrix, GenericList;

from main import
    PictureObjectType, RectType;
    
from util import
    RectToPolygon, PointsIdentical;
    
from xforms import
    TransformBbox;

import 
    genlists,
    vdi;


(* export *)
procedure LoadBbox(
    var   bbox : BoundingBox;
    const x0 : XCoord;
    const y0 : YCoord;
    const x1 : XCoord;
    const y1 : YCoord
);
begin
    bbox.lowLeft.x := min(x0, x1);
    bbox.lowLeft.y := min(y0, y1);
    bbox.upRight.x := max(x0, x1);
    bbox.upRight.y := max(y0, y1);
    bbox.center.x := float(x1 + x0) / 2.0;
    bbox.center.y := float(y1 + y0) / 2.0;
end LoadBbox;


(* export *)
procedure PointInBbox(
    const px : XCoord;
    const py : YCoord; 
    const bbox : BoundingBox
) : boolean;
begin
    return (  
        (px >= bbox.lowLeft.x) and (px <= bbox.upRight.x) and 
        (py >= bbox.lowLeft.y) and (py <= bbox.upRight.y)
    );
end PointInBbox;


(* export *)
procedure BboxesIntersect(
    const bbox1, bbox2 : BoundingBox
) : boolean;
begin
    return (
        (bbox1.lowLeft.x <= bbox2.upRight.x) and
        (bbox2.lowLeft.x <= bbox1.upRight.x) and 
	(bbox1.lowLeft.y <= bbox2.upRight.y) and 
	(bbox2.lowLeft.y <= bbox1.upRight.y) 
    ) or (
        (bbox2.lowLeft.x <= bbox1.upRight.x) and
        (bbox1.lowLeft.x <= bbox2.upRight.x) and
        (bbox2.lowLeft.y <= bbox1.upRight.y) and 
	(bbox1.lowLeft.y <= bbox2.upRight.y) 
    );
end BboxesIntersect;


(* export *)
procedure BboxWithin(
    const bbox1, bbox2 : BoundingBox
) : boolean;
begin
    return (
        (bbox1.lowLeft.x >= bbox2.lowLeft.x) and
	(bbox1.lowLeft.y >= bbox2.lowLeft.y) and 
        (bbox1.upRight.x <= bbox2.upRight.x) and 
	(bbox1.upRight.y <= bbox2.upRight.y) 
    );
end BboxWithin;


(* export *)
procedure MergeBboxes(
    const bbox1, bbox2 : BoundingBox
) : BoundingBox;
    var newBbox : BoundingBox;
begin
    if
        (bbox1.lowLeft.x = bbox1.upRight.x) and 
	(bbox1.lowLeft.y = bbox1.upRight.y)
    then
        newBbox := bbox2;
    elsif
        (bbox2.lowLeft.x = bbox2.upRight.x) and
	(bbox2.lowLeft.y = bbox2.upRight.y)
    then
        newBbox := bbox1;
    else
        newBbox.lowLeft.x := min(bbox1.lowLeft.x, bbox2.lowLeft.x);
        newBbox.lowLeft.y := min(bbox1.lowLeft.y, bbox2.lowLeft.y);
        newBbox.upRight.x := max(bbox1.upRight.x, bbox2.upRight.x);
        newBbox.upRight.y := max(bbox1.upRight.y, bbox2.upRight.y);
	newBbox.center.x := float(newBbox.lowLeft.x + newBbox.upRight.x) / 2.0;
	newBbox.center.y := float(newBbox.lowLeft.y + newBbox.upRight.y) / 2.0;
    end;
    return newBbox;
end MergeBboxes;


(* export *)
procedure IntersectBboxes(
    const bbox1, bbox2 : BoundingBox
) : BoundingBox;
    var newBbox : BoundingBox;
begin
    if not BboxesIntersect(bbox1, bbox2) then
        newBbox.lowLeft.x := 0;
	newBbox.lowLeft.y := 0;
	newBbox.upRight.x := 0;
	newBbox.upRight.y := 0;
    elsif
        (bbox1.lowLeft.x = bbox1.upRight.x) and 
	(bbox1.lowLeft.y = bbox1.upRight.y)
    then
        newBbox := bbox1;
    elsif
        (bbox2.lowLeft.x = bbox2.upRight.x) and
	(bbox2.lowLeft.y = bbox2.upRight.y)
    then
        newBbox := bbox2;
    else
        newBbox.lowLeft.x := max(bbox1.lowLeft.x, bbox2.lowLeft.x);
        newBbox.lowLeft.y := max(bbox1.lowLeft.y, bbox2.lowLeft.y);
        newBbox.upRight.x := min(bbox1.upRight.x, bbox2.upRight.x);
        newBbox.upRight.y := min(bbox1.upRight.y, bbox2.upRight.y);
	newBbox.center.x := float(newBbox.lowLeft.x + newBbox.upRight.x) / 2.0;
	newBbox.center.y := float(newBbox.lowLeft.y + newBbox.upRight.y) / 2.0;
    end;
    return newBbox;
end IntersectBboxes;


(* export *)
procedure CalcPolygonBbox(
    const x : array of XCoord;
    const y : array of YCoord
) : BoundingBox;
    var i : integer;
	minX, maxX : XCoord;
	minY, maxY : YCoord;
	bbox : BoundingBox;
begin
    minX := x[0];
    minY := y[0];
    maxX := x[0];
    maxY := y[0];

    for i := 1 to high(x) do
	minX := min(minX, x[i]);
	minY := min(minY, y[i]);
	maxX := max(maxX, x[i]);
	maxY := max(maxY, y[i]);
    end;
    LoadBbox(bbox, minX, minY, maxX, maxY);
    return bbox;
end CalcPolygonBbox;


(* export *)
procedure SizeTextBbox(const picture : Picture);
    var size : integer;
begin
    if picture^.objType # TextObj then
        return;
    end;

    vdi.SetFont(picture^.attrib.font);
    size := number(picture^.text.chars^);
    LoadBbox(
        picture^.bbox,
	picture^.text.start.x, 
	picture^.text.start.y,
	picture^.text.start.x + vdi.StrWidth(picture^.text.chars^[0:size]),
	picture^.text.start.y + vdi.GetHeight()
    );
end SizeTextBbox;	    
    

(* export *)
procedure UpdateParentBboxes(const picture : Picture);
begin
    if picture^.parent # Picture(nil) then
        picture^.parent^.bbox := CalcBbox(picture^.parent);
	UpdateParentBboxes(picture^.parent);
    end;
end UpdateParentBboxes;


(* export *)
procedure BboxToPolygon(
    const bbox : BoundingBox;
    const xform : TransformMatrix;
    var   rectX : RectPolyX;
    var   rectY : RectPolyY
);
    var tempRect : RectType;
begin
    tempRect.lowLeft := bbox.lowLeft;
    tempRect.upRight := bbox.upRight;
    RectToPolygon(tempRect, xform, rectX, rectY, 0, 0);
end BboxToPolygon;


(* export *)
procedure PolygonToBbox(
    const rectX : RectPolyX;
    const rectY : RectPolyY
) : BoundingBox;
    var i : integer;
        bbox : BoundingBox;
begin
    bbox.lowLeft.x := rectX[1];
    bbox.lowLeft.y := rectY[1];
    bbox.upRight := bbox.lowLeft;
    for i := 1 to 4 do
	bbox.lowLeft.x := min(bbox.lowLeft.x, rectX[i]);
	bbox.lowLeft.y := min(bbox.lowLeft.y, rectY[i]);
	bbox.upRight.x := max(bbox.upRight.x, rectX[i]);
	bbox.upRight.y := max(bbox.upRight.y, rectY[i]);
    end;
    return bbox;
end PolygonToBbox;


(*
 * CalcBbox calculates the bounding box of either (1) a primitive picture
 * (ie, rectangle, circle), or (2) combines the bounding boxes of
 * the given picture's list of primitive pictures.
 *)
(* export *)
procedure CalcBbox(const picture : Picture) : BoundingBox;
    var i : genlists.Iterator;
	bbox : BoundingBox;
	subpict : Picture;
begin
    bbox.upRight := bbox.lowLeft;
    if picture^.destroyed or not picture^.enabled then
        return bbox;
    elsif picture^.objType = PictureList then
        i := genlists.BeginIteration(picture^.pictList);
	while genlists.MoreElements(i, subpict) do
	    if not subpict^.destroyed and subpict^.enabled then
		if subpict^.xform = nil then
		    bbox := MergeBboxes(bbox, subpict^.bbox);
		else
	            bbox := MergeBboxes(
		        bbox, TransformBbox(subpict^.bbox, subpict^.xform^)
		    );
		end;
	    end;
	end;
        genlists.EndIteration(i);
    else
        bbox := TransformBbox(picture^.bbox, picture^.xform^);
    end;
    return bbox;
end CalcBbox;



begin

end bbox.
