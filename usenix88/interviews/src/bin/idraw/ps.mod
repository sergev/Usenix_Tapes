implementation module ps;

import
    genlists,
    picture;

(* export *)
from main import 	(* picture main module *)
    Picture, XCoord, YCoord;

from main import
    TransformMatrix, EllipseType, EllipseSplineX, EllipseSplineY;

from ops import		(* picture pict-format font uid to LoadFont id *)
    GetFontId;

from vdi import
    GetFontHeight,
    ColorValue, WHITE, BLACK, solidPattern, clearPattern, vertPattern,
    horizPattern, majorDiagPattern, minorDiagPattern, thickVertPattern,
    thickHorizPattern, thickMajorDiagPattern, thickMinorDiagPattern, 
    stipplePattern1, stipplePattern2, stipplePattern3, stipplePattern4, 
    stipplePattern5, stipplePattern6;

from file import
    POINTOBJ, CIRCLEOBJ, RECTOBJ, POLYGONOBJ, LINEOBJ, TEXTOBJ, PICTURELIST,
    ELLIPSEOBJ, BSPLINEOBJ, CLOSEDBSPLINEOBJ,
    FILL, STROKE,
    OPAQUE, TRANSPARENT, XOR,
    XFORM, NOXFORM, IDENTITY, XFORMCODE,
    NESTLEVELFMT,
    ENABLEFMT, ATTRIBFMT, BBOXREADFMT, XFORMREADFMT, COMPLXFMT, PICTOBJFMT,
    POINTFMT, LINEFMT, CIRCLEFMT, ELLIPSEFMT, RECTFMT, 
    NELEMS, POLYFMT, TEXTSTARTFMT, TEXTFMT;

from xforms import
    FloatTransformX, FloatTransformY;

from utilities import
    StringsEqual, AssignString, round;

from util import
    EllipseToSpline;

from genlists import
    GenericList;

from math import
    sqrt;

from dvboot import
    GetGrayLevel;

from io import
    File, Open, Close, Readf, Writef;


const
    NEWPATH	= "newpath\n";
    MOVETO	= "%g %g moveto\n";
    LINETO	= "%g %g lineto\n";
    CURVETO	= "%g %g %g curveto\n";
    SPLINE	= "%g %g moveto\n%g %g %g %g %g %g curveto\n";
    SPLINETO	= "%g %g %g %g %g %g curveto\n";
    CLOSEPATH	= "closepath\n";
    SETGRAY	= "%g setgray\n";
    PSFILL	= "eofill\n\n";
    PSSTROKE	= "stroke\n\n";
    SHOWPAGE	= "showpage\n\n";
    DEF		= "/%s {%s} def\n\n";
    COMMENT	= "%%%s\n";
    SETLINEWIDTH= "%d setlinewidth\n";
    COPY	= "%d copy\n";
    PUSHCMD	= "%s\n";
    PUSHPOINT	= "%g %g\n";
    PUSHINT	= "%d\n";
    PUSHREAL	= "%g\n";
    PUSHSTRING	= "(%s) ";
    PUSHCHAR	= "%c";
    PUSHNAME	= "/%s\n";
    EXCH	= "exch\n";
    PSREPEAT	= "repeat\n";
    TRANSLATE	= "%g %g translate\n";
    SCALE	= "%g %g scale\n";
    ROTATE	= "%g rotate\n";

    BUTTCAP	= 0;
    ROUNDCAP	= 1;
    PROJSQCAP	= 2;

type
    FontMapEntry = pointer to FontMapRecord;
    FontMapRecord = record
   	screenFont : integer;
	printFont : dynarray of char;
	printFontSize : integer;
	textOffsetX, textOffsetY : real;
    end;

    ObjectInfo = record
	enabled : boolean;
        font, color, bkgndColor, pattern, lineStyle : integer;
	left, bottom, right, top : integer;
	centerX, centerY : real;
	paintType, rendering, xformCode : char;
	xform : TransformMatrix;
    end;
    
    SplinePrintProc = procedure(File, array of real, array of real);

(* export type String = dynarray of char; *)

var
    fontMap : GenericList;
    pictOriginX, pictOriginY : integer;
    

(* export *)
procedure MapFont(
    const screenFont : integer;
    const printFont : array of char;
    const printFontSize : integer;
    const textOffsetX, textOffsetY : real
);
    var fontMapEntry : FontMapEntry;
        size : integer;
