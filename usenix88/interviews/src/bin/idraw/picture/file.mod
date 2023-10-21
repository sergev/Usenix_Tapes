implementation module file;

import
    genlists,
    vdi;

(* export *)
from main import
    Picture, XCoord, YCoord;

from main import
    PictureObjectType, PaintType, RenderingStyle, identityMatrix, 
    solidPattern, clearPattern;

from xforms import
    MatricesIdentical;

from alloc import
    AllocXform;

from bbox import
    CalcBbox;

from ops import
    Create, DestroyPict, GetUid, GetFontId;

(* export *)
from genlists import
    GenericList;

from io import
    File, Open, Close, Readf, Writef;


(* export const POINTOBJ 	= "a"; *)	(* PictureObjectType *)
(* export const CIRCLEOBJ 	= "b"; *)
(* export const RECTOBJ 	= "c"; *)
(* export const POLYGONOBJ 	= "d"; *)
(* export const LINEOBJ 	= "e"; *)
(* export const TEXTOBJ 	= "f"; *)
(* export const PICTURELIST 	= "g"; *)
(* export const ELLIPSEOBJ	= "h"; *)
(* export const BSPLINEOBJ	= "i"; *)
(* export const CLOSEDBSPLINEOBJ= "j"; *)
    
(* export const FILL 		= "m"; *)	(* RenderingStyle *)
(* export const STROKE 		= "n"; *)
    
(* export const OPAQUE 		= "r"; *)	(* PaintType *)
(* export const TRANSPARENT 	= "s"; *)
(* export const XOR 		= "t"; *)

(* export const XFORM 		= "X"; *)
(* export const NOXFORM 	= "N"; *)
(* export const IDENTITY 	= "I"; *)
(* export const XFORMCODE	= "%c"; *)

(* export const NESTLEVELFMT  	= "%d"; *)

(* export const ENABLEFMT 	= "%c"; *)
(* export const ATTRIBFMT 	= "%d %d %d%c%d%c%d %d %d %d %d "; *)
(* export const BBOXREADFMT 	= "%d %d %d %d %f %f"; *)
(* export const BBOXWRITEFMT 	= "%d %d %d %d %g %g"; *)
(* export const XFORMREADFMT 	= "%f %f %f %f %f %f "; *)
(* export const XFORMWRITEFMT 	= "%g %g %g %g %g %g "; *)
(* export const COMPLXFMT 	= "%d"; *)
(* export const PICTOBJFMT 	= "%c"; *)
(* export const POINTFMT 	= "%d %d"; *)
(* export const LINEFMT 	= "%d %d %d %d"; *)
(* export const CIRCLEFMT 	= "%d %d %d"; *)
(* export const ELLIPSEFMT	= "%d %d %d %d"; *)
(* export const RECTFMT 	= "%d %d %d %d"; *)
(* export const NELEMS		= "%d"; *)
(* export const POLYFMT 	= " %d %d"; *)
(* export const TEXTSTARTFMT  	= " %d %d-"; *)
(* export const TEXTFMT 	= "%c"; *)

const
    LEVELNOTREAD= -1;

var
    nestLevel, readNestLevel : integer;
    emptyFile : boolean;


procedure EncodePictObjType(const pot : PictureObjectType) : char;
begin
    case pot of
        PointObj:	return POINTOBJ;|
	CircleObj:	return CIRCLEOBJ;|
	EllipseObj:	return ELLIPSEOBJ;|
	RectObj:	return RECTOBJ;|
	PolygonObj:	return POLYGONOBJ;|
	BSplineObj:	return BSPLINEOBJ;|
	ClosedBSplineObj: return CLOSEDBSPLINEOBJ;|
	LineObj:	return LINEOBJ;|
	TextObj:	return TEXTOBJ;|
	PictureList:	return PICTURELIST;
    end;
end EncodePictObjType;


