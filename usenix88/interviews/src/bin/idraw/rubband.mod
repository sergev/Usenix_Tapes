implementation module rubband;

import
    genlists;

(* export *)
from vdi import
    XCoord, YCoord, SetRendering, RenderingFunction,
    SetPattern, solidPattern,
    Rectangle, FilledRectangle, Circle, Ellipse, 
    Line, LineList, BezierArc, Writable, AllWritable;
    
from utilities import
    Distance, FloatDistance, round;

from math import
    sqrt;

var freelist : genlists.GenericList;

(* export const HANDLESIZE = 2; *)


(* export type Rubberband; *)
(* export
type
    RubberbandType = (
        RubberPerpLine, RubberLine, RubberRect, RubberCircle, RubberEllipse,
	SlidingLine, SlidingRect, RotatingRect, ScalingRect, StretchingRect,
	HandleRect, PolyVertexHandles, SplineVertexHandles,
	ClosedSplineVertexHandles
    );
 *)

type
    Vertex = record
        x : XCoord;
	y : YCoord;
    end;
    VertexArray = dynarray of Vertex;

    Rubberband = pointer to RubberbandRecord;
    RubberbandRecord = record
        rubType : RubberbandType;
	skipRate : cardinal;
	skipCount : cardinal;
	originX0 : XCoord;
	originY0 : YCoord;
	originX1 : XCoord;
	originY1 : YCoord;
	xc, yc : real;
	x, y : array [0..4] of real;
	vertex : VertexArray;
	rubberVertexSet : boolean;
	rubberVertexNo : integer;
	vertexHandlesDrawn : boolean;
        origX : XCoord;
	origY : YCoord;
	curX : XCoord;
	curY : YCoord;
	refptX : XCoord;
	refptY : YCoord;
	drawn : boolean;
    end;


procedure Transform(
    const r : Rubberband;
    const x0, y0, x1, y1, x2, y2 : real
);
    var i : integer;
        x : real;
begin
    for i := 0 to 3 do
	r^.x[i] := r^.x[i] - r^.xc;
	r^.y[i] := r^.y[i] - r^.yc;
        x := r^.x[i] * x0 + r^.y[i] * x1 + x2;
	r^.y[i] := r^.x[i] * y0 + r^.y[i] * y1 + y2;
	r^.x[i] := x;
	r^.x[i] := r^.x[i] + r^.xc;
	r^.y[i] := r^.y[i] + r^.yc;
    end;
    r^.x[4] := r^.x[0];
    r^.y[4] := r^.y[0];
end Transform;


procedure DrawSplineSection(
    const x : array of XCoord;
    const y : array of YCoord
);
    var twicex1, twicex2, p0x, p1x, p2x, p3x, tempx : real;
        twicey1, twicey2, p0y, p1y, p2y, p3y, tempy : real;
begin
    twicex1 := 2.0*float(x[1]);
    twicey1 := 2.0*float(y[1]);
    twicex2 := 2.0*float(x[2]);
    twicey2 := 2.0*float(y[2]);
    
    p1x := (twicex1 + float(x[2])) / 3.0;
    p1y := (twicey1 + float(y[2])) / 3.0;
    p2x := (twicex2 + float(x[1])) / 3.0;
    p2y := (twicey2 + float(y[1])) / 3.0;
    tempx := (twicex1 + float(x[0])) / 3.0;
    tempy := (twicey1 + float(y[0])) / 3.0;
    p0x := (tempx + p1x) / 2.0;
    p0y := (tempy + p1y) / 2.0;
    tempx := (twicex2 + float(x[3])) / 3.0;
    tempy := (twicey2 + float(y[3])) / 3.0;
    p3x := (tempx + p2x) / 2.0;
    p3y := (tempy + p2y) / 2.0;
    BezierArc(
        round(p0x), round(p0y), round(p1x), round(p1y),
	round(p2x), round(p2y), round(p3x), round(p3y)
    );
