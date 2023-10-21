implementation module attribs;

(* export *)
from main import
    Picture, XCoord, YCoord, ColorValue, WHITE, Attributes, AttribType,
    RenderingStyle, GenericList, PaintType;
    
from main import
    PictureObjectType;

from bbox import
    SizeTextBbox, UpdateParentBboxes;

from util import
    Root;

from redraw import
    UpdateEraseMask;
    
import 
    genlists,
    vdi;


(* export *)
procedure AlterPictureList(
    const list : GenericList;
    const whichAttrib : AttribType;
    const attrib : Attributes
);
    var i : genlists.Iterator;
        subpict : Picture;
begin
    i := genlists.BeginIteration(list);
    while genlists.MoreElements(i, subpict) do
	case whichAttrib of
	    FontAttrib:	AlterFont(subpict, attrib.font)
	    |
	    Color:	AlterColor(subpict, attrib.color)
	    |
	    BkgndColor: AlterBackgroundColor(subpict, attrib.bkgndColor);
	    |
	    Paint:      AlterPaintType(subpict, attrib.paintType);
	    |
	    PatternAttrib:    AlterPattern(subpict, attrib.pattern)
	    |
	    Rendering:  AlterRenderingStyle(subpict, attrib.rendering)
	    |
	    LineStyleAttrib:  AlterLineStyle(subpict, attrib.lineStyle)
	    else        (* nada *)
	end;
    end;
    genlists.EndIteration(i);
end AlterPictureList;


(* export *)
procedure Alter(
    const picture : Picture;
    const whichAttrib : AttribType;
    const attrib : Attributes
);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.altered := true;
    
    if picture^.objType = PictureList then
	AlterPictureList(picture^.pictList, whichAttrib, attrib);
    else
        case whichAttrib of
	    FontAttrib:	
	        picture^.attrib.font := attrib.font;
		if picture^.objType = TextObj then
		    SizeTextBbox(picture);
		    UpdateParentBboxes(picture);
		end;
	    |
	    Color:	picture^.attrib.color := attrib.color
	    |
	    BkgndColor: picture^.attrib.bkgndColor := attrib.bkgndColor
	    |
	    Paint:	picture^.attrib.paintType := attrib.paintType
	    |
	    PatternAttrib:    picture^.attrib.pattern := attrib.pattern
	    |
	    Rendering:  picture^.attrib.rendering := attrib.rendering
	    |
	    LineStyleAttrib:  picture^.attrib.lineStyle := attrib.lineStyle
	    else  	(* nada *)
	end;
    end;
end Alter;


(* public export *)
procedure SetOrigin(
    const picture : Picture;
    const x       : XCoord;
    const y       : YCoord
);
    var root : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;
    root := Root(picture);
    root^.attrib.origin.x := x;
    root^.attrib.origin.y := y;
end SetOrigin;


(* public export *)
procedure GetOrigin(
    const picture : Picture;
    var   x       : XCoord;
    var   y       : YCoord
);
    var root : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;
    root := Root(picture);
    x := root^.attrib.origin.x;
    y := root^.attrib.origin.y;
end GetOrigin;


(* public export *)
procedure MoveTo(
    const picture : Picture;
    const x : XCoord;
    const y : YCoord
);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.attrib.curPos.x := x;
    picture^.attrib.curPos.y := y;
end MoveTo;


(* public export *)
procedure GetPosition(
    const picture : Picture;
    var   x : XCoord;
    var   y : YCoord
);
begin
    if picture = Picture(nil) then
        return;
    end;

    x := picture^.attrib.curPos.x;
    y := picture^.attrib.curPos.y;
end GetPosition;


(* public export *)
procedure GetBoundingBox(
    const picture : Picture;
    var   left	  : XCoord;
    var   bottom  : YCoord;
    var   right   : XCoord;
    var   top     : YCoord
);
begin
    if picture = Picture(nil) then
        return;
    end;
    
    left := picture^.bbox.lowLeft.x;
    bottom := picture^.bbox.lowLeft.y;
    right := picture^.bbox.upRight.x;
    top := picture^.bbox.upRight.y;
end GetBoundingBox;


(* public export *)
procedure GetCenter(
    const picture : Picture;
    var   x, y	  : real
);
begin
    if picture = Picture(nil) then
        return;
    end;

    x := picture^.bbox.center.x;
    y := picture^.bbox.center.y;
end GetCenter;


(* public export *)
procedure GetParent(const picture : Picture) : Picture;
begin
    if picture # Picture(nil) then
        return picture^.parent;
    end;
    return Picture(nil);
end GetParent;


(* public export *)
procedure StrWidth(
    const picture : Picture;
    const string  : array of char
) : integer;
begin
    if picture = Picture(nil) then
        return 0;
    end;

    vdi.SetFont(picture^.attrib.font);
    return vdi.StrWidth(string);
end StrWidth;


(* public export *)
procedure DefaultFont(
    const picture : Picture;
    const font    : integer
);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.attrib.font := font;
end DefaultFont;


