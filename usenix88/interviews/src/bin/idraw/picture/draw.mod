implementation module draw;

(* export *)
from main import
    Picture, XCoord, YCoord;
    
from main import
    PictureObjectType, BoundingBox, identityMatrix,
    RectPolyX, RectPolyY, EllipseSplineX, EllipseSplineY,
    originX, originY, PointType, FloatPointType, EllipseType,
    RenderingStyle, PaintType;

from redraw import
    UpdateEraseMask;
    
import 
    genlists,
    vdi;

from xforms import
    NoRotation;
    
from ops import
    DestroyPict;
    
from util import
    RectToPolygon, EllipseToSpline, Root, Distance, FloatDistance,
    OrthoRect, round;
 
from bbox import
    BboxesIntersect, CalcBbox, UpdateParentBboxes;

from math import
    sqrt;


procedure DrawPoint(const picture : Picture);
    var p : PointType;
        px, py : real;
begin
    with picture^ do
        px := float(point.x);
	py := float(point.y);
	p.x := round(px * xform^[1,1] + py * xform^[2,1] + xform^[3,1]);
	p.y := round(px * xform^[1,2] + py * xform^[2,2] + xform^[3,2]);
    end;
    vdi.Point(p.x + originX, p.y + originY);
end DrawPoint;


procedure DrawLine(const picture : Picture);
    var p1, p2 : PointType;
        p1x, p1y, p2x, p2y : real;
begin
    with picture^ do
	p1x := float(line.p1.x);
	p1y := float(line.p1.y);
	p2x := float(line.p2.x);
	p2y := float(line.p2.y);
	p1.x := round(p1x * xform^[1,1] + p1y * xform^[2,1] + xform^[3,1]);
	p1.y := round(p1x * xform^[1,2] + p1y * xform^[2,2] + xform^[3,2]);
	p2.x := round(p2x * xform^[1,1] + p2y * xform^[2,1] + xform^[3,1]);
	p2.y := round(p2x * xform^[1,2] + p2y * xform^[2,2] + xform^[3,2]);
    end;
    vdi.Line(p1.x + originX, p1.y + originY, p2.x + originX, p2.y + originY);
end DrawLine;


procedure DrawCircle(const p : Picture);
    var c, p1, p2, p1n, p2n : FloatPointType;
        ci : PointType;
	dx1, dy1, dx2, dy2 : real;
	r1, r2 : integer;
	circular : boolean;
	ellipse : EllipseType;
begin
    ci.x := round(
        float(p^.circle.center.x) * p^.xform^[1,1] + 
	float(p^.circle.center.y) * p^.xform^[2,1] + p^.xform^[3,1]
    );
    ci.y := round(
        float(p^.circle.center.x) * p^.xform^[1,2] +
	float(p^.circle.center.y) * p^.xform^[2,2] + p^.xform^[3,2]
    );

    circular := true;

    if
        (p^.xform^[1,1] = 1.0) and (p^.xform^[1,2] = 0.0) and
	(p^.xform^[2,1] = 0.0) and (p^.xform^[2,2] = 1.0) and
	(p^.xform^[3,1] = 0.0) and (p^.xform^[3,2] = 0.0)
    then
        r1 := p^.circle.radius;
    else
        p1.x := float(p^.circle.center.x);
	p1.y := float(p^.circle.center.y);
	c.x := p1.x * p^.xform^[1,1] + p1.y * p^.xform^[2,1] + p^.xform^[3,1];
	c.y := p1.x * p^.xform^[1,2] + p1.y * p^.xform^[2,2] + p^.xform^[3,2];
	p2 := p1;
        p1.x := p1.x + float(p^.circle.radius);
        p1n.x := p1.x * p^.xform^[1,1] + p1.y * p^.xform^[2,1] + p^.xform^[3,1];
	p1n.y := p1.x * p^.xform^[1,2] + p1.y * p^.xform^[2,2] + p^.xform^[3,2];
	p2.y := p2.y + float(p^.circle.radius);
	p2n.x := p2.x * p^.xform^[1,1] + p2.y * p^.xform^[2,1] + p^.xform^[3,1];
	p2n.y := p2.x * p^.xform^[1,2] + p2.y * p^.xform^[2,2] + p^.xform^[3,2];
	
	dx1 := c.x - p1n.x;
	dy1 := c.y - p1n.y;
	dx2 := c.x - p2n.x;
	dy2 := c.y - p2n.y;
	r1 := trunc(sqrt(dx1*dx1 + dy1*dy1) + 0.5);
	r2 := trunc(sqrt(dx2*dx2 + dy2*dy2) + 0.5);
	if r1 # r2 then
	    circular := false;
	end;
    end;
    
    if circular then
        if p^.attrib.rendering = Fill then
            vdi.FilledCircle(ci.x + originX, ci.y + originY, r1);
        else
            vdi.Circle(ci.x + originX, ci.y + originY, r1);
        end;
    else
	ellipse.center := p^.circle.center;
	ellipse.r1 := p^.circle.radius;
	ellipse.r2 := p^.circle.radius;
	p^.objType := EllipseObj;
	p^.ellipse := ellipse;
	DrawEllipse(p);
    end;