procedure DecodePictObjType(const c : char) : PictureObjectType;
begin
    case c of
        POINTOBJ:	return PointObj;|
	CIRCLEOBJ:	return CircleObj;|
	ELLIPSEOBJ:	return EllipseObj;|
	RECTOBJ:	return RectObj;|
	POLYGONOBJ:	return PolygonObj;|
	BSPLINEOBJ:	return BSplineObj;|
	CLOSEDBSPLINEOBJ: return ClosedBSplineObj;|
	LINEOBJ:	return LineObj;|
	TEXTOBJ:	return TextObj;|
	PICTURELIST:	return PictureList;
    end;
end DecodePictObjType;


procedure EncodeRendStyle(const rs : RenderingStyle) : char;
begin
    case rs of
        Fill:		return FILL;|
	Stroke:		return STROKE;
    end;
end EncodeRendStyle;


procedure DecodeRendStyle(const c : char) : RenderingStyle;
begin
    case c of
    	FILL:		return Fill;|
	STROKE:		return Stroke;
    end;
end DecodeRendStyle;


procedure EncodePaintType(const pt : PaintType) : char;
begin
    case pt of
    	Opaque:		return OPAQUE;|
	Transparent:	return TRANSPARENT;|
	Xor:		return XOR;
    end;
end EncodePaintType;


procedure DecodePaintType(const c : char) : PaintType;
begin
    case c of
        OPAQUE:		return Opaque;|
	TRANSPARENT:	return Transparent;|
	XOR:		return Xor;
    end;
end DecodePaintType;


procedure WriteObj(
    const fp : File;
    const pict : Picture
);
    var i, n : integer;
        j : genlists.Iterator;
	subpict : Picture;
begin
    with pict^ do
        case objType of
            PointObj:
	        Writef(fp, POINTFMT, point.x, point.y);
		|
	    LineObj:
	        Writef(
		    fp, LINEFMT, line.p1.x, line.p1.y, line.p2.x, line.p2.y
		);
		|
	    CircleObj:
	        Writef(
		    fp, CIRCLEFMT, 
		    circle.center.x, circle.center.y, circle.radius
		);
		|
	    EllipseObj:
	        Writef(
		    fp, ELLIPSEFMT,
		    ellipse.center.x, ellipse.center.y, ellipse.r1, ellipse.r2
		);
		|
	    RectObj:
	        Writef(
		    fp, RECTFMT,
		    rect.lowLeft.x, rect.lowLeft.y,
		    rect.upRight.x, rect.upRight.y
		);
		|
	    PolygonObj, BSplineObj, ClosedBSplineObj:
		n := number(vertex.x^);
	        Writef(fp, NELEMS, n);
		for i := 0 to n - 1 do
		    Writef(fp, POLYFMT, vertex.x^[i], vertex.y^[i]);
		end;
		|
	    TextObj:
	        n := number(text.chars^);
		Writef(fp, NELEMS, n);
		Writef(fp, TEXTSTARTFMT, text.start.x, text.start.y);
		for i := 0 to n - 1 do
		    Writef(fp, TEXTFMT, text.chars^[i]);
		end;
	        |
	    PictureList:
		j := genlists.BeginIteration(pictList);
		while genlists.MoreElements(j, subpict) do
		    nestLevel := nestLevel + 1;
		    Writef(fp, "\n");
		    WritePict(fp, subpict);
		    nestLevel := nestLevel - 1;
		end;
		genlists.EndIteration(j);
	end;
    end;
end WriteObj;


procedure WritePict(
    const fp : File;
    const pict : Picture
);
    var uid, newpat : integer;
