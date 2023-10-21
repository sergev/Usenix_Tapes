implementation module ops;

import 
    genlists,
    vdi;

(* export *)
from main import
    Picture, XCoord, YCoord, PictureObjectType, GenericList, Word;

from main import
    BoundingBox, TransformMatrix, PointType, FloatPointType, identityMatrix,
    MAXCOMPLEXITY, ColorValue, WHITE, BLACK, PaintType, RenderingStyle,
    solidPattern, ConcatOrder;
    
from bbox import
    MergeBboxes, UpdateParentBboxes, PointInBbox, LoadBbox,
    BboxesIntersect, BboxWithin, CalcPolygonBbox;

from xforms import
    GlobalTransform, ConcatMatrices, 
    TranslateMatrix, ScaleMatrix, RotateMatrix, TransformX, TransformY;

from draw import
    Disable;

from redraw import
    UpdateEraseMask;

from alloc import
    AllocPicture, AllocXform, DeallocPicture, DeallocXform;

from util import
    GetPictList, Root, round, Distance, StringsEqual;


type
    FontMapEntry = pointer to FontMapRecord;
    FontMapRecord = record
	fontid, uid : integer;
    end;

var
    fontMap : genlists.GenericList;
    defaultFont : integer;


(* public export *)
procedure MapFont(const fontid, uid : integer);
    var fontMapEntry : FontMapEntry;
begin
    new(fontMapEntry);
    fontMapEntry^.fontid := fontid;
    fontMapEntry^.uid := uid;
    genlists.Prepend(fontMapEntry, fontMap);
end MapFont;


(* public export *)
procedure UnmapFont(const fontid : integer);
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if fontMapEntry^.fontid = fontid then
            genlists.DeleteCur(i);
 	    dispose(fontMapEntry);
	end;
    end;
    genlists.EndIteration(i);
end UnmapFont;


(* export *)
procedure GetUid(
    const fontid : integer;
    var   uid : integer
) : boolean;
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if fontMapEntry^.fontid = fontid then
	    uid := fontMapEntry^.uid;
	    genlists.EndIteration(i);
	    return true;
	end;
    end;
    genlists.EndIteration(i);
    return false;
end GetUid;


(* export *)
procedure GetFontId(
    const uid: integer;
    var   fontid : integer
) : boolean;
    var i : genlists.Iterator;
        fontMapEntry : FontMapEntry;
begin
    i := genlists.BeginIteration(fontMap);
    while genlists.MoreElements(i, fontMapEntry) do
        if fontMapEntry^.uid = uid then
	    fontid := fontMapEntry^.fontid;
	    genlists.EndIteration(i);
	    return true;
	end;
    end;
    genlists.EndIteration(i);
    return false;
end GetFontId;