end DrawSplineSection;


procedure xorDraw(const r : Rubberband);
    var dx, midx, left, right : XCoord;
        dy, midy, bottom, top : YCoord;
	x1, y1, x2, y2, cos, sin, hprod : real;
	i, j, k, minimum, maximum : integer;
	infinite : boolean;
	sx : array [0..6] of XCoord;
	sy : array [0..6] of YCoord;
begin
    SetRendering(INVERT);
    case r^.rubType of
	RubberPerpLine:
	    if abs(r^.originX0 - r^.curX) < abs(r^.originY0 - r^.curY) then
	        Line(r^.originX0, r^.originY0, r^.originX0, r^.curY);
	    else
	        Line(r^.originX0, r^.originY0, r^.curX, r^.originY0);
	    end;
	    |
	RubberLine:
	    Line(r^.originX0, r^.originY0, r^.curX, r^.curY)
	    |
        RubberCircle:
	    Circle(
	        r^.originX0, r^.originY0,
		Distance(r^.originX0, r^.originY0, r^.curX, r^.curY)
	    )
	    |
	RubberEllipse:
	    Ellipse(
		r^.originX0, r^.originY0, 
		abs(r^.originX0 - r^.curX), abs(r^.originY0 - r^.curY)
	    );
	    |
	RubberRect:
	    if (r^.originX0 = r^.curX) or (r^.originY0 = r^.curY) then
		Line(r^.originX0, r^.originY0, r^.curX, r^.curY);
	    else
	        Rectangle(r^.originX0, r^.originY0, r^.curX, r^.curY);
	    end;
	    |
	StretchingRect:
	    left := r^.originX0;
	    bottom := r^.originY0;
	    right := r^.originX1;
	    top := r^.originY1;
	    if top = bottom then
	        if r^.origX = r^.originX0 then
		    right := r^.curX;
		else
		    left := r^.curX;
		end;
	    elsif r^.origY = r^.originY0 then
		top := r^.curY;
	    elsif r^.origY = r^.originY1 then
	        bottom := r^.curY;
	    elsif r^.origX = r^.originX0 then
	        right := r^.curX;
	    else
	        left := r^.curX;
	    end;
	    if (left = right) or (bottom = top) then
		Line(left, bottom, right, top);
	    else
	        Rectangle(left, bottom, right, top);
	    end;
	    |
	SlidingLine:
	    if (r^.curX # r^.refptX) or (r^.curY # r^.refptY) then
	        dx := r^.curX - r^.refptX;
	        dy := r^.curY - r^.refptY;
	        r^.originX0 := r^.originX0 + dx;
	        r^.originY0 := r^.originY0 + dy;
	        r^.originX1 := r^.originX1 + dx;
	        r^.originY1 := r^.originY1 + dy;
	        r^.refptX := r^.curX;
	        r^.refptY := r^.curY;
	    end;
	    Line(r^.originX0, r^.originY0, r^.originX1, r^.originY1);
	    |
	SlidingRect, HandleRect:
	    if (r^.curX # r^.refptX) or (r^.curY # r^.refptY) then
	        dx := r^.curX - r^.refptX;
	        dy := r^.curY - r^.refptY;
	        r^.originX0 := r^.originX0 + dx;
	        r^.originY0 := r^.originY0 + dy;
	        r^.originX1 := r^.originX1 + dx;
	        r^.originY1 := r^.originY1 + dy;
	        r^.refptX := r^.curX;
	        r^.refptY := r^.curY;
	    end;
	    if r^.rubType = SlidingRect then
		if
		    (r^.originX0 = r^.originX1) or 
		    (r^.originY0 = r^.originY1)
		then
		    Line(r^.originX0, r^.originY0, r^.originX1, r^.originY1);
		else
		    Rectangle(r^.originX0, r^.originY0, r^.originX1, r^.originY1);
		end;
	    else	(* HandleRect *)
		SetPattern(solidPattern);
		FilledRectangle(
		    r^.originX0 - HANDLESIZE, r^.originY0 - HANDLESIZE,
		    r^.originX0 + HANDLESIZE, r^.originY0 + HANDLESIZE
		);
		if
		    (r^.originX1 # r^.originX0) or (r^.originY1 # r^.originY0)
		then
		    FilledRectangle(
		        r^.originX1 - HANDLESIZE, r^.originY1 - HANDLESIZE,
		        r^.originX1 + HANDLESIZE, r^.originY1 + HANDLESIZE
		    );
		    if
		        (r^.originX1 # r^.originX0) and 
			(r^.originY1 # r^.originY0) 
		    then
			midx := (r^.originX0 + r^.originX1) div 2;
			midy := (r^.originY0 + r^.originY1) div 2;
		        FilledRectangle(
		            r^.originX1 - HANDLESIZE, r^.originY0 - HANDLESIZE,
		            r^.originX1 + HANDLESIZE, r^.originY0 + HANDLESIZE
		        );
		        FilledRectangle(
		            midx - HANDLESIZE, r^.originY0 - HANDLESIZE,
		            midx + HANDLESIZE, r^.originY0 + HANDLESIZE
		        );
		        FilledRectangle(
		            midx - HANDLESIZE, r^.originY1 - HANDLESIZE,
		            midx + HANDLESIZE, r^.originY1 + HANDLESIZE
		        );
		        FilledRectangle(
		            r^.originX0 - HANDLESIZE, r^.originY1 - HANDLESIZE,
		            r^.originX0 + HANDLESIZE, r^.originY1 + HANDLESIZE
		        );
		        FilledRectangle(
		            r^.originX1 - HANDLESIZE, midy - HANDLESIZE,
		            r^.originX1 + HANDLESIZE, midy + HANDLESIZE
		        );
		        FilledRectangle(
		            r^.originX0 - HANDLESIZE, midy - HANDLESIZE,
		            r^.originX0 + HANDLESIZE, midy + HANDLESIZE
		        );
		    end;
		end;
	    end;
	    |
	RotatingRect:
	    if (r^.curX # r^.refptX) or (r^.curY # r^.refptY) then
		x1 := float(r^.refptX) - r^.xc;
		y1 := float(r^.refptY) - r^.yc;
		x2 := float(r^.curX) - r^.xc;
		y2 := float(r^.curY) - r^.yc;
		hprod := sqrt((x1*x1 + y1*y1) * (x2*x2 + y2*y2));
		if hprod = 0.0 then
		    return;
		end;
		cos := (x1*x2 + y1*y2) / hprod;
		sin := (x1*y2 - x2*y1) / hprod;
		Transform(r, cos, sin, -sin, cos, 0.0, 0.0);
	        r^.refptX := r^.curX;
	        r^.refptY := r^.curY;
	    end;
	    if (r^.x[0] = r^.x[2]) or (r^.y[0] = r^.y[2]) then
	        maximum := 0;
	    else
	        maximum := 3;
	    end;
	    for i := 0 to maximum do
	        Line(
		    trunc(r^.x[i]), trunc(r^.y[i]),
		    trunc(r^.x[(i + 1) mod 4]), trunc(r^.y[(i + 1) mod 4])
		);
	    end;
	    |
	ScalingRect:
	    if (r^.curX # r^.refptX) or (r^.curY # r^.refptY) then
		infinite := false;
	        x1 := abs(float(r^.curX) - r^.xc);
		y1 := abs(float(r^.curY) - r^.yc);
		x2 := abs(r^.x[2] - r^.xc);
		y2 := abs(r^.y[2] - r^.yc);
		if (x1 > y1) and (x2 # 0.0) then
		    x1 := x1 / x2;
		elsif y2 # 0.0 then
		    x1 := y1 / y2;
		else
		    infinite := true;
		end;
		if not infinite and (x1 # 0.0) then
		    Transform(r, x1, 0.0, 0.0, x1, 0.0, 0.0);
		    r^.originX0 := trunc(r^.x[0]);
	            r^.originY0 := trunc(r^.y[0]);
	            r^.originX1 := trunc(r^.x[2]);
	            r^.originY1 := trunc(r^.y[2]);
		end;
	        r^.refptX := r^.curX;
	        r^.refptY := r^.curY;
	    end;
	    if (r^.originX0 = r^.originX1) or (r^.originY0 = r^.originY1) then
		Line(r^.originX0, r^.originY0, r^.originX1, r^.originY1);
	    else
		Rectangle(r^.originX0, r^.originY0, r^.originX1, r^.originY1);
	    end;
	    |
	PolyVertexHandles, SplineVertexHandles, ClosedSplineVertexHandles:
	    maximum := high(r^.vertex^);
	    if r^.rubberVertexSet then
		k := r^.rubberVertexNo;
	        r^.vertex^[k].x := r^.curX;
		r^.vertex^[k].y := r^.curY;
		if r^.rubType = PolyVertexHandles then
		    if r^.rubberVertexNo = maximum then
		        k := 0;
		    else
		        k := r^.rubberVertexNo + 1;
		    end;
		    Line(r^.curX, r^.curY, r^.vertex^[k].x, r^.vertex^[k].y);

		    if r^.rubberVertexNo = 0 then
		        k := maximum;
		    else
			k := r^.rubberVertexNo - 1;
		    end;
		    Line(r^.vertex^[k].x, r^.vertex^[k].y, r^.curX, r^.curY);
		else	(* spline *)
		    for i := -3 to 3 do
		        if r^.rubType = SplineVertexHandles then
		            j := min(max(k + i, 0), maximum);
			else
			    j := ((k + maximum + 1) + i) mod (maximum + 1);
			end;
			sx[i + 3] := r^.vertex^[j].x; 
			sy[i + 3] := r^.vertex^[j].y;
		    end;
		    DrawSplineSection(sx[0:4], sy[0:4]);
		    DrawSplineSection(sx[1:4], sy[1:4]);
		    DrawSplineSection(sx[2:4], sy[2:4]);
		    if
		        (r^.rubType # ClosedSplineVertexHandles) or
			(maximum > 2)
		    then
		        DrawSplineSection(sx[3:4], sy[3:4]);
		    end;
		end;
	    else
		SetPattern(solidPattern);
		for i := 0 to maximum do
		    FilledRectangle(
		        r^.vertex^[i].x - HANDLESIZE, 
			r^.vertex^[i].y - HANDLESIZE,
		        r^.vertex^[i].x + HANDLESIZE,
			r^.vertex^[i].y + HANDLESIZE
		    );
		end;
	    end;
	else
    end;
end xorDraw;


(* export *)
procedure Create(
    var   r : Rubberband;
    const rubType : RubberbandType;
    const skipRate : cardinal
);
begin
    r := Rubberband(genlists.Head(freelist));
    if r = Rubberband(nil) then
        new(r);
    else
        genlists.Delete(r, freelist);
    end;
    r^.rubType := rubType;
    r^.skipRate := skipRate;
end Create;


(* export *)
procedure Initialize(
    const r : Rubberband;
    const originX0 : XCoord;
    const originY0 : YCoord;
    const originX1 : XCoord;
    const originY1 : YCoord;
    const origX : XCoord;
    const origY : YCoord
);
begin
    if r = Rubberband(nil) then
        return;
    end;
    r^.skipCount := r^.skipRate;
    r^.originX0 := originX0;
    r^.originY0 := originY0;
    r^.originX1 := originX1;
    r^.originY1 := originY1;
    r^.xc := float(originX0 + originX1) / 2.0;
    r^.yc := float(originY0 + originY1) / 2.0;
    if (r^.rubType = RotatingRect) or (r^.rubType = ScalingRect) then
	r^.x[0] := float(originX0);
	r^.y[0] := float(originY0);
	r^.x[1] := float(originX1);
	r^.y[1] := float(originY0);
	r^.x[2] := float(originX1);
	r^.y[2] := float(originY1);
	r^.x[3] := float(originX0);
	r^.y[3] := float(originY1);
	r^.x[4] := r^.x[0];
	r^.y[4] := r^.y[0];
    end;
    r^.origX := origX;
    r^.origY := origY;
    r^.curX := origX;
    r^.curY := origY;
    r^.refptX := origX;
    r^.refptY := origY;
    r^.drawn := false;
end Initialize;


(* export *)
procedure InitVertexRubberband(
    const r : Rubberband;
    const x : array of XCoord;
    const y : array of YCoord
);
    var i : integer;
begin
    new(r^.vertex, number(x));
    new(r^.vertex, number(y));
    
    for i := 0 to high(x) do
        r^.vertex^[i].x := x[i];
	r^.vertex^[i].y := y[i];
    end;
    r^.rubberVertexSet := false;
    r^.vertexHandlesDrawn := false;
    r^.drawn := false;
    r^.skipCount := r^.skipRate;
end InitVertexRubberband;


(* export *)
procedure SetRubberVertex(
    const r : Rubberband;
    const number : integer
);
begin
    r^.rubberVertexSet := true;
    r^.rubberVertexNo := number;
    r^.origX := r^.vertex^[number].x;
    r^.origY := r^.vertex^[number].y;
    r^.curX := r^.origX;
    r^.curY := r^.origY;
end SetRubberVertex;


(* export *)
procedure ResetRubberVertex(const r : Rubberband);
begin
    if r^.rubberVertexSet then
        r^.vertex^[r^.rubberVertexNo].x := r^.origX;
	r^.vertex^[r^.rubberVertexNo].y := r^.origY;
    end;
    r^.rubberVertexSet := false;
end ResetRubberVertex;


(* export *)
procedure GetRubberVertex(
    const r : Rubberband;
    var   n : integer
) : boolean;
begin
    n := r^.rubberVertexNo;
    return r^.rubberVertexSet;
end GetRubberVertex;


(* export *)
procedure GetAffectedArea(
    const r : Rubberband;
    var   x0 : XCoord;
    var   y0 : YCoord;
    var   x1 : XCoord;
    var   y1 : YCoord
) : boolean;
    var nvertices : integer;
        i, j, limit : integer;
begin
    if 
        (r = Rubberband(nil)) or (
	    (r^.rubType # PolyVertexHandles) and
	    (r^.rubType # SplineVertexHandles) and
	    (r^.rubType # ClosedSplineVertexHandles)
	) or not r^.rubberVertexSet
    then
        return false;
    end;
    nvertices := number(r^.vertex^);
    x0 := r^.origX;
    y0 := r^.origY;
    x1 := r^.origX;
    y1 := r^.origY;
    if r^.rubType = PolyVertexHandles then
        limit := 1;
    else
        limit := 2;
    end;
    for i := -limit to limit do
	j := (r^.rubberVertexNo + i + nvertices) mod nvertices;
	x0 := min(x0, r^.vertex^[j].x);
	y0 := min(y0, r^.vertex^[j].y);
	x1 := max(x1, r^.vertex^[j].x);
	y1 := max(y1, r^.vertex^[j].y);
    end;
    x0 := x0 - HANDLESIZE;
    y0 := y0 - HANDLESIZE;
    x1 := x1 + HANDLESIZE;
    y1 := y1 + HANDLESIZE;
    return true;
end GetAffectedArea;


(* export *)
procedure EraseAndDestroy(var r : Rubberband);
begin
    if r = Rubberband(nil) then
        return;
    end;
    if
        (r^.rubType = SplineVertexHandles) or
	(r^.rubType = ClosedSplineVertexHandles) or
	(r^.rubType = PolyVertexHandles)
    then
	if r^.drawn then
	    r^.rubberVertexSet := true;
            xorDraw(r);
        end;
	if r^.vertexHandlesDrawn then
	    r^.rubberVertexSet := false;
	    xorDraw(r);
	end;	
        dispose(r^.vertex);

    elsif r^.drawn then
        xorDraw(r);
    end;
    genlists.Prepend(r, freelist);
    r := Rubberband(nil);
end EraseAndDestroy;


(* export *)
procedure Destroy(var r: Rubberband);
begin
    if r = Rubberband(nil) then
	return;
    end;
    if
        (r^.rubType = SplineVertexHandles) or 
	(r^.rubType = ClosedSplineVertexHandles) or
	(r^.rubType = PolyVertexHandles)
    then
         dispose(r^.vertex);
    end;
    genlists.Prepend(r, freelist);
    r := Rubberband(nil);
end Destroy;


(* export *)
procedure Draw(
    r : Rubberband;
    x : XCoord;
    y : YCoord
);
begin
    if r = Rubberband(nil) then
        return;
    end;
    if r^.skipCount = 0 then
	r^.skipCount := r^.skipRate;
	if  (x # r^.curX) or (y # r^.curY) then
	    if
		(
		    (r^.rubType = SplineVertexHandles) or 
		    (r^.rubType = ClosedSplineVertexHandles) or
		    (r^.rubType = PolyVertexHandles)
		) and not r^.rubberVertexSet
	    then
		(* nada *)
	    elsif r^.drawn then
	        xorDraw(r);
	    else
	        r^.drawn := true;
	    end;
	    r^.curX := x;
	    r^.curY := y;
	    xorDraw(r);
	end;
    else
        r^.skipCount := r^.skipCount - 1;
    end;
end Draw;


(* export *)
procedure DrawStatic(const r : Rubberband);
begin
    if r = Rubberband(nil) then
        return;
    end;
    if
	(
            (r^.rubType = SplineVertexHandles) or 
	    (r^.rubType = ClosedSplineVertexHandles) or
	    (r^.rubType = PolyVertexHandles)
	) and not r^.rubberVertexSet
    then
        r^.vertexHandlesDrawn := true;
    else
        r^.drawn := true;
    end;
    xorDraw(r);
end DrawStatic;


(* export *)
procedure Erase(const r : Rubberband);
begin
    if r = Rubberband(nil) then
        return;
    end;
    if
	(
            (r^.rubType = SplineVertexHandles) or 
	    (r^.rubType = ClosedSplineVertexHandles) or
	    (r^.rubType = PolyVertexHandles)
	) and not r^.rubberVertexSet
    then
        if not r^.vertexHandlesDrawn then
	    return;
	end;
        r^.vertexHandlesDrawn := false;
    elsif not r^.drawn then
        return;
    else
        r^.drawn := false;
    end;
    xorDraw(r);
end Erase;


(* export *)
procedure DrawStaticInBoundingBox(
    const r : Rubberband;
    const left : XCoord;
    const bottom : YCoord;
    const right : XCoord;
    const top : YCoord
);
begin
    Writable(left, bottom, right, top);
    DrawStatic(r);
    AllWritable();
end DrawStaticInBoundingBox;


(* export *)
procedure GetRubberbandType(const r : Rubberband) : RubberbandType;
begin
    return r^.rubType;
end GetRubberbandType;


(* export *)
procedure GetOrigin(
    const r : Rubberband;
    var   x0 : XCoord;
    var   y0 : YCoord;
    var   x1 : XCoord;
    var   y1 : YCoord;
    var   origX : XCoord;
    var   origY : YCoord
);
begin
    if r = Rubberband(nil) then
        return;
    end;
    if r # nil then
        x0 := r^.originX0;
	y0 := r^.originY0;
	x1 := r^.originX1;
	y1 := r^.originY1;
	origX := r^.origX;
	origY := r^.origY;
    end;
end GetOrigin;

begin
    genlists.Create(freelist);
end rubband.