begin
    size := number(printFont);
    new(fontMapEntry);
    new(fontMapEntry^.printFont, size);
    fontMapEntry^.screenFont := screenFont;
    AssignString(printFont, fontMapEntry^.printFont^[0:size]);
    fontMapEntry^.printFontSize := printFontSize;
    fontMapEntry^.textOffsetX := textOffsetX;
    fontMapEntry^.textOffsetY := textOffsetY;
    genlists.Prepend(fontMapEntry, fontMap);
end MapFont;


(* export *)
procedure UnmapPrintFont(
    const printFont : array of char;
    const printFontSize : integer
);
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if
	    (fontMapEntry^.printFontSize = printFontSize) and
	    StringsEqual(fontMapEntry^.printFont^, printFont)
	then
            genlists.DeleteCur(i);
	    dispose(fontMapEntry^.printFont);
 	    dispose(fontMapEntry);
	end;
    end;
    genlists.EndIteration(i);
end UnmapPrintFont;


(* export *)
procedure UnmapScreenFont(const screenFont : integer);
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if fontMapEntry^.screenFont = screenFont then
	    genlists.DeleteCur(i);
	    dispose(fontMapEntry^.printFont);
	    dispose(fontMapEntry);
	end;
    end;
    genlists.EndIteration(i);
end UnmapScreenFont;


(* export *)
procedure GetPrintFont(
    const screenFont : integer;
    var   printFont : String;
    var   printFontSize : integer
) : boolean;
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
	size : integer;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if fontMapEntry^.screenFont = screenFont then
	    size := number(fontMapEntry^.printFont^);
	    new(printFont, size);
	    AssignString(
	        fontMapEntry^.printFont^[0:size], printFont^[0:size]
	    );
	    printFontSize := fontMapEntry^.printFontSize;
	    genlists.EndIteration(i);
	    return true;
	end;
    end;
    genlists.EndIteration(i);
    return false;
end GetPrintFont;


(* export *)
procedure GetTextOffset(
    const printFont : array of char;
    const printFontSize : integer;
    var textOffsetX, textOffsetY : real
);
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if
	    StringsEqual(fontMapEntry^.printFont^, printFont) and
	    (
	        (fontMapEntry^.printFontSize = printFontSize) or
		(printFontSize < 0)
	    )
	then
	    textOffsetX := fontMapEntry^.textOffsetX;
	    textOffsetY := fontMapEntry^.textOffsetY;
	    genlists.EndIteration(i);
	    return;
	end;
    end;
    genlists.EndIteration(i);
end GetTextOffset;


(* export *)
procedure SetTextOffset(
    const printFont : array of char;
    const printFontSize : integer;
    const textOffsetX, textOffsetY : real
);
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if
	    StringsEqual(fontMapEntry^.printFont^, printFont) and
	    (
	        (fontMapEntry^.printFontSize = printFontSize) or
		(printFontSize < 0)
	    )
	then
	    fontMapEntry^.textOffsetX := textOffsetX;
	    fontMapEntry^.textOffsetY := textOffsetY;
	    genlists.EndIteration(i);
	    return;
	end;
    end;
    genlists.EndIteration(i);
end SetTextOffset;


(* export *)
procedure GetScreenFont(
    var   screenFont : integer;
    const printFont : array of char;
    const printFontSize : integer
) : boolean;
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if
	    StringsEqual(fontMapEntry^.printFont^, printFont) and
	    (
	        (fontMapEntry^.printFontSize = printFontSize) or
		(printFontSize < 0)
	    )
	then
	    screenFont := fontMapEntry^.screenFont;
	    genlists.EndIteration(i);
	    return true;
	end;
    end;
    genlists.EndIteration(i);
    return false;
end GetScreenFont;


procedure PrintProlog(const fp : File);
begin
    Writef(fp, COMMENT, "!");
    Writef(fp, DEF, "point", "\n0.5 0.0 360.0\nnewpath\narc\nclosepath\n");
    Writef(fp, DEF, "line", "\nnewpath\nmoveto\nlineto\n");
    Writef(
        fp, DEF, "rect",
	"\nnewpath\nmoveto\nlineto\nlineto\nlineto\nclosepath\n"
    );
    Writef(fp, DEF, "circle", "\n0.0 360.0\nnewpath\narc\nclosepath\n");
    Writef(
        fp, DEF, "polygon",
        "\n3 1 roll\nnewpath\nmoveto\n{lineto} repeat\nclosepath\n"
    );
    Writef(fp, DEF, "text", "\n3 1 roll\nmoveto\nshow\n");
    Writef(
        fp, DEF, "initfont", "\nexch\nfindfont\nexch\nscalefont\nsetfont\n"
    );