begin
    if (pict = Picture(nil)) or pict^.destroyed then
        return;
    end;

    Writef(fp, NESTLEVELFMT, nestLevel);
    Writef(fp, ENABLEFMT, char(pict^.enabled));
    with pict^.attrib do
	if not GetUid(font, uid) then
	    uid := font;
	end;
	if pattern = vdi.solidPattern then
	    newpat := solidPattern;
	elsif pattern = vdi.clearPattern then
	    newpat := clearPattern;
	else
	    newpat := pattern;
	end;
	Writef(
	    fp, ATTRIBFMT,
	    uid, color, bkgndColor, EncodePaintType(paintType),
	    newpat, EncodeRendStyle(rendering), lineStyle,
	    curPos.x, curPos.y, origin.x, origin.y
	);
    end;
    with pict^.bbox do
	Writef(
	    fp, BBOXWRITEFMT, 
	    lowLeft.x, lowLeft.y, upRight.x, upRight.y,
	    center.x, center.y
	);
    end;
    if pict^.xform = nil then
        Writef(fp, NOXFORM);
    elsif MatricesIdentical(pict^.xform^, identityMatrix) then
        Writef(fp, IDENTITY);
    else
        with pict^ do
	    Writef(fp, XFORM);
 	    Writef(
		fp, XFORMWRITEFMT,
		xform^[1, 1], xform^[1, 2],
		xform^[2, 1], xform^[2, 2],
		xform^[3, 1], xform^[3, 2]
	    );
	end;
    end;
    Writef(fp, COMPLXFMT, pict^.complexity);
    Writef(fp, PICTOBJFMT, EncodePictObjType(pict^.objType));
    WriteObj(fp, pict);
end WritePict;
	

(* public export *)
procedure Write(
    const pict : Picture;
    const fileName : array of char
) : boolean;
    var fp : File;
begin
    fp := Open(fileName, "w");
    if fp = nil then
        return false;
    end;
    nestLevel := 0;
    WritePict(fp, pict);
    Close(fp);
    return true;
end Write;


(* public export *)
procedure WriteSelections(
    const selections : GenericList;
    const fileName : array of char
) : boolean;
    var parent : Picture;
        result : boolean;
begin
    if Picture(genlists.Head(selections)) = Picture(nil) then
        return false;
    end;
    Create(parent);
    genlists.Release(parent^.pictList);
    parent^.pictList := selections;
    parent^.bbox := CalcBbox(parent);
    result := Write(parent, fileName);
    parent^.pictList := GenericList(nil);
    DestroyPict(parent);
    return result;
end WriteSelections;


procedure ReadObj(
    const fp : File;
    var   pict : Picture
);
    var i, d, dummy, x0, x1, y0, y1 : integer;
	j : genlists.Iterator;
	subpict, parent : Picture;
begin
    if pict^.objType = PictureList then
	Create(subpict);
	subpict^.parent := pict;
 	nestLevel := nestLevel + 1;
	dummy := Readf(fp, "\n");
	ReadPict(fp, subpict);
	nestLevel := nestLevel - 1;

    else
	with pict^ do
	    altered := true;
            case objType of
                PointObj:
		    dummy := Readf(fp, POINTFMT, x0, y0);
		    point.x := XCoord(x0);
		    point.y := YCoord(y0);
		    |
	        LineObj:
	            dummy := Readf(fp, LINEFMT, x0, y0, x1, y1);
		    line.p1.x := XCoord(x0);
		    line.p1.y := YCoord(y0);
		    line.p2.x := XCoord(x1);
		    line.p2.y := YCoord(y1);
		    |
	        CircleObj:
	            dummy := Readf(fp, CIRCLEFMT, x0, y0, circle.radius);
		    circle.center.x := XCoord(x0);
		    circle.center.y := YCoord(y0);
		    |
		EllipseObj:
		    dummy := Readf(
		        fp, ELLIPSEFMT, x0, y0, ellipse.r1, ellipse.r2
		    );
		    ellipse.center.x := XCoord(x0);
		    ellipse.center.y := YCoord(y0);
		    |
	        RectObj:
	            dummy := Readf(fp, RECTFMT, x0, y0, x1, y1);
		    rect.lowLeft.x := XCoord(x0);
		    rect.lowLeft.y := YCoord(y0);
		    rect.upRight.x := XCoord(x1);
		    rect.upRight.y := YCoord(y1);
		    |
	        PolygonObj, BSplineObj, ClosedBSplineObj:
	            dummy := Readf(fp, NELEMS, d);
		    new(vertex.x, d);
		    new(vertex.y, d);
		    for i := 0 to d - 1 do
		        dummy := Readf(fp, POLYFMT, x0, y0);
			vertex.x^[i] := XCoord(x0);
			vertex.y^[i] := YCoord(y0);
		    end;
		    |
	        TextObj:
		    dummy := Readf(fp, NELEMS, d);
		    dummy := Readf(fp, TEXTSTARTFMT, x0, y0);
		    text.start.x := XCoord(x0);
		    text.start.y := YCoord(y0);
		    new(text.chars, d);
		    for i := 0 to d - 1 do
		        dummy := Readf(fp, TEXTFMT, text.chars^[i]);
		    end;
	    end;
	end;
    end;

    if pict^.parent # Picture(nil) then
        parent := pict^.parent;
        genlists.Append(pict, parent^.pictList);
    end;
    dummy := Readf(fp, "\n");
    Create(pict);
    pict^.parent := parent;