end DrawCircle;


procedure DrawEllipse(const p : Picture);
    var c, pr1, pr2, pr1n, pr2n : PointType;
        i, r1, r2 : integer;
	x : EllipseSplineX;
	y : EllipseSplineY;
	noXform : boolean;
begin
    noXform := 
        (p^.xform^[1,1] = 1.0) and (p^.xform^[1,2] = 0.0) and
	(p^.xform^[2,1] = 0.0) and (p^.xform^[2,2] = 1.0) and
	(p^.xform^[3,1] = 0.0) and (p^.xform^[3,2] = 0.0);
    
    if NoRotation(p^.xform^) or noXform then
	c.x := round(
	    float(p^.ellipse.center.x) * p^.xform^[1,1] + 
	    float(p^.ellipse.center.y) * p^.xform^[2,1] + p^.xform^[3,1]
	);
	c.y := round(
	    float(p^.ellipse.center.x) * p^.xform^[1,2] +
	    float(p^.ellipse.center.y) * p^.xform^[2,2] + p^.xform^[3,2]
	);

	if noXform then
	    r1 := p^.ellipse.r1;
	    r2 := p^.ellipse.r2;
	else
	    pr1.x := p^.ellipse.center.x + p^.ellipse.r1;
	    pr1n.x := round(
	        float(pr1.x) * p^.xform^[1,1] + 
	        float(pr1.y) * p^.xform^[2,1] + p^.xform^[3,1]
	    );
	    r1 := pr1n.x - c.x;
	    pr2.y := p^.ellipse.center.y + p^.ellipse.r2;
	    pr2n.y := round(
	        float(pr2.x) * p^.xform^[1,2] +
	        float(pr2.y) * p^.xform^[2,2] + p^.xform^[3,2]
	    );
	    r2 := pr2n.y - c.y;
	end;
    
	if p^.attrib.rendering = Fill then
	    vdi.FilledEllipse(c.x + originX, c.y + originY, r1, r2);
	else
	    vdi.Ellipse(c.x + originX, c.y + originY, r1, r2);
	end;
    else
	EllipseToSpline(p^.ellipse, p^.xform^, x, y, originX, originY);
	if p^.attrib.rendering = Fill then
	    vdi.FilledBSpline(x, y);
	else
	    vdi.ClosedBSpline(x, y);
	end;
    end;
end DrawEllipse;


procedure DrawRect(const p : Picture);
    var x0, x1, x2, x3 : XCoord;
        y0, y1, y2, y3 : YCoord;
    	rectX : RectPolyX;
	rectY : RectPolyY;
	noRot : boolean;