end PrintProlog;


procedure Convert(
    const x, y : integer;
    var   newx, newy : real;
    const xformCode : char;
    const xform : TransformMatrix
);
begin
    if xformCode = IDENTITY then
        newx := float(x);
	newy := float(y);
    else
        newx := FloatTransformX(float(x), float(y), xform);
	newy := FloatTransformY(float(x), float(y), xform);
    end;
end Convert;


procedure CalcGray(const pattern, color : integer) : real;
    var value : real;
begin
    if color = WHITE then
        value := 1.0;
    elsif pattern = clearPattern then
        value := 1.0;
    elsif pattern = solidPattern then
        value := 0.0;
    elsif pattern = stipplePattern1 then
        value := 0.95;
    elsif pattern = stipplePattern3 then
        value := 0.85;
    elsif pattern = vertPattern then
        value := 0.8;
    elsif pattern = horizPattern then
        value := 0.8;
    elsif pattern = majorDiagPattern then
        value := 0.6;
    elsif pattern = minorDiagPattern then
        value := 0.6;
    elsif pattern = thickVertPattern then
        value := 0.5;
    elsif pattern = thickHorizPattern then
        value := 0.5;
    elsif pattern = thickMajorDiagPattern then
        value := 0.4;
    elsif pattern = thickMinorDiagPattern then
        value := 0.4;
    elsif pattern = stipplePattern2 then
        value := 0.3;
    elsif pattern = stipplePattern4 then
        value := 0.25;
    elsif pattern = stipplePattern5 then
        value := 0.15;
    elsif pattern = stipplePattern6 then
        value := 0.05;
    else
        value := GetGrayLevel(pattern);
    end;
    return value;
end CalcGray;


procedure PrintPoint(
    const pictf, psf : File;
    const o : ObjectInfo
);
    var x, y, result : integer;
        xf, yf : real;
begin
    result := Readf(pictf, POINTFMT, x, y);
    if o.enabled then
        Convert(x, y, xf, yf, o.xformCode, o.xform);
        Writef(psf, PUSHPOINT, xf, yf);
        Writef(psf, PUSHCMD, "point");
    end;
end PrintPoint;


procedure PrintLine(
    const pictf, psf : File;
    const o : ObjectInfo
);
    var x0, y0, x1, y1, result : integer;
        xf0, yf0, xf1, yf1 : real;
begin
    result := Readf(pictf, LINEFMT, x0, y0, x1, y1);
    if o.enabled then
        Convert(x0, y0, xf0, yf0, o.xformCode, o.xform);
        Convert(x1, y1, xf1, yf1, o.xformCode, o.xform);
        Writef(psf, PUSHPOINT, xf0, yf0);
        Writef(psf, PUSHPOINT, xf1, yf1);
        Writef(psf, PUSHCMD, "line");
    end;
end PrintLine;


procedure PrintRect(
    const pictf, psf : File;
    const o : ObjectInfo
);
    var x0, y0, x1, y1, result : integer;
        xf0, yf0, xf1, yf1, xf2, yf2, xf3, yf3 : real;
begin
    result := Readf(pictf, LINEFMT, x0, y0, x1, y1);
    if o.enabled then
        Convert(x0, y0, xf0, yf0, o.xformCode, o.xform);
        Convert(x1, y0, xf1, yf1, o.xformCode, o.xform);
        Convert(x1, y1, xf2, yf2, o.xformCode, o.xform);
        Convert(x0, y1, xf3, yf3, o.xformCode, o.xform);
        Writef(psf, PUSHPOINT, xf0, yf0);
        Writef(psf, PUSHPOINT, xf1, yf1);
        Writef(psf, PUSHPOINT, xf2, yf2);
        Writef(psf, PUSHPOINT, xf3, yf3);
        Writef(psf, PUSHCMD, "rect");
    end;
end PrintRect;


procedure PrintCircle(
    const pictf, psf : File;
    const o : ObjectInfo
);
    var x, y, radius, result : integer;
        xf, yf, rxf, ryf, scaledRadius : real;