end ReadObj;


procedure ReadPict(
    const fp : File;
    var   pict : Picture
);
    var c, c1 : char;
        dummy, x0, x1, y0, y1, uid : integer;
	objType : PictureObjectType;
begin
    loop
        if 
	    (readNestLevel = LEVELNOTREAD) and
	    (Readf(fp, NESTLEVELFMT, readNestLevel) < 0)
	then
	    DestroyPict(pict);
	    return;
	else
	    emptyFile := false;
	end;

        if readNestLevel < nestLevel then
	    return;
	else
	    readNestLevel := LEVELNOTREAD;
	end;

        dummy := Readf(fp, ENABLEFMT, c);
	pict^.enabled := boolean(c);
	with pict^.attrib do
	    dummy := Readf(
	        fp, ATTRIBFMT, uid, color, bkgndColor, c, pattern, 
		c1, lineStyle, x0, y0, x1, y1
	    );
	    if not GetFontId(uid, font) then
	        font := uid;
	    end;
	    curPos.x := XCoord(x0);
	    curPos.y := YCoord(y0);
	    origin.x := XCoord(x1);
	    origin.y := XCoord(y1);
	    paintType := DecodePaintType(c);
	    rendering := DecodeRendStyle(c1);
	end;
	with pict^.bbox do
	    dummy := Readf(fp, BBOXREADFMT, x0, y0, x1, y1, center.x, center.y);
	    lowLeft.x := XCoord(x0);
	    lowLeft.y := YCoord(y0);
	    upRight.x := XCoord(x1);
	    upRight.y := YCoord(y1);
	end;
	dummy := Readf(fp, XFORMCODE, c);
	if c = IDENTITY then
	    AllocXform(pict^.xform);
	    pict^.xform^ := identityMatrix;
	elsif c = XFORM then
	    AllocXform(pict^.xform);
	    with pict^ do
	        dummy := Readf(
	    	    fp, XFORMREADFMT,
		    xform^[1, 1], xform^[1, 2],
		    xform^[2, 1], xform^[2, 2],
		    xform^[3, 1], xform^[3, 2]
		);
	    end;
	end;
	dummy := Readf(fp, COMPLXFMT, pict^.complexity);
	dummy := Readf(fp, PICTOBJFMT, c);
	objType := DecodePictObjType(c);
	if objType # PictureList then
	    genlists.Release(pict^.pictList);
	    pict^.objType := objType;
	end;
	ReadObj(fp, pict);
    end;
end ReadPict;


(* public export *)
procedure Read(
    var   pict : Picture;
    const fileName : array of char
) : boolean;
    var fp : File;
        head : Picture;
begin
    emptyFile := true;
    fp := Open(fileName, "r");
    if fp = nil then
	return false;
    end;
    nestLevel := 0;
    Create(pict);
    head := pict;
    readNestLevel := LEVELNOTREAD;
    ReadPict(fp, head);
    if emptyFile then
        pict := Picture(nil);
    end;
    Close(fp);
    return true;
end Read;


begin

end file.