(* public export *)
procedure GetFont(const picture : Picture) : integer;
begin
    if picture = Picture(nil) then
        return 0;
    end;

    return picture^.attrib.font;
end GetFont;


(* public export *)
procedure AlterFont(
    const picture    : Picture;
    const font	     : integer
);
    var attrib : Attributes;
begin
    if picture = Picture(nil) then
        return;
    end;

    UpdateEraseMask(picture);
    attrib.font := font;
    Alter(picture, FontAttrib, attrib);
end AlterFont;     


(* public export *)
procedure DefaultColor(
    const picture : Picture;
    const color   : ColorValue
);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.attrib.color := color;
end DefaultColor;


(* public export *)
procedure GetColor(const picture : Picture) : ColorValue;
begin
    if picture = Picture(nil) then
        return WHITE;
    end;

    return picture^.attrib.color;
end GetColor;


(* public export *)
procedure AlterColor(
    const picture    : Picture;
    const color      : ColorValue
);
    var attrib : Attributes;
begin
    UpdateEraseMask(picture);
    attrib.color := color;
    Alter(picture, Color, attrib);
end AlterColor;


(* public export *)
procedure DefaultBackgroundColor(
    const picture : Picture;
    const color   : ColorValue
);
begin
    picture^.attrib.bkgndColor := color;
end DefaultBackgroundColor;


(* public export *)
procedure GetBackgroundColor(const picture : Picture) : ColorValue;
begin
    if picture = Picture(nil) then
        return WHITE;
    end;

    return picture^.attrib.bkgndColor;
end GetBackgroundColor;


(* public export *)
procedure AlterBackgroundColor(
    const picture    : Picture;
    const color      : ColorValue
);
    var attrib : Attributes;
begin
    if picture = Picture(nil) then
        return;
    end;

    UpdateEraseMask(picture);
    attrib.bkgndColor := color;
    Alter(picture, BkgndColor, attrib);
end AlterBackgroundColor;


(* public export *)
procedure DefaultPaintType(
    const picture   : Picture;
    const paintType : PaintType
);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.attrib.paintType := paintType;
end DefaultPaintType;


(* public export *)
procedure GetPaintType(const picture : Picture) : PaintType;
begin
    if picture = Picture(nil) then
        return Opaque;
    end;

    return picture^.attrib.paintType;
end GetPaintType;


(* public export *)
procedure AlterPaintType(
    const picture   : Picture;
    const paintType : PaintType
);
    var attrib : Attributes;
begin
    if picture = Picture(nil) then
        return;
    end;

    UpdateEraseMask(picture);
    attrib.paintType := paintType;
    Alter(picture, Paint, attrib);
end AlterPaintType;


(* public export *)
procedure DefaultPattern(
    const picture : Picture;
    const pattern : integer
);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.attrib.pattern := pattern;
end DefaultPattern;


(* public export *)
procedure GetPattern(const picture : Picture) : integer;
begin
    if picture = Picture(nil) then
        return 0;
    end;

    return picture^.attrib.pattern;
end GetPattern;


(* public export *)
procedure AlterPattern(
    const picture    : Picture;
    const pattern    : integer
);
    var attrib : Attributes;
begin
    if picture = Picture(nil) then
        return;
    end;

    UpdateEraseMask(picture);
    attrib.pattern := pattern;
    Alter(picture, PatternAttrib, attrib);
end AlterPattern;


(* public export *)
procedure DefaultRenderingStyle(
    const picture : Picture;
    const rendering : RenderingStyle
);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.attrib.rendering := rendering;
end DefaultRenderingStyle;


(* public export *)
procedure GetRenderingStyle(const picture : Picture) : RenderingStyle;
begin
    if picture = Picture(nil) then
        return Fill;
    end;

    return picture^.attrib.rendering;
end GetRenderingStyle;


(* public export *)
procedure AlterRenderingStyle(
    const picture    : Picture;
    const rendering  : RenderingStyle
);
    var attrib : Attributes;
begin
    if picture = Picture(nil) then
        return;
    end;

    UpdateEraseMask(picture);
    attrib.rendering := rendering;
    Alter(picture, Rendering, attrib);
end AlterRenderingStyle;


(* public export *)
procedure DefaultLineStyle(
    const picture   : Picture;
    const lineStyle : integer
);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.attrib.lineStyle := lineStyle;
end DefaultLineStyle;


(* public export *)
procedure GetLineStyle(const picture : Picture) : integer;
begin
    if picture = Picture(nil) then
        return 0;
    end;

    return picture^.attrib.lineStyle;
end GetLineStyle;


(* public export *)
procedure AlterLineStyle(
    const picture   : Picture;
    const lineStyle : integer
);
    var attrib : Attributes;
begin
    if picture = Picture(nil) then
        return;
    end;

    UpdateEraseMask(picture);
    attrib.lineStyle := lineStyle;
    Alter(picture, LineStyleAttrib, attrib);
end AlterLineStyle;



begin

end attribs.