begin
    result := Readf(pictf, CIRCLEFMT, x, y, radius);
    if o.enabled then
        Convert(x, y, xf, yf, o.xformCode, o.xform);
        if o.xformCode = IDENTITY then
            scaledRadius := float(radius);
        else
            Convert(x + radius, y, rxf, ryf, o.xformCode, o.xform);
	    rxf := rxf - xf;
	    ryf := ryf - yf;
            scaledRadius := sqrt(rxf*rxf + ryf*ryf);
        end;
        Writef(psf, PUSHPOINT, xf, yf);
        Writef(psf, PUSHINT, round(scaledRadius));
        Writef(psf, PUSHCMD, "circle");
    end;
end PrintCircle;


procedure PrintEllipse(
    const pictf, psf : File;
    const o : ObjectInfo
);
    var i, n, result : integer;
        e : EllipseType;
	identity : TransformMatrix;
	ex : EllipseSplineX;
	ey : EllipseSplineY;
	x, y : dynarray of real;
begin
    identity[1, 1] := 1.0;
    identity[1, 2] := 0.0;
    identity[2, 1] := 0.0;
    identity[2, 2] := 1.0;
    identity[3, 1] := 0.0;
    identity[3, 2] := 0.0;

    result := Readf(pictf, ELLIPSEFMT, e.center.x, e.center.y, e.r1, e.r2);
    if o.enabled then
	n := number(ex);
	new(x, n);
	new(y, n);
        EllipseToSpline(e, identity, ex, ey, 0, 0);
	for i := 1 to n do
	    Convert(ex[i], ey[i], x^[i - 1], y^[i - 1], o.xformCode, o.xform);
	end;
	PrintClosedSpline(psf, x^[0:n], y^[0:n]);
	dispose(x);
	dispose(y);
    end;
end PrintEllipse;


procedure PrintPolygon(
    const pictf, psf : File;
    const o : ObjectInfo
);
    var i, n, x, y, result : integer;
        xf, yf : real;
begin
    result := Readf(pictf, NELEMS, n);
    for i := 1 to n do
        result := Readf(pictf, POLYFMT, x, y);
	if o.enabled then
            Convert(x, y, xf, yf, o.xformCode, o.xform);
            Writef(psf, PUSHPOINT, xf, yf);
	end;
    end;
    if o.enabled then
        if n = 1 then
	    n := 2;
	    Writef(psf, PUSHPOINT, xf, yf);
	end;
        Writef(psf, PUSHINT, n - 1);
        Writef(psf, PUSHCMD, "polygon");
    end;
end PrintPolygon;


procedure Midpoint(
    const x0, y0, x1, y1 : real;
    var   mx, my : real
);
begin
    mx := (x0 + x1) / 2.0;
    my := (y0 + y1) / 2.0;
end Midpoint;


procedure ThirdPoint(
    const x0, y0, x1, y1 : real;
    var   tx, ty : real
);
begin
    tx := (2.0*x0 + x1) / 3.0;
    ty := (2.0*y0 + y1) / 3.0;
end ThirdPoint;


procedure PrintSubspline(
    const psf : File;
    const x0, y0, x1, y1, x2, y2, x3, y3 : real;
    const movetoNeeded : boolean
);
    var p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y, tempx, tempy : real;
begin
    ThirdPoint(x1, y1, x2, y2, p1x, p1y);
    ThirdPoint(x2, y2, x1, y1, p2x, p2y);
    ThirdPoint(x1, y1, x0, y0, tempx, tempy);
    Midpoint(tempx, tempy, p1x, p1y, p0x, p0y);
    ThirdPoint(x2, y2, x3, y3, tempx, tempy);
    Midpoint(tempx, tempy, p2x, p2y, p3x, p3y);
    if movetoNeeded then
	Writef(psf, SPLINE, p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y);
    else
        Writef(psf, SPLINETO, p1x, p1y, p2x, p2y, p3x, p3y);
    end;
end PrintSubspline;


procedure PrintSpline(
    const psf : File;
    const x, y : array of real
);
    var i, n : integer;
begin
    n := number(x);
    Writef(psf, NEWPATH);
    if n > 1 then
        PrintSubspline(psf, x[0], y[0], x[0], y[0], x[0], y[0], x[1], y[1], true);

        if n > 2 then
	    PrintSubspline(
	        psf, x[0], y[0], x[0], y[0], x[1], y[1], x[2], y[2], false
	    );

	    if n > 3 then
		for i := 1 to n - 3 do
	            PrintSubspline(
	                psf, x[i-1], y[i-1], x[i], y[i],
	                x[i + 1], y[i + 1], x[i + 2], y[i + 2], false
	            );
		end;
	    end;

            PrintSubspline(
	        psf, x[n-3], y[n-3], x[n-2], y[n-2], 
	        x[n-1], y[n-1], x[n-1], y[n-1], false
            );
	end;
        PrintSubspline(
	    psf, x[n-2], y[n-2], x[n-1], y[n-1], 
	    x[n-1], y[n-1], x[n-1], y[n-1], false
        );
    else
	Writef(psf, PUSHPOINT, x[0], y[0]);
	Writef(psf, PUSHPOINT, x[0], y[0]);
	Writef(psf, PUSHINT, 1);
	Writef(psf, PUSHCMD, "polygon");
    end;
