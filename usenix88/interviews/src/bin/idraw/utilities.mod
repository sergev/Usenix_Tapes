implementation module utilities;


from math import
    sqrt;

(* export *)
from picture import
    Picture, XCoord, YCoord;

(* export *)
from genlists import
    GenericList;

from genlists import
    Iterator, Create, BeginIteration, MoreElements, EndIteration;


(* export const PI = 3.1415927; *)


(* export *)
procedure round(const x : real) : integer;
begin
    if x < 0.0 then
	return -round(-x);
    else
        return trunc(x + 0.5);
    end;
end round;


(* export *)
procedure sign(const x : real) : integer;
begin
    if x >= 0.0 then
        return 1;
    else
	return -1;
    end;
end sign;


(* export *)
procedure Deg(const x : real) : real;
begin
    return x * 180.0 / PI;
end Deg;


(* export *)
procedure Rad(const x : real) : real;
begin
   return x * PI / 180.0;
end Rad;


(* export *)
procedure Distance(const x0, y0, x1, y1 : integer) : integer;
    var dx, dy : integer;
begin
    dx := x0 - x1;
    dy := y0 - y1;
    return round(sqrt(float(dx*dx + dy*dy)));
end Distance;


(* export *)
procedure FloatDistance(const x0, y0, x1, y1 : real) : real;
    var dx, dy : real;
begin
    dx := x0 - x1;
    dy := y0 - y1;
    return sqrt(dx*dx + dy*dy);
end FloatDistance;


(* export *)
procedure PointInBox(
    const px : XCoord;
    const py : YCoord; 
    const left : XCoord;
    const bottom : YCoord;
    const right : XCoord;
    const top : YCoord
) : boolean;
begin
    return (
        (px >= left) and (px <= right) and (py >= bottom) and (py <= top)
    );
end PointInBox;


(* export *)
procedure RectInBox(
    const rx0 : XCoord;
    const ry0 : YCoord;
    const rx1 : XCoord;
    const ry1 : YCoord;
    const bx0 : XCoord;
    const by0 : YCoord;
    const bx1 : XCoord;
    const by1 : YCoord
) : boolean;
begin
    return (
        (rx0 >= bx0) and (ry0 >= by0) and (rx1 <= bx1) and (ry1 <= by1)
    );
end RectInBox;


(* export *)
procedure MergeBoxes(
    var   ax0 : XCoord;
    var   ay0 : YCoord;
    var   ax1 : XCoord;
    var   ay1 : YCoord;
    const bx0 : XCoord;
    const by0 : YCoord;
    const bx1 : XCoord;
    const by1 : YCoord
);
begin
    if (ax0 = ax1) and (ay0 = ay1) then
        ax0 := bx0;
	ay0 := by0;
	ax1 := bx1;
	ay1 := by1;
    else
        ax0 := min(ax0, bx0);
        ay0 := min(ay0, by0);
        ax1 := max(ax1, bx1);
        ay1 := max(ay1, by1);
    end;
end MergeBoxes;


(* export *)
procedure BoxesIntersect(
    const ax0 : XCoord;
    const ay0 : YCoord;
    const ax1 : XCoord;
    const ay1 : YCoord;
    const bx0 : XCoord;
    const by0 : YCoord;
    const bx1 : XCoord;
    const by1 : YCoord
) : boolean;
begin
    return (
        ((ax0 <= bx1) and (bx0 <= ax1) and (ay0 <= by1) and (by0 <= ay1)) or
	((bx0 <= ax1) and (ax0 <= bx1) and (by0 <= ay1) and (ay0 <= by1))
    );
end BoxesIntersect;


(* export *)
procedure IntersectBoxes(
    var   ax0 : XCoord;
    var   ay0 : YCoord;
    var   ax1 : XCoord;
    var   ay1 : YCoord;
    const bx0 : XCoord;
    const by0 : YCoord;
    const bx1 : XCoord;
    const by1 : YCoord
) : boolean;
begin
    if not BoxesIntersect(ax0, ay0, ax1, ay1, bx0, by0, bx1, by1) then
	ax0 := 0;
	ay0 := 0;
	ax1 := 0;
	ay1 := 0;
	return false;
    else
	ax0 := max(ax0, bx0);
	ay0 := max(ay0, by0);
	ax1 := min(ax1, bx1);
	ay1 := min(ay1, by1);
	return true;
    end;
end IntersectBoxes;


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


(* export *)
procedure AssignString(
    const source : array of char;
    var   dest : array of char
);
    var i : integer;
begin
    for i := 0 to min(number(source), number(dest)) - 1 do
        dest[i] := source[i];
    end;
end AssignString;


(* export *)
procedure PictInList(
    const p : Picture;
    const l : GenericList
) : boolean;
    var test : Picture;
        i : Iterator;
begin
    i := BeginIteration(l);
    while MoreElements(i, test) do
        if test = p then
	    EndIteration(i);
	    return true;
	end;
    end;
    EndIteration(i);
    return false;
end PictInList;


end utilities.


