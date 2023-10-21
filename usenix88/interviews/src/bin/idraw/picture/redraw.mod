implementation module redraw;

(* export *)
from main import
    Picture, XCoord, YCoord;

from main import
    BoundingBox, BboxPtr, PictureObjectType, MAXCOMPLEXITY, PaintType, 
    ColorValue, originX, originY, GenericList, WHITE, solidPattern,
    RenderingStyle;

from util import
    Root;

from alloc import
    AllocBbox, DeallocBbox;

from ops import
    Copy, Create, Initialize, Nest, DestroyPict;

from objs import
    Rectangle;

from draw import
    Draw, DrawPict, DrawInBoundingBox, DrawInBbox;

from bbox import
    CalcBbox, MergeBboxes, BboxesIntersect, IntersectBboxes;

from vdi import
    RenderingFunction;

from xforms import
    TransformBbox;

import
    genlists,
    vdi;


procedure CollectEraseMasks(
    const picture : Picture;
    const newEraseMask : Picture
);
    var i : genlists.Iterator;
        subpict, erasePict : Picture;
	bbox : BoundingBox;
begin
    if  
        (picture = Picture(nil)) or 
        (newEraseMask = Picture(nil)) or
	not picture^.redraw
    then
        return;
    end;

    if (picture^.objType # PictureList) and not picture^.altered then
(*
 *  Create eraseMasks for primitive objects that HAVE already been drawn.
 *  Primitive objects with altered = true but whose parents have not been
 *  altered are newly added objects (and have not yet been drawn), so
 *  there's no need to make an eraseMask for them.
 *)
        if newEraseMask^.complexity < MAXCOMPLEXITY then
            Create(erasePict);
	    erasePict^.attrib.color := picture^.attrib.bkgndColor;
	    erasePict^.attrib.bkgndColor := picture^.attrib.bkgndColor;
	    erasePict^.attrib.paintType := Opaque;
	    erasePict^.attrib.rendering := Fill;
	    bbox := CalcBbox(picture);
	    Rectangle(
		erasePict,
		bbox.lowLeft.x, bbox.lowLeft.y,	bbox.upRight.x, bbox.upRight.y
	    );
	    Nest(erasePict, newEraseMask);
	else
	    newEraseMask^.bbox := MergeBboxes(
	        newEraseMask^.bbox, CalcBbox(picture)
	    );
	end;
    elsif picture^.objType = PictureList then
        i := genlists.BeginIteration(picture^.pictList);
        while genlists.MoreElements(i, subpict) do
            if subpict^.altered then
	        Nest(subpict^.eraseMask, newEraseMask);
	        subpict^.eraseMask := Picture(nil);
	    end;
	    CollectEraseMasks(subpict, newEraseMask);
        end;
	genlists.EndIteration(i);
    end;
end CollectEraseMasks; 
    

procedure AddToBoxList(
    const bbox : BoundingBox;
    const boxlist : GenericList
);
    var existingBbox, newBbox : BboxPtr;
        i : genlists.Iterator;
    	found : boolean;
begin
    found := false;
    i := genlists.BeginIteration(boxlist);
    while genlists.MoreElements(i, existingBbox) and not found do
	if BboxesIntersect(existingBbox^, bbox) then
	    existingBbox^ := MergeBboxes(existingBbox^, bbox);
	    found := true;
        end;
    end;
    genlists.EndIteration(i);
    if not found then
        AllocBbox(newBbox);
	newBbox^ := bbox;
	genlists.Append(newBbox, boxlist);
    end;
end AddToBoxList;


procedure DestroyBoxList(var boxlist : GenericList);
    var i : genlists.Iterator;
        bbox : BboxPtr;
begin
    i := genlists.BeginIteration(boxlist);
    while genlists.MoreElements(i, bbox) do
	genlists.DeleteCur(i);
        DeallocBbox(bbox);
    end;
    genlists.EndIteration(i);
    genlists.Release(boxlist);
end DestroyBoxList;


procedure DrawEraseMasks(
    const root    : Picture;
    const picture : Picture;
    const boxlist : GenericList
);
    var i : genlists.Iterator;
        subpict : Picture;
begin
    if 
        (picture = Picture(nil)) or
	(root = Picture(nil)) or
	(picture^.objType # PictureList) or
	not picture^.redraw
    then
        return;
    end;

    if picture^.altered and (picture^.eraseMask # Picture(nil)) then
        if picture^.eraseMask^.complexity = MAXCOMPLEXITY then
            vdi.SetColors(WHITE, WHITE);
            vdi.SetRendering(PAINTFGBG);
	    with picture^.eraseMask^.bbox do
	        vdi.FilledRectangle(
	            lowLeft.x + originX, lowLeft.y + originY, 
		    upRight.x + originX, upRight.y + originY
	        );
	    end;
        else
            DrawPict(picture^.eraseMask);
        end;
	AddToBoxList(picture^.eraseMask^.bbox, boxlist);

    elsif picture^.objType = PictureList then
        i := genlists.BeginIteration(picture^.pictList);
	while genlists.MoreElements(i, subpict) do
	    DrawEraseMasks(root, subpict, boxlist);
	end;
	genlists.EndIteration(i);
    end;
    
    DestroyPict(picture^.eraseMask);
end DrawEraseMasks;    


procedure DrawEraseMasksInBbox(
    const root    : Picture;
    const picture : Picture;
    const boxlist : GenericList;
          ubbox   : BoundingBox
);
    var i : genlists.Iterator;
        subpict : Picture;
	newbbox : BoundingBox;
begin
    if 
        (picture = Picture(nil)) or
	(root = Picture(nil)) or
	(picture^.objType # PictureList) or
	not picture^.redraw
    then
        return;
    end;

    if picture^.altered and (picture^.eraseMask # Picture(nil)) then
        newbbox := IntersectBboxes(ubbox, picture^.eraseMask^.bbox);
        if picture^.eraseMask^.complexity = MAXCOMPLEXITY then
            vdi.SetColors(WHITE, WHITE);
            vdi.SetRendering(PAINTFGBG);
	    vdi.FilledRectangle(
	        newbbox.lowLeft.x + originX, newbbox.lowLeft.y + originY, 
		newbbox.upRight.x + originX, newbbox.upRight.y + originY
	    );
        else
            DrawInBbox(
	        picture^.eraseMask,
		newbbox.lowLeft.x, newbbox.lowLeft.y, 
		newbbox.upRight.x, newbbox.upRight.y
	    );
        end;
	AddToBoxList(newbbox, boxlist);

    elsif picture^.objType = PictureList then
        i := genlists.BeginIteration(picture^.pictList);
	while genlists.MoreElements(i, subpict) do
	    DrawEraseMasksInBbox(root, subpict, boxlist, ubbox);
	end;
	genlists.EndIteration(i);
    end;
    
    DestroyPict(picture^.eraseMask);
end DrawEraseMasksInBbox;


procedure DestroyEraseMasks(const picture : Picture);
    var i : genlists.Iterator;
        subpict : Picture;
begin
    if picture = Picture(nil) then
        return;
    end;

    DestroyPict(picture^.eraseMask);
    picture^.altered := false;
    if picture^.objType # PictureList then
        return;
    end;

    i := genlists.BeginIteration(picture^.pictList);
    while genlists.MoreElements(i, subpict) do
	DestroyEraseMasks(subpict);
    end;
    genlists.EndIteration(i);
end DestroyEraseMasks; 
    

(* export *)
procedure UpdateEraseMask(picture : Picture);
    var erasedPict : Picture;
begin
    if 
        (picture = Picture(nil)) or 
	picture^.altered or
	not picture^.redraw
    then
        return;
    elsif
        (picture^.objType # PictureList) and
	not picture^.parent^.altered and
	picture^.parent^.redraw
    then
        picture := picture^.parent;
    end;

    DestroyPict(picture^.eraseMask);
    Create(picture^.eraseMask);
    CollectEraseMasks(picture, picture^.eraseMask);
end UpdateEraseMask;


procedure TouchPict(const p : Picture);
    var i : genlists.Iterator;
        subpict : Picture;
begin
    if p # Picture(nil) then
	p^.altered := true;

	if p^.objType = PictureList then
	    i := genlists.BeginIteration(p^.pictList);
	    while genlists.MoreElements(i, subpict) do
	        TouchPict(subpict);
	    end;
	    genlists.EndIteration(i);
	end;
    end;
end TouchPict;


(* public export *)
procedure Touch(const p : Picture);
begin
    if (p # Picture(nil)) and not p^.destroyed then
        UpdateEraseMask(p);
        TouchPict(p);
    end;
end Touch;


procedure AddChangedBboxes(
    const root    : Picture;
    const boxlist : GenericList
);
    var i : genlists.Iterator;
        bbox : BoundingBox;
	subpict : Picture;
begin
    if (root = Picture(nil)) or (root^.objType # PictureList) then
  	return;
    end;
    i := genlists.BeginIteration(root^.pictList);
    while genlists.MoreElements(i, subpict) do
	if subpict^.objType = PictureList then
	    AddChangedBboxes(subpict, boxlist);
	elsif subpict^.altered then
	    bbox := TransformBbox(subpict^.bbox, subpict^.xform^);
	    AddToBoxList(bbox, boxlist);
	end;
    end;
    genlists.EndIteration(i);
end AddChangedBboxes;


procedure AddChangedBboxesInBbox(
    const root    : Picture;
    const boxlist : GenericList;
    const ubbox	  : BoundingBox
);
    var i : genlists.Iterator;
        bbox : BoundingBox;
	subpict : Picture;
begin
    if (root = Picture(nil)) or (root^.objType # PictureList) then
  	return;
    end;
    if BboxesIntersect(root^.bbox, ubbox) then
        i := genlists.BeginIteration(root^.pictList);
        while genlists.MoreElements(i, subpict) do
	    if subpict^.objType = PictureList then
	        AddChangedBboxesInBbox(subpict, boxlist, ubbox);
	    elsif subpict^.altered then
	        bbox := TransformBbox(subpict^.bbox, subpict^.xform^);
		bbox := IntersectBboxes(bbox, ubbox);
		if 
		    (bbox.lowLeft.x # bbox.upRight.x) or
		    (bbox.lowLeft.y # bbox.upRight.y)
		then
	            AddToBoxList(bbox, boxlist);
		end;
	    end;
	end;
	genlists.EndIteration(i);
    end;
end AddChangedBboxesInBbox;


procedure DrawChanges(
    const root    : Picture;
    const boxlist : GenericList
);
    var subpict : Picture;
        i : genlists.Iterator;
	bbox : pointer to BoundingBox;
begin
    if 
        (root = Picture(nil)) or 
	(root^.objType # PictureList) or
	not root^.enabled
    then
	return;
    end;

    i := genlists.BeginIteration(boxlist);
    while genlists.MoreElements(i, bbox) do
	DrawInBoundingBox(
	    root, 
	    bbox^.lowLeft.x, bbox^.lowLeft.y, 
	    bbox^.upRight.x, bbox^.upRight.y
	);
    end;
    genlists.EndIteration(i);
end DrawChanges;


procedure DrawChangesInBbox(
    const root    : Picture;
    const boxlist : GenericList;
    const bbox    : BoundingBox
);
    var subpict : Picture;
        i : genlists.Iterator;
	bboxptr : pointer to BoundingBox;
begin
    if 
        (root = Picture(nil)) or 
	(root^.objType # PictureList) or
	not root^.enabled
    then
	return;
    end;

    i := genlists.BeginIteration(boxlist);
    while genlists.MoreElements(i, bboxptr) do
	bboxptr^ := IntersectBboxes(bbox, bboxptr^);
	DrawInBoundingBox(
	    root, 
	    bboxptr^.lowLeft.x, bboxptr^.lowLeft.y, 
	    bboxptr^.upRight.x, bboxptr^.upRight.y
	);
    end;
    genlists.EndIteration(i);
end DrawChangesInBbox;


(* export *)
procedure IncrementComplexity(const picture : Picture);
    var complexity : integer;
begin
    complexity := picture^.complexity + 1;
    if complexity > MAXCOMPLEXITY then
        complexity := MAXCOMPLEXITY;
    end;
    picture^.complexity := complexity;
end IncrementComplexity;
    

(* public export *)
procedure EnableRedraw(const picture : Picture);
begin
    if picture = Picture(nil) then
        return;
    end;

    DestroyEraseMasks(picture);
    picture^.redraw := true;
end EnableRedraw;


(* public export *)
procedure DisableRedraw(const picture : Picture);
begin
    if picture = Picture(nil) then
        return;
    end;

    picture^.redraw := false;
end DisableRedraw;


(* public export *)
procedure Redraw(const picture : Picture);
    var root : Picture;
        boxlist : GenericList;
begin
    if 
        (picture = Picture(nil)) or 
	not picture^.redraw
    then
        return;
    end;
    
    root := Root(picture);
    originX := root^.attrib.origin.x;
    originY := root^.attrib.origin.y;
    genlists.Create(boxlist);
    DrawEraseMasks(root, root, boxlist);
    AddChangedBboxes(root, boxlist);
    DrawChanges(root, boxlist);
    DestroyBoxList(boxlist);
end Redraw;


(* public export *)
procedure RedrawInBoundingBox(
    const picture : Picture;
    const x0 : XCoord;
    const y0 : YCoord;
    const x1 : XCoord;
    const y1 : YCoord    
);
    var root : Picture;
        boxlist : GenericList;
	bbox : BoundingBox;
begin
    if 
        (picture = Picture(nil)) or 
	not picture^.redraw
    then
        return;
    end;
    
    root := Root(picture);
    originX := root^.attrib.origin.x;
    originY := root^.attrib.origin.y;
    genlists.Create(boxlist);
    bbox.lowLeft.x := min(x0, x1);
    bbox.lowLeft.y := min(y0, y1);
    bbox.upRight.x := max(x0, x1);
    bbox.upRight.y := max(y0, y1);
    DrawEraseMasksInBbox(root, root, boxlist, bbox);
    AddChangedBboxesInBbox(root, boxlist, bbox);
    DrawChangesInBbox(root, boxlist, bbox);
    DestroyBoxList(boxlist);
end RedrawInBoundingBox;


begin

end redraw.