end PrintSpline;


procedure PrintClosedSpline(
    const psf : File;
    const x, y : array of real
);
    var i, n : integer;
begin
    n := number(x);
    Writef(psf, NEWPATH);
    if n > 2 then
        PrintSubspline(
            psf, x[n-1], y[n-1], x[0], y[0], x[1], y[1], x[2], y[2], true
        );

        for i := 1 to n - 3 do
            PrintSubspline(
	        psf, x[i-1], y[i-1], x[i], y[i],
	        x[i + 1], y[i + 1], x[i + 2], y[i + 2], false
	   );
        end;
	
        PrintSubspline(
	    psf, x[n-3], y[n-3], x[n-2], y[n-2],
	    x[n-1], y[n-1], x[0], y[0], false
        );
        PrintSubspline(
	    psf, x[n-2], y[n-2], x[n-1], y[n-1], x[0], y[0], x[1], y[1], false
        );
    else
        Writef(psf, PUSHPOINT, x[0], y[0]);
	if n > 1 then
	    Writef(psf, PUSHPOINT, x[1], y[1]);
	else
            Writef(psf, PUSHPOINT, x[0], y[0]);
	    n := 2;
	end;
	Writef(psf, PUSHINT, n - 1);
	Writef(psf, PUSHCMD, "polygon");
    end;
end PrintClosedSpline;


procedure ReadAndPrintSpline(
    const pictf, psf : File;
    const o : ObjectInfo;
    const printProc : SplinePrintProc
);
    var i, n, result, xt, yt : integer;
        x, y : dynarray of real;
begin
    result := Readf(pictf, NELEMS, n);
    new(x, n);
    new(y, n);
    for i := 0 to n - 1 do
        result := Readf(pictf, POLYFMT, xt, yt);
        Convert(xt, yt, x^[i], y^[i], o.xformCode, o.xform);
    end;
    if o.enabled then
        printProc(psf, x^[0:n], y^[0:n])
    end;
    dispose(x);
    dispose(y);
end ReadAndPrintSpline;


procedure PrintText(
    const pictf, psf : File;
    const o : ObjectInfo
);
    var i, n, x, y, printFontSize,
        newPrintFontSize, result, origHeight : integer;
        xf, yf, offx, offy, bboxHeight, left, right, bottom, top : real;
	printFont : String;
	c : char;