begin
    noRot := NoRotation(p^.xform^);
    with p^.rect do
        x0 := round(
	    float(lowLeft.x) * p^.xform^[1,1] + 
	    float(lowLeft.y) * p^.xform^[2,1] + p^.xform^[3,1]
	);
	y0 := round(
	    float(lowLeft.x) * p^.xform^[1,2] +
	    float(lowLeft.y) * p^.xform^[2,2] + p^.xform^[3,2]
	);
        x2 := round(
	    float(upRight.x) * p^.xform^[1,1] + 
	    float(upRight.y) * p^.xform^[2,1] + p^.xform^[3,1]
	);
	y2 := round(
	    float(upRight.x) * p^.xform^[1,2] +
	    float(upRight.y) * p^.xform^[2,2] + p^.xform^[3,2]
	);
	if not noRot then
            x1 := round(
	        float(upRight.x) * p^.xform^[1,1] + 
	        float(lowLeft.y) * p^.xform^[2,1] + p^.xform^[3,1]
	    );
	    y1 := round(
	        float(upRight.x) * p^.xform^[1,2] +
	        float(lowLeft.y) * p^.xform^[2,2] + p^.xform^[3,2]
	    );
            x3 := round(
	        float(lowLeft.x) * p^.xform^[1,1] + 
	        float(upRight.y) * p^.xform^[2,1] + p^.xform^[3,1]
	    );
	    y3 := round(
	        float(lowLeft.x) * p^.xform^[1,2] +
	        float(upRight.y) * p^.xform^[2,2] + p^.xform^[3,2]
	    );
	end;
    end;
    if noRot or OrthoRect(x0, y0, x1, y1, x2, y2, x3, y3) then
 	if p^.attrib.rendering = Fill then
	    vdi.FilledRectangle(
	        x0 + originX, y0 + originY, x2 + originX, y2 + originY
	    );
	else
	    vdi.Rectangle(
	        x0 + originX, y0 + originY, x2 + originX, y2 + originY
	    );
	end;
    else
        RectToPolygon(
	    p^.rect, p^.xform^, rectX, rectY, originX, originY
	);
	if p^.attrib.rendering = Fill then
	    vdi.Polygon(rectX, rectY);
	else
	    vdi.LineList(rectX, rectY);
	end;
    end;
end DrawRect;


procedure DrawPolygon(const p : Picture);
    var polyX : dynarray of XCoord;
	polyY : dynarray of YCoord;
	i, maxVal, size : integer;