(* export *)
procedure InitPictObj(
    const picture : Picture;
    const subpict : Picture;
    const objType : PictureObjectType
);
begin
    subpict^.parent := picture;
    subpict^.altered := true;
    subpict^.redraw := true;
    subpict^.attrib := picture^.attrib;
    AllocXform(subpict^.xform);
    subpict^.xform^ := identityMatrix;
    genlists.Release(subpict^.pictList);	(* don't need it *)
    subpict^.objType := objType;
end InitPictObj;


(* export *)
procedure CopyPictureList(
    const picture : Picture;
    const newParent : Picture
) : GenericList;
    var i : genlists.Iterator;
        duplicate : GenericList;
	subpict, subpictCopy : Picture;
begin
    duplicate := GenericList(nil);
    if picture^.pictList <> GenericList(nil) then
        genlists.Create(duplicate);
        i := genlists.BeginIteration(picture^.pictList);
        while genlists.MoreElements(i, subpict) do
	    subpictCopy := Copy(subpict);
	    subpictCopy^.parent := newParent;
	    genlists.Append(subpictCopy, duplicate);
        end;
        genlists.EndIteration(i);
    end;
    return duplicate;
end CopyPictureList;


(* public export *)
procedure ConvertToScreenCoord(
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
    x := x - root^.attrib.origin.x;
    y := y - root^.attrib.origin.y; 
end ConvertToScreenCoord;


(* public export *)
procedure ConvertToWorldCoord(
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
    x := x + root^.attrib.origin.x;
    y := y + root^.attrib.origin.y;
end ConvertToWorldCoord;


(* public export *)
procedure Create(var picture : Picture);
begin
    AllocPicture(picture);
    picture^.bbox.lowLeft := picture^.bbox.upRight;
    picture^.altered := false;
    picture^.enabled := true;
    picture^.destroyed := false;
    picture^.redraw := true;
    picture^.xform := nil;
    picture^.parent := Picture(nil);
    picture^.eraseMask := Picture(nil);
    picture^.complexity := 0;
    Initialize(picture);
end Create;


(* export *)
procedure DestroyPict(var picture : Picture);
    var i : genlists.Iterator;
        subpict : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;

    if 
        (picture^.objType = PictureList) and 
	(picture^.pictList # GenericList(nil))
    then
        i := genlists.BeginIteration(picture^.pictList);
	while genlists.MoreElements(i, subpict) do
	    DestroyPict(subpict);
	end;
        genlists.EndIteration(i);
	genlists.Release(picture^.pictList);
	DestroyPict(picture^.eraseMask);

    elsif
        (picture^.objType = PolygonObj) or
	(picture^.objType = BSplineObj) or
	(picture^.objType = ClosedBSplineObj)
    then
        dispose(picture^.vertex.x);
	dispose(picture^.vertex.y);

    elsif picture^.objType = TextObj then
        dispose(picture^.text.chars);
    end;
    
    if picture^.xform # nil then
        DeallocXform(picture^.xform);
    end;
    DeallocPicture(picture);
end DestroyPict;


(* public export *)
procedure Destroy(var picture : Picture);
    var i : genlists.Iterator;
begin
    if (picture = Picture(nil)) or picture^.destroyed then
	picture := Picture(nil);
    elsif picture^.parent = Picture(nil) then
        DestroyPict(picture);
    else
        Disable(picture);
        picture^.destroyed := true;
	UpdateParentBboxes(picture);
	picture := Picture(nil);
    end;
end Destroy;


(* public export *)
procedure Initialize(const picture : Picture);
begin
    if picture = Picture(nil) then
        return;
    end;

    with picture^.attrib do
	font := defaultFont;
        color := BLACK;
        bkgndColor := WHITE;
        paintType := Opaque;
        pattern := solidPattern;
        rendering := Stroke;
        lineStyle := 1;
        curPos.x := 0;
        curPos.y := 0;
        origin.x := 0;
        origin.y := 0;
    end;
    picture^.objType := PictureList;
    genlists.Create(picture^.pictList);
end Initialize;


(* public export *)
procedure Nest(
    const sourcePicture : Picture;
    const destPicture : Picture
);
    var complexity : integer;
begin
    if (sourcePicture = Picture(nil)) or (destPicture = Picture(nil)) then
	return;
    end;

    complexity := destPicture^.complexity + sourcePicture^.complexity + 1;
    if complexity > MAXCOMPLEXITY then
        complexity := MAXCOMPLEXITY;
    end;
    destPicture^.complexity := complexity;

    genlists.Append(sourcePicture, destPicture^.pictList);
    destPicture^.bbox := MergeBboxes(destPicture^.bbox, sourcePicture^.bbox);
    sourcePicture^.parent := destPicture;
    UpdateParentBboxes(destPicture);
end Nest;


(* public export *)
procedure Unnest(var picture : Picture);
begin
    if 
        (picture = Picture(nil)) or 
	(picture^.parent = Picture(nil)) or
	(picture^.objType # PictureList)
    then
        return;
    end;
    
    genlists.Delete(picture, picture^.parent^.pictList);
    UpdateParentBboxes(picture);
    picture^.parent := Picture(nil);
end Unnest;


(* public export *)
procedure Insert(
    const sourcePicture : Picture;
    const followingPicture : Picture
);
    var complexity : integer;
        parent, subpict : Picture;
	i : genlists.Iterator;
	found : boolean;
begin
    if  (sourcePicture = Picture(nil)) or 
        (followingPicture = Picture(nil)) or
	(followingPicture^.parent = Picture(nil)) then
	return;
    end;

    found := false;
    parent := followingPicture^.parent;
    complexity := parent^.complexity + sourcePicture^.complexity + 1;
    if complexity > MAXCOMPLEXITY then
        complexity := MAXCOMPLEXITY;
    end;
    parent^.complexity := complexity;

    i := genlists.BeginIteration(parent^.pictList);
    while not found and genlists.MoreElements(i, subpict) do
  	if subpict = followingPicture then
	    found := true;
	end;
    end;
    found := genlists.PrevElement(i, subpict);
    genlists.Insert(sourcePicture, i);
    genlists.EndIteration(i);
    sourcePicture^.parent := parent;
    UpdateParentBboxes(sourcePicture);
end Insert;


(* public export *)
procedure Copy(const picture : Picture) : Picture;
    var i : genlists.Iterator;
    	duplicate : Picture;
	j : integer;
begin
    if picture = Picture(nil) then
        return nil;
    end;
    
    Create(duplicate);
    duplicate^ := picture^;
    duplicate^.altered := false;
    duplicate^.parent := Picture(nil);
    duplicate^.eraseMask := Picture(nil);
    
    if picture^.objType = PictureList then
        duplicate^.pictList := CopyPictureList(picture, duplicate);
	duplicate^.xform := nil;
    else
        AllocXform(duplicate^.xform);
	duplicate^.xform^ := picture^.xform^;
	
	if 
	    (picture^.objType = PolygonObj) or
	    (picture^.objType = BSplineObj) or
	    (picture^.objType = ClosedBSplineObj)
	then
	    new(duplicate^.vertex.x, number(picture^.vertex.x^));
	    new(duplicate^.vertex.y, number(picture^.vertex.y^));
	    for j := 0 to high(picture^.vertex.x^) do
	        duplicate^.vertex.x^[j] := picture^.vertex.x^[j];
	        duplicate^.vertex.y^[j] := picture^.vertex.y^[j];
	    end;
	elsif picture^.objType = TextObj then
	    new(duplicate^.text.chars, number(picture^.text.chars^));
	    for j := 0 to high(picture^.text.chars^) do
	        duplicate^.text.chars^[j] := picture^.text.chars^[j];
	    end;
	end;
    end;
    return duplicate;
end Copy;


(* public export *)
procedure SendToBack(
    const picture    : Picture;
    var   subPicture : Picture
);
    var	pictList : GenericList;
	found : boolean;
begin
    if (picture = Picture(nil)) or (picture^.objType # PictureList) then
        return;
    end;

    found := false;
    pictList := GetPictList(picture, subPicture, found);
    if found then
        UpdateEraseMask(subPicture);
        genlists.Delete(subPicture, pictList);
        genlists.Prepend(subPicture, picture^.pictList);
	subPicture^.altered := true;
    end;
end SendToBack;


(* public export *)
procedure BringToFront(
    const picture    : Picture;
    var   subPicture : Picture
);
    var	pictList : GenericList;
	found : boolean;
begin
    if (picture = Picture(nil)) or (picture^.objType # PictureList) then
        return;
    end;

    found := false;
    pictList := GetPictList(picture, subPicture, found);
    if found then
        UpdateEraseMask(subPicture);
        genlists.Delete(subPicture, pictList);
        genlists.Append(subPicture, picture^.pictList);
	subPicture^.altered := true;
    end;
end BringToFront;


(* public export *)
procedure SelectByPoint(
    const picture    : Picture;
    const x          : XCoord; 
    const y          : YCoord
) : GenericList;
    var i : genlists.Iterator;
        subpict : Picture;
	selections : GenericList;
begin
    if 
	(picture = Picture(nil)) or
	(not picture^.enabled) or
	(not PointInBbox(x, y, picture^.bbox))
    then
        return GenericList(nil);
    end;
    
    genlists.Create(selections);
    if picture^.objType = PictureList then
        i := genlists.BeginReverseIteration(picture^.pictList);
        while genlists.PrevElement(i, subpict) do
            if 
	        not subpict^.destroyed and subpict^.enabled and
		(subpict^.objType = PictureList) and
		PointInBbox(x, y, subpict^.bbox) 
	    then
	        genlists.Append(subpict, selections);
	    end;
	end;
        genlists.EndIteration(i);
    end;
    if Picture(genlists.Head(selections)) = Picture(nil) then
        genlists.Release(selections);
    end;
    return selections;
end SelectByPoint;


(* public export *)
procedure SelectByBoundingBox(
    const picture    : Picture;
    const left       : XCoord;
    const bottom     : YCoord;
    const right	     : XCoord;
    const top	     : YCoord
) : GenericList;
    var i : genlists.Iterator;
        subpict : Picture;
	selections : GenericList;
	selectionBbox : BoundingBox;
begin
    if (picture = Picture(nil)) or not picture^.enabled then
        return GenericList(nil);
    end;
    selectionBbox.lowLeft.x := left;
    selectionBbox.lowLeft.y := bottom;
    selectionBbox.upRight.x := right;
    selectionBbox.upRight.y := top;
    if not BboxesIntersect(selectionBbox, picture^.bbox) then
        return GenericList(nil);
    end;
    
    genlists.Create(selections);
    if picture^.objType = PictureList then
        i := genlists.BeginReverseIteration(picture^.pictList);
        while genlists.PrevElement(i, subpict) do
            if 
	        not subpict^.destroyed and subpict^.enabled and
		(subpict^.objType = PictureList) and
	        BboxesIntersect(selectionBbox, subpict^.bbox) 
	    then
	        genlists.Append(subpict, selections);
	    end;
	end;
        genlists.EndIteration(i);
    end;
    if Picture(genlists.Head(selections)) = Picture(nil) then
        genlists.Release(selections);
    end;
    return selections;
end SelectByBoundingBox;


(* public export *)
procedure SelectWithinBoundingBox(
    const picture    : Picture;
    const left       : XCoord;
    const bottom     : YCoord;
    const right	     : XCoord;
    const top	     : YCoord
) : GenericList;
    var i : genlists.Iterator;
        subpict : Picture;
	selections : GenericList;
	selectionBbox : BoundingBox;
begin
    if (picture = Picture(nil)) or not picture^.enabled  then
        return GenericList(nil);
    end;
    selectionBbox.lowLeft.x := left;
    selectionBbox.lowLeft.y := bottom;
    selectionBbox.upRight.x := right;
    selectionBbox.upRight.y := top;
    if not BboxesIntersect(selectionBbox, picture^.bbox) then
        return GenericList(nil);
    end;
    
    genlists.Create(selections);
    if picture^.objType = PictureList then
        i := genlists.BeginReverseIteration(picture^.pictList);
        while genlists.PrevElement(i, subpict) do
            if 
	        not subpict^.destroyed and subpict^.enabled and
		(subpict^.objType = PictureList) and
		BboxWithin(subpict^.bbox, selectionBbox) 
	    then
	        genlists.Append(subpict, selections);
	    end;
	end;
        genlists.EndIteration(i);
    end;
    if Picture(genlists.Head(selections)) = Picture(nil) then
        genlists.Release(selections);
    end;
    return selections;
end SelectWithinBoundingBox;


(* public export *)
procedure SelectAll(const picture : Picture) : GenericList;
    var i : genlists.Iterator;
        subpict : Picture;
	selections : GenericList;
begin
    if (picture = Picture(nil)) and subpict^.enabled then
        return GenericList(nil);
    end;
    
    genlists.Create(selections);
    if picture^.objType = PictureList then
        i := genlists.BeginIteration(picture^.pictList);
        while genlists.MoreElements(i, subpict) do
            if
	        not subpict^.destroyed and subpict^.enabled and
		(subpict^.objType = PictureList)
	    then
	        genlists.Append(subpict, selections);
	    end;
	end;
        genlists.EndIteration(i);
    end;
    if Picture(genlists.Head(selections)) = Picture(nil) then
        genlists.Release(selections);
    end;
    return selections;
end SelectAll;


(*
 *  OrderSelections sorts the subpicts in the given list by drawing order.
 *  It only sorts those subpict that are in the given picture's PictureList;
 *  deeply nested subpicts are just appended after the sorted ones.
 *)
(* public export *)
procedure OrderSelections(
    var   selections : GenericList;
    const picture : Picture
);
    var newlist : GenericList;
        i, j : genlists.Iterator;
	p, selec : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;
    genlists.Create(newlist);
    i := genlists.BeginIteration(picture^.pictList);
    loop
        if 
	    not genlists.MoreElements(i, p) or
	    (Picture(genlists.Head(selections)) = Picture(nil))
	then
	    exit;
	end;
	j := genlists.BeginIteration(selections);
	loop
	    if not genlists.MoreElements(j, selec) then
	        exit;
	    elsif selec = p then
	        genlists.Append(selec, newlist);
		genlists.DeleteCur(j);
	        exit;
	    end;
	end;
	genlists.EndIteration(j);
    end;
    genlists.EndIteration(i);

(*  Append remaining (deeply nested) selections *)
    i := genlists.BeginIteration(selections);
    while genlists.MoreElements(i, selec) do
        genlists.Append(selec, newlist);
	genlists.DeleteCur(i);
    end;
    genlists.Release(selections);
    selections := newlist;
end OrderSelections;	    


(* public export *)
procedure Translate(
    const picture    : Picture;
    const dx, dy     : real
);
begin
    if 
        (picture = Picture(nil)) or
	((dx = 0.0) and (dy = 0.0))
    then
        return;
    end;

    UpdateEraseMask(picture);
    GlobalTransform(picture, TranslateMatrix(dx, dy), After);
    UpdateParentBboxes(picture);
end Translate;


(* public export *)
procedure Scale(
    const picture    : Picture;
    const sx, sy     : real
);
    var xform : TransformMatrix;
        center : FloatPointType;
begin
    if 
        (picture = Picture(nil)) or
	((sx = 1.0) and (sy = 1.0))
    then
        return;
    end;

    UpdateEraseMask(picture);
    center := picture^.bbox.center;
    xform := 
        ConcatMatrices(
            ConcatMatrices(
	    	TranslateMatrix(-center.x, -center.y),
	        ScaleMatrix(sx, sy)
	    ),
	    TranslateMatrix(center.x, center.y)
	);	    
    GlobalTransform(picture, xform, After);
    picture^.bbox.center := center;
    UpdateParentBboxes(picture);
end Scale;


(* public export *)
procedure Rotate(
    const picture : Picture;
    const angle   : real
);
    var xform : TransformMatrix;
        center : FloatPointType;
begin
    if 
        (picture = Picture(nil)) or
	(angle = 0.0)
    then
        return;
    end;

    UpdateEraseMask(picture);
    center := picture^.bbox.center;
    xform := 
        ConcatMatrices(
            ConcatMatrices(
	    	TranslateMatrix(-center.x, -center.y),
	        RotateMatrix(angle)
	    ),
	    TranslateMatrix(center.x, center.y)
	);	    
    GlobalTransform(picture, xform, After);
    picture^.bbox.center := center;
    UpdateParentBboxes(picture);
end Rotate;


(* public export *)
procedure ScaleAboutPoint(
    const picture      : Picture;
    const sx, sy, x, y : real
);
    var xform : TransformMatrix;
begin
    if 
        (picture = Picture(nil)) or
	((sx = 1.0) and (sy = 1.0))
    then
        return;
    end;

    UpdateEraseMask(picture);
    xform := 
        ConcatMatrices(
            ConcatMatrices(TranslateMatrix(-x, -y), ScaleMatrix(sx, sy)),
	    TranslateMatrix(x, y)
	);	    
    GlobalTransform(picture, xform, After);
    UpdateParentBboxes(picture);
end ScaleAboutPoint;


(* public export *)
procedure RotateAboutPoint(
    const picture     : Picture;
    const angle, x, y : real
);
    var xform : TransformMatrix;
begin
    if 
        (picture = Picture(nil)) or
	(angle = 0.0)
    then
        return;
    end;

    UpdateEraseMask(picture);
    xform := 
        ConcatMatrices(
            ConcatMatrices(TranslateMatrix(-x, -y), RotateMatrix(angle)),
	    TranslateMatrix(x, y)
	);	    
    GlobalTransform(picture, xform, After);
    UpdateParentBboxes(picture);
end RotateAboutPoint;


(* public export *)
procedure GetNestedPicture(
    const picture    : Picture;
    const number     : integer
) : Picture;
    var i : genlists.Iterator;
        subpict : Picture;
	more : boolean;
        n : integer;
begin
    if picture = Picture(nil) then 
        return picture;
    end;

    if picture^.objType = PictureList then
        n := 1;
	if number > 0 then
            i := genlists.BeginIteration(picture^.pictList);
  	else
	    i := genlists.BeginReverseIteration(picture^.pictList);
	end;
	repeat
	    if number > 0 then
	        more := genlists.MoreElements(i, subpict);
	    else
	        more := genlists.PrevElement(i, subpict);
	    end;
    	    if
	         more and not subpict^.destroyed and 
		 (n <= abs(number)) and (subpict^.objType = PictureList)
	    then
	        if n = abs(number) then
	            genlists.EndIteration(i);
	            return subpict;
	        else
	            n := n + 1;
		end;
	    end;
        until not more or (n > abs(number));
        genlists.EndIteration(i);
    end;
    return Picture(nil);
end GetNestedPicture;


(* public export *)
procedure GetNestedPictureNumber(
    const picture : Picture;
    const subpict : Picture
) : integer;
    var i : genlists.Iterator;
        testpict : Picture;
	n : integer;
begin
    if
        (picture # Picture(nil)) and (subpict # Picture(nil)) and
	(picture^.objType = PictureList)
    then
        n := 0;
        i := genlists.BeginIteration(picture^.pictList);
        while genlists.MoreElements(i, testpict) do
            n := n + 1;
            if testpict = subpict then
	        genlists.EndIteration(i);
	        return n;
	    end;
	end;
	genlists.EndIteration(i);
    end;
    return 0;
end GetNestedPictureNumber;


(* public export *)
procedure NumberOfNestedPictures(const picture : Picture) : integer;
    var i : genlists.Iterator;
        p : Picture;
	n : integer;
begin
    n := 0;
    if (picture # Picture(nil)) and (picture^.objType = PictureList) then
        i := genlists.BeginIteration(picture^.pictList);
	while genlists.MoreElements(i, p) do
	    if (p^.objType = PictureList) and not p^.destroyed then
		n := n + 1;
	    end;
  	end;
	genlists.EndIteration(i);
    end;
    return n;
end NumberOfNestedPictures;


(* public export *)
procedure GetNextSibling(const picture : Picture) : Picture;
    var i : genlists.Iterator;
	p : Picture;
	moreSibs : boolean;
begin
    if (picture # Picture(nil)) and (picture^.parent # Picture(nil)) then
	i := genlists.BeginIteration(picture^.parent^.pictList);
	while genlists.MoreElements(i, p) do
	    if p = picture then
		loop
		    moreSibs := genlists.MoreElements(i, p);
		    if moreSibs and not p^.destroyed then
			exit;
		    elsif not moreSibs then
			p := Picture(nil);
			exit;
		    end;
		end;
		genlists.EndIteration(i);
		return p;
	    end;
	end;
	genlists.EndIteration(i);
    end;
    return Picture(nil);
end GetNextSibling;	
        

(* public export *)
procedure ContainsPrimitives(const picture : Picture) : boolean;
    var i : genlists.Iterator;
        p : Picture;
begin
    if (picture = Picture(nil)) or (picture^.objType # PictureList) then
        return true;
    else
        i := genlists.BeginIteration(picture^.pictList);
	while genlists.MoreElements(i, p) do
	    if (p^.objType # PictureList) and not p^.destroyed then
	        genlists.EndIteration(i);
		return true;
	    end;
  	end;
	genlists.EndIteration(i);
    end;
    return false;
end ContainsPrimitives;
        

(* public export *)
procedure SetUserData(
    const picture : Picture;
    const userData : Word
);
begin
    picture^.attrib.userData := userData;
end SetUserData;


(* public export *)
procedure GetUserData(const picture : Picture) : Word;
begin
    return picture^.attrib.userData;
end GetUserData;


(* public export *)
procedure GetPictureType(const p : Picture) : PictureObjectType;
begin
    if p = Picture(nil) then
        return None;
    end;
    return p^.objType;
end GetPictureType;


(* public export *)
procedure GetPrimitiveType(
    const p : Picture;
    const nestPictNum : integer
) : PictureObjectType;
    var i : genlists.Iterator;
        j : integer;
	leaf : Picture;
begin
    if p = Picture(nil) then
        return None;
    end;
    j := 0;
    i := genlists.BeginIteration(p^.pictList);
    loop
        j := j + 1;
	if not genlists.MoreElements(i, leaf) then
	    exit;
	elsif j = nestPictNum then
	    genlists.EndIteration(i);
	    return leaf^.objType;
	end;
    end;
    genlists.EndIteration(i);
    return None;
end GetPrimitiveType;


(* public export *)
procedure GetNumberOfVertices(
    const p : Picture;
    const nestPictNum : integer
) : integer;
    var i : genlists.Iterator;
        j : integer;
	leaf : Picture;
begin
    if p = Picture(nil) then
        return 0;
    end;
    j := 0;
    i := genlists.BeginIteration(p^.pictList);
    loop
        j := j + 1;
	if not genlists.MoreElements(i, leaf) then
	    exit;
	elsif j = nestPictNum then
	    if
	        (leaf^.objType = PolygonObj) or
	        (leaf^.objType = BSplineObj) or
	        (leaf^.objType = ClosedBSplineObj)
	    then
	        genlists.EndIteration(i);
	        return number(leaf^.vertex.x);
	    else
	        exit;
	    end;
	end;
    end;
    genlists.EndIteration(i);
    return 0;
end GetNumberOfVertices;


(* public export *)
procedure GetVertices(
    const p : Picture;
    const nestPictNum : integer;
    var   x : array of XCoord;
    var   y : array of YCoord
) : boolean;
    var i : genlists.Iterator;
        j, k, size : integer;
	leaf : Picture;
begin
    if p = Picture(nil) then
        return false;
    end;
    j := 0;
    i := genlists.BeginIteration(p^.pictList);
    loop
        j := j + 1;
	if not genlists.MoreElements(i, leaf) then
	    exit;
	elsif j = nestPictNum then
	    if
	        (leaf^.objType = PolygonObj) or
	        (leaf^.objType = BSplineObj) or
	        (leaf^.objType = ClosedBSplineObj)
	    then
	        size := min(number(leaf^.vertex.x), number(x));
	        for k := 0 to size - 1 do
		    x[k] := TransformX(
		        leaf^.vertex.x^[k], leaf^.vertex.y^[k], leaf^.xform^
		    );
		    y[k] := TransformY(
		        leaf^.vertex.x^[k], leaf^.vertex.y^[k], leaf^.xform^
		    );
		end;
	        genlists.EndIteration(i);
	        return true;
	    else
	        exit;
	    end;
	end;
    end;
    genlists.EndIteration(i);
    return false;
end GetVertices;


(* public export *)
procedure MoveVertex(
    const p : Picture;
    const nestPictNum : integer;
    const vertex : integer;
    const dx : XCoord;
    const dy : YCoord
);
    var i : genlists.Iterator;
        j, size : integer;
	l : Picture;		(* l *)
	fdx, fdy : real;	(* dx & dy in untransformed coordinates *)
	det : real;
begin
    if 
        (p = Picture(nil)) or (vertex < 0) or
	((dx = 0) and (dy = 0))
    then
        return;
    end;
    j := 0;
    i := genlists.BeginIteration(p^.pictList);
    loop
        j := j + 1;
	if not genlists.MoreElements(i, l) then
	    exit;
	elsif j = nestPictNum then
	    if not (
	        (l^.objType = LineObj) or (l^.objType = PolygonObj) or
		(l^.objType = BSplineObj) or 
		(l^.objType = ClosedBSplineObj)
	    ) or (
	        (l^.objType = LineObj) and (vertex # 0) and (vertex # 1)
	    ) then
	    	exit;
	    end;
	    if l^.objType # LineObj then
		size := number(l^.vertex.x^);
	        if vertex >= size then
		    exit;
		end;
	    end;

	    UpdateEraseMask(p);
	    p^.altered := true;
	    l^.altered := true;
	    det := l^.xform^[1,1]*l^.xform^[2,2] - l^.xform^[1,2]*l^.xform^[2,1];
	    fdy := (l^.xform^[1,1]*float(dy) - l^.xform^[1,2]*float(dx)) / det;
	    fdx := (float(dx) - fdy*l^.xform^[2,1]) / l^.xform^[1,1];
	    if l^.objType = LineObj then
	        if vertex = 0 then
		    l^.line.p1.x := l^.line.p1.x + round(fdx);
		    l^.line.p1.y := l^.line.p1.y + round(fdy);
		else
		    l^.line.p2.x := l^.line.p2.x + round(fdx);
		    l^.line.p2.y := l^.line.p2.y + round(fdy);
		end;
		LoadBbox(
		    l^.bbox, 
		    l^.line.p1.x, l^.line.p1.y, l^.line.p2.x, l^.line.p2.y
		);
	    else
		l^.vertex.x^[vertex] := l^.vertex.x^[vertex] + round(fdx);
		l^.vertex.y^[vertex] := l^.vertex.y^[vertex] + round(fdy);
		l^.bbox := CalcPolygonBbox(
		    l^.vertex.x^[0:size], l^.vertex.y^[0:size]
		);
	    end;
	    UpdateParentBboxes(l);
	    exit;
	end;
    end;
    genlists.EndIteration(i);        
end MoveVertex;


begin
    genlists.Create(fontMap);
    defaultFont := vdi.GetFont();
end ops.