begin
    if o.enabled then
	if 
	    (o.pattern = clearPattern) or (o.pattern = picture.clearPattern)
	then
	    Writef(psf, SETGRAY, CalcGray(clearPattern, o.color));
	else
	    Writef(psf, SETGRAY, CalcGray(solidPattern, o.color));
	end;
    end;
    if not GetPrintFont(o.font, printFont, printFontSize) then
        return;
    end;
    origHeight := o.top - o.bottom;
    Convert(o.left, o.bottom, left, bottom, o.xformCode, o.xform);
    Convert(o.right, o.top, right, top, o.xformCode, o.xform);
    bboxHeight := top - bottom;
    if (round(bboxHeight) # origHeight) and (origHeight > 0) then
        newPrintFontSize := round(
	    float(printFontSize) * bboxHeight / float(origHeight)
        );
	newPrintFontSize := max(newPrintFontSize, 1);
    else
        newPrintFontSize := printFontSize;
    end;
    if o.enabled then
	Writef(psf, PUSHNAME, printFont^);
	Writef(psf, PUSHINT, newPrintFontSize);
	Writef(psf, PUSHCMD, "initfont");
    end;
    result := Readf(pictf, NELEMS, n);
    result := Readf(pictf, TEXTSTARTFMT, x, y);
    if o.enabled then
        Convert(x, y, xf, yf, o.xformCode, o.xform);
	GetTextOffset(printFont^, printFontSize, offx, offy);
        Writef(psf, PUSHPOINT, xf + offx, yf + offy);
	Writef(psf, PUSHCHAR, '(');
    end;
    
    for i := 0 to n - 2 do
        result := Readf(pictf, TEXTFMT, c);
	if o.enabled then
	    if (c = '(') or (c = ')') then
		Writef(psf, PUSHCHAR, 134C);	(* backslash *)
	    end;
	    Writef(psf, PUSHCHAR, c);
	end;
    end;
    result := Readf(pictf, TEXTFMT, c);		(* read & chuck null *)
    if o.enabled then
	if c # 0C then
	    Writef(psf, PUSHCHAR, c);		(* in case last char wasn't *)
	end;					(* a null, for compatibility *)
	Writef(psf, PUSHCHAR, ')');
        Writef(psf, PUSHCMD, " text");
    end;
end PrintText;


procedure PrintObject(const pictf, psf : File);
    var result, d  : integer;
        c, pictObjType : char;
	df : real;
	o : ObjectInfo;
	dummy : boolean;
begin
    if Readf(pictf, NESTLEVELFMT, d) < 0 then
        return;
    end;
    result := Readf(pictf, ENABLEFMT, c);
    o.enabled := boolean(c);
    result := Readf(
        pictf, ATTRIBFMT, 
	o.font, o.color, o.bkgndColor, o.paintType, o.pattern, o.rendering,
	o.lineStyle, d, d, d, d
    );
    dummy := GetFontId(o.font, o.font);
    result := Readf(
        pictf, BBOXREADFMT,
	o.left, o.bottom, o.right, o.top, o.centerX, o.centerY
    );
    result := Readf(pictf, XFORMCODE, o.xformCode);
    if o.xformCode = XFORM then		(* node is a leaf *)
        result := Readf(
	    pictf, XFORMREADFMT,
	    o.xform[1, 1], o.xform[1, 2], 
	    o.xform[2, 1], o.xform[2, 2],
	    o.xform[3, 1], o.xform[3, 2]
	);
    end;
    result := Readf(pictf, COMPLXFMT, d);
    result := Readf(pictf, PICTOBJFMT, pictObjType);
    if o.xformCode = NOXFORM then 	(* if node not a leaf *)
	return;
    end;
    
    case pictObjType of
        POINTOBJ:	PrintPoint(pictf, psf, o);|
	LINEOBJ:	o.rendering := STROKE;		(* kludge *)
			PrintLine(pictf, psf, o);
	|
	RECTOBJ:	PrintRect(pictf, psf, o);|
	CIRCLEOBJ:	PrintCircle(pictf, psf, o);|
	ELLIPSEOBJ:	PrintEllipse(pictf, psf, o);|
	POLYGONOBJ:	PrintPolygon(pictf, psf, o);|
	BSPLINEOBJ:	ReadAndPrintSpline(pictf, psf, o, PrintSpline);|
	CLOSEDBSPLINEOBJ:
			ReadAndPrintSpline(pictf, psf, o, PrintClosedSpline);|
	TEXTOBJ:	PrintText(pictf, psf, o);
			return;
	else
    end;
    
    if o.enabled then
	Writef(psf, SETLINEWIDTH, 1);
	case o.rendering of
	    FILL:	Writef(psf, SETGRAY, CalcGray(o.pattern, o.color));
			Writef(psf, PSFILL);
	    |
	    STROKE:	Writef(psf, SETGRAY, CalcGray(solidPattern, o.color));
			Writef(psf, PSSTROKE);
	end;
    end;
end PrintObject;


procedure PrintPicture(const pictf, psf : File);
    var result, d : integer;
        c : char;
begin
    repeat
	PrintObject(pictf, psf);
    until Readf(pictf, "%c", c) < 0;
end PrintPicture;


(* export *)
procedure PictToPS(
    const pictfile, psfile : array of char;
    const x0, y0, sx, sy, angle : real	(* origin, scale, rotation factors *)
) : boolean;
    var pictf, psf : File;
begin
    pictf := Open(pictfile, "r");
    psf := Open(psfile, "w");
    if (pictf = nil) or (psf = nil) then
        return false;
    end;
    PrintProlog(psf);
    Writef(psf, TRANSLATE, x0, y0);
    Writef(psf, SCALE, sx, sy);
    Writef(psf, ROTATE, angle);
    PrintPicture(pictf, psf);
    Writef(psf, SHOWPAGE);
    Close(pictf);
    Close(psf);
    return true;
end PictToPS;


begin
    genlists.Create(fontMap);
end ps.