begin
    maxVal := high(p^.vertex.x);
    new(polyX, maxVal + 1);
    new(polyY, maxVal + 1);
    size := number(polyX);
    for i := 0 to maxVal do
	polyX^[i] := round(
	    float(p^.vertex.x^[i]) * p^.xform^[1,1] + 
	    float(p^.vertex.y^[i]) * p^.xform^[2,1] + p^.xform^[3,1]
	) + originX;
	polyY^[i] := round(
	    float(p^.vertex.x^[i]) * p^.xform^[1,2] + 
	    float(p^.vertex.y^[i]) * p^.xform^[2,2] + p^.xform^[3,2]
	) + originY;
    end;
    if p^.attrib.rendering = Fill then
        vdi.Polygon(polyX^[0:size], polyY^[0:size]);
    else
        vdi.LineList(polyX^[0:size], polyY^[0:size]);
	if (polyX^[maxVal] # polyX^[0]) or (polyY^[maxVal] # polyY^[0]) then
	    vdi.Line(polyX^[maxVal], polyY^[maxVal], polyX^[0], polyY^[0]);
	end;
    end;
    dispose(polyX);
    dispose(polyY);
end DrawPolygon;


procedure DrawBSpline(const p : Picture);
    var polyX : dynarray of XCoord;
	polyY : dynarray of YCoord;
	i, maxVal, size : integer;
begin
    maxVal := high(p^.vertex.x);
    new(polyX, maxVal + 5);
    new(polyY, maxVal + 5);
    size := number(polyX^);
    for i := 0 to maxVal do
	polyX^[i + 2] := round(
	    float(p^.vertex.x^[i]) * p^.xform^[1,1] + 
	    float(p^.vertex.y^[i]) * p^.xform^[2,1] + p^.xform^[3,1]
	) + originX;
	polyY^[i + 2] := round(
	    float(p^.vertex.x^[i]) * p^.xform^[1,2] + 
	    float(p^.vertex.y^[i]) * p^.xform^[2,2] + p^.xform^[3,2]
	) + originY;
    end;
    maxVal := maxVal + 2;
    polyX^[0] := polyX^[2];	polyY^[0] := polyY^[2];
    polyX^[1] := polyX^[2];	polyY^[1] := polyY^[2];
    polyX^[maxVal + 1] := polyX^[maxVal];
    polyX^[maxVal + 2] := polyX^[maxVal];
    polyY^[maxVal + 1] := polyY^[maxVal];
    polyY^[maxVal + 2] := polyY^[maxVal];
    if p^.attrib.rendering = Fill then
        vdi.FilledBSpline(polyX^[0:size], polyY^[0:size]);
    else
        vdi.BSpline(polyX^[0:size], polyY^[0:size]);
    end;
    dispose(polyX);
    dispose(polyY);
end DrawBSpline;


procedure DrawClosedBSpline(const p : Picture);
    var polyX : dynarray of XCoord;
	polyY : dynarray of YCoord;
	i, size : integer;
begin
    new(polyX, number(p^.vertex.x));
    new(polyY, number(p^.vertex.y));
    size := number(polyX^);
    for i := 0 to high(p^.vertex.x) do
	polyX^[i] := round(
	    float(p^.vertex.x^[i]) * p^.xform^[1,1] + 
	    float(p^.vertex.y^[i]) * p^.xform^[2,1] + p^.xform^[3,1]
	) + originX;
	polyY^[i] := round(
	    float(p^.vertex.x^[i]) * p^.xform^[1,2] + 
	    float(p^.vertex.y^[i]) * p^.xform^[2,2] + p^.xform^[3,2]
	) + originY;
    end;
    if p^.attrib.rendering = Fill then
        vdi.FilledBSpline(polyX^[0:size], polyY^[0:size]);
    else
        vdi.ClosedBSpline(polyX^[0:size], polyY^[0:size]);
    end;
    dispose(polyX);
    dispose(polyY);
end DrawClosedBSpline;


procedure DrawText(const p : Picture);
    var start : PointType;
begin
    vdi.SetFont(p^.attrib.font);
    start.x := round(
	float(p^.text.start.x) * p^.xform^[1,1] + 
	float(p^.text.start.y) * p^.xform^[2,1] + p^.xform^[3,1]
    );
    start.y := round(
	float(p^.text.start.x) * p^.xform^[1,2] +
	float(p^.text.start.y) * p^.xform^[2,2] + p^.xform^[3,2]
    );
    if 
	(
	    (p^.attrib.pattern = vdi.clearPattern) or (p^.attrib.pattern = 2)
	) and 
	(p^.attrib.rendering = Fill) and
	(p^.attrib.color # vdi.WHITE)
    then
	vdi.SetForegroundColor(vdi.WHITE);
    end;
    vdi.MoveTo(start.x + originX, start.y + originY);
    vdi.CharStr(p^.text.chars^[0:number(p^.text.chars^)]);
end DrawText;


procedure DrawObject(const p : Picture);
begin
    if p^.attrib.rendering = Fill then
        vdi.SetPattern(p^.attrib.pattern);
    end;

    if p^.attrib.paintType = Opaque then
        vdi.SetColors(p^.attrib.color, p^.attrib.bkgndColor);
	vdi.SetRendering(vdi.PAINTFGBG);
    elsif p^.attrib.paintType = Transparent then
        vdi.SetForegroundColor(p^.attrib.color);
	vdi.SetRendering(vdi.PAINTFG);
    else
        vdi.SetRendering(vdi.INVERT);
    end;

    case p^.objType of
	PointObj:		DrawPoint(p)		|
	LineObj:		DrawLine(p) 		|
	CircleObj:		DrawCircle(p)		|
	EllipseObj:		DrawEllipse(p)		|
	RectObj:		DrawRect(p)		|
	PolygonObj:		DrawPolygon(p)		|
	BSplineObj:		DrawBSpline(p)		|
	ClosedBSplineObj:	DrawClosedBSpline(p)	|
	TextObj:   		DrawText(p)
    end;
end DrawObject;

procedure EnablePict(const p : Picture);
    var i : genlists.Iterator;
        subpict : Picture;
begin
    if p # Picture(nil) then
        p^.enabled := true;
	p^.altered := true;

	if p^.objType = PictureList then
	    i := genlists.BeginIteration(p^.pictList);
	    while genlists.MoreElements(i, subpict) do
	        EnablePict(subpict);
	    end;
	    genlists.EndIteration(i);
	    p^.bbox := CalcBbox(p);
	end;
    end;
end EnablePict;


(* public export *)
procedure Enable(const p : Picture);
begin
    if 
        (p # Picture(nil)) and 
	not p^.enabled and
	not p^.destroyed 
    then
        EnablePict(p);
	UpdateParentBboxes(p);
    end;
end Enable;


procedure DisablePict(const p : Picture);
    var i : genlists.Iterator;
        subpict : Picture;
begin
    if p # Picture(nil) then
        p^.enabled := false;
	p^.altered := true;

	if p^.objType = PictureList then
	    i := genlists.BeginIteration(p^.pictList);
	    while genlists.MoreElements(i, subpict) do
	        DisablePict(subpict);
	    end;
	    genlists.EndIteration(i);
	end;
    end;
end DisablePict;


(* public export *)
procedure Disable(const p : Picture);
begin
    if (p # Picture(nil)) and p^.enabled then
        UpdateEraseMask(p);
        DisablePict(p);
	UpdateParentBboxes(p);
    end;
end Disable;


(* export *)
procedure DrawPict(const p : Picture);
    var subpict : Picture;
        i : genlists.Iterator;
begin
    if (p = Picture(nil)) or not p^.enabled then
        return;
    end;
    
    if p^.objType = PictureList then
        i := genlists.BeginIteration(p^.pictList);
        while genlists.MoreElements(i, subpict) do
	    if subpict^.destroyed then
	        genlists.DeleteCur(i);
		DestroyPict(subpict);
	    else
   	        DrawPict(subpict);
	    end;
	end;
	genlists.EndIteration(i);
	DestroyPict(p^.eraseMask);
    else
	DrawObject(p);
    end;
    p^.altered := false;
end DrawPict;


(* public export *)
procedure Draw(const p : Picture);
    var root : Picture;
begin
    root := Root(p);
    originX := root^.attrib.origin.x;
    originY := root^.attrib.origin.y;
    DrawPict(p);
end Draw;


(* export *)
procedure DrawInBbox(
    const p : Picture;
    const x0	  : XCoord;
    const y0      : YCoord;
    const x1	  : XCoord;
    const y1	  : YCoord
);
    var i : genlists.Iterator;
        subpict : Picture;
        bbox, pictBbox : BoundingBox;
begin
    if (p = Picture(nil)) or not p^.enabled then
        return;
    end;

    bbox.lowLeft.x := x0;
    bbox.lowLeft.y := y0;
    bbox.upRight.x := x1;
    bbox.upRight.y := y1;
    if p^.objType # PictureList then
        pictBbox := CalcBbox(p);
    end;
    if  
        (p^.objType # PictureList) and
	BboxesIntersect(bbox, pictBbox)
    then
	vdi.Writable(
	    bbox.lowLeft.x + originX, bbox.lowLeft.y + originY,
	    bbox.upRight.x + originX, bbox.upRight.y + originY
	);
	DrawObject(p);
	vdi.AllWritable();
	p^.altered := false;
    elsif not BboxesIntersect(bbox, p^.bbox) then
        return;
    elsif p^.objType = PictureList then
        i := genlists.BeginIteration(p^.pictList);
        while genlists.MoreElements(i, subpict) do
	    if subpict^.destroyed then
	        genlists.DeleteCur(i);
		DestroyPict(subpict);
	    else
	        DrawInBbox(subpict, x0, y0, x1, y1);
	    end;
	end;
	genlists.EndIteration(i);
	p^.altered := false;
    end;
end DrawInBbox;


(* public export *)
procedure DrawInBoundingBox(
    const p : Picture;
    const x0	  : XCoord;
    const y0      : YCoord;
    const x1	  : XCoord;
    const y1	  : YCoord
);
    var root : Picture;
begin
    root := Root(p);
    originX := root^.attrib.origin.x;
    originY := root^.attrib.origin.y;
    DrawInBbox(p, x0, y0, x1, y1);
end DrawInBoundingBox;



begin

end draw.
