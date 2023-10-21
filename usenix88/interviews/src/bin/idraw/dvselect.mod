implementation module dvselect;

import
    rubband,
    picture;

(* export *)
from dvdefs import
    DrawingView;

from dvdefs import
    Box, GRIDSIZE, SELECTAREA;

(* export *)
from genlists import
    GenericList;

from genlists import
    Create, Iterator, BeginIteration, BeginReverseIteration,
    PrevElement, MoreElements, 
    EndIteration, Head, Tail, Append, Prepend, Insert, Delete, DeleteCur,
    Release;

from dverror import
    ReportErrorBanner, READFILEERR, WRITEFILEERR, PRINTERR;

from dvundo import
    SelectionsLogged, LogInsertion, LogDeletion;

from dvupict import
    DrawInClipBox, DrawPageInClipBox, RedrawInClipBox, ClipToClipBox,
    CalcPictClipBox, CalcCenterPictClipBox,
    CenterInClipBox, GetNearestGridPoint;


(* export *)
from picture import
    Picture, XCoord, YCoord;

from picture import
    PictureObjectType;

from utilities import
    MergeBoxes, IntersectBoxes, round, PointInBox, 
    RectInBox, PictInList;

from rubband import
    Rubberband, RubberbandType;

from vdi import
    Sync, AllWritable;

from math import
    ldexp;

from io import
    File, Open, Close;


type
    CountPtr = pointer to integer;



(* export *)
procedure CutSelections(
    const view : DrawingView;
    const filename : array of char
) : boolean;
    var i : Iterator;
        s : Picture;
	zoomFactor : real;
	testFile : File;
begin
    if Picture(Head(view^.stateInfo.selecList)) = Picture(nil) then
        return false;
    end;
    testFile := Open(filename, "w");
    if testFile = nil then
        ReportErrorBanner(view, WRITEFILEERR, filename);
	return false;
    end;
    Close(testFile);
    zoomFactor := ldexp(1.0, -view^.stateInfo.zoom);
    if zoomFactor # 1.0 then
        ScaleSelectionsAboutPoint(view, zoomFactor, zoomFactor, 0.0, 0.0);
    end;
    if not view^.stateInfo.selecSorted then
	picture.OrderSelections(view^.stateInfo.selecList, view^.pict);
    end;
    if picture.WriteSelections(view^.stateInfo.selecList, filename) then
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
    else
        ReportErrorBanner(view, WRITEFILEERR, filename);
	return false;
    end;
    DisableSelections(view);
    return true;
end CutSelections;


(* export *)
procedure CopySelections(
    const view : DrawingView;
    const filename : array of char
) : boolean;
    var pict, s : Picture;
        zoomFactor : real;
	ok : boolean;
begin
    ok := true;
    if Picture(Head(view^.stateInfo.selecList)) = Picture(nil) then
        return false;
    end;
    zoomFactor := ldexp(1.0, -view^.stateInfo.zoom);
    if zoomFactor # 1.0 then
        ScaleSelectionsAboutPoint(view, zoomFactor, zoomFactor, 0.0, 0.0);
    end;
    if not view^.stateInfo.selecSorted then
	picture.OrderSelections(view^.stateInfo.selecList, view^.pict);
	view^.stateInfo.selecSorted := true;
    end;
    if not picture.WriteSelections(view^.stateInfo.selecList, filename) then
        ReportErrorBanner(view, WRITEFILEERR, filename);
	ok := false;
    end;
    if zoomFactor # 1.0 then
        zoomFactor := 1.0 / zoomFactor;
        ScaleSelectionsAboutPoint(view, zoomFactor, zoomFactor, 0.0, 0.0);
    end;
    return ok;
end CopySelections;


(* export *)
procedure PasteSelections(
    const view : DrawingView;
    const filename : array of char
) : boolean;
    var pict, s : Picture;
        i : integer;
	zoomFactor, cx, cy : real;
begin
    if not picture.Read(pict, filename) then
        ReportErrorBanner(view, READFILEERR, filename);
        return false;
    end;
    ClipToClipBox(view);
    ReleaseSelections(view);
    AllWritable();
    CenterInClipBox(pict, view);
    for i := 1 to picture.NumberOfNestedPictures(pict) do
        s := picture.GetNestedPicture(pict, 1);
	picture.Unnest(s);
	Append(s, view^.stateInfo.selecList);
	picture.Nest(s, view^.pict);
    end;
    picture.Destroy(pict);
    zoomFactor := ldexp(1.0, view^.stateInfo.zoom);
    if zoomFactor # 1.0 then
        CalcCenterPictClipBox(view, cx, cy);
        ScaleSelectionsAboutPoint(view, zoomFactor, zoomFactor, cx, cy);
    end;
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	if view^.stateInfo.gridOn then
	    AlignSelectionsToGrid(view);
	end;
    end;
    view^.stateInfo.selecSorted := true;
    return true;
end PasteSelections;


(* export *)
procedure DisableSelections(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
	picture.Disable(s);
    end;
    EndIteration(i);
end DisableSelections;


(* export *)
procedure EnableSelections(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
	picture.Enable(s);
    end;
    EndIteration(i);
end EnableSelections;


procedure GetAncestor(
    const view : DrawingView;
    	  p : Picture
) : Picture;
    var test : Picture;
begin
    loop
        test:= picture.GetParent(p);
	if test = view^.pict then
	    exit;
	else
	    p := test;
	end;
    end;
    return p;
end GetAncestor;


procedure NotEnoughSelections(const list : GenericList) : boolean;
    var i : Iterator;
        result : boolean;
	p : Picture;
begin
    i := BeginIteration(list);
    result := MoreElements(i, p);
    result := MoreElements(i, p);
    EndIteration(i);
    return not result;
end NotEnoughSelections;


(* export *)
procedure CleanupSelectionsForGrouping(
    const view : DrawingView;
    var   parent : Picture
);
    var s : Picture;
        i : Iterator;
begin
(*
 *  First blow away Show Structure'd selections and put their top-level
 *  ancestor (child of view^.pict) in the selection list if it's not
 *  already there.
 *)
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        if picture.GetParent(s) # view^.pict then
	    view^.stateInfo.selecSorted := false;
	    DeleteCur(i);
	    s := GetAncestor(view, s);
	    if not AlreadySelected(s, view) then
	        Insert(s, i);
	    end;
	end;
    end;
    EndIteration(i);

    if NotEnoughSelections(view^.stateInfo.selecList) then
        picture.Destroy(parent);
	return;
    end;

    if not view^.stateInfo.selecSorted then
	picture.OrderSelections(view^.stateInfo.selecList, view^.pict);
	view^.stateInfo.selecSorted := true;
    end;
end CleanupSelectionsForGrouping;
    

(* export *)
procedure Group(
    const view : DrawingView;
    var   parent : Picture
);
    var s : Picture;
        i : Iterator;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        picture.Unnest(s);
        picture.Nest(s, parent);
    end;
    EndIteration(i);
    Create(view^.stateInfo.selecList);
    Append(parent, view^.stateInfo.selecList);
    picture.Nest(parent, view^.pict);
    DrawInClipBox(parent, view);
end Group;


(* export *)
procedure Ungroup(const view : DrawingView);
    var	parent, child : Picture;
	i : Iterator;
	n, j : integer;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, parent) do
	if not picture.ContainsPrimitives(parent) then
            for j := 1 to picture.NumberOfNestedPictures(parent) do
	        child := picture.GetNestedPicture(parent, 1);
	        picture.Unnest(child);
	        picture.Insert(child, parent);
		Delete(child, view^.stateInfo.selecList);
	        Insert(child, i);
	    end;
	    Delete(parent, view^.stateInfo.selecList);
	    picture.Unnest(parent);
	    picture.Destroy(parent);
	end;
    end;
    EndIteration(i);
end Ungroup;


(* export *)
procedure ShowStructure(const view : DrawingView);
    var	parent, child : Picture;
	i, k : Iterator;
	j, l, nvertices : integer;
	subpictList : GenericList;
	r : Rubberband;
	rtype : RubberbandType;
	ptype : picture.PictureObjectType;
	x : dynarray of XCoord;
	y : dynarray of YCoord;
	result : boolean;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, parent) do
	if not picture.ContainsPrimitives(parent) then
	    subpictList := picture.SelectAll(parent);
	    if subpictList # GenericList(nil) then
	        k := BeginIteration(subpictList);
	        while MoreElements(k, child) do
		    Delete(child, view^.stateInfo.selecList);
		    Insert(child, i);
		    AttachAHandleRect(view, child);
	        end;
	        Delete(parent, view^.stateInfo.selecList);
	        EndIteration(k);
	    else
	        AttachAHandleRect(view, parent);
	    end;
	else
	    nvertices := picture.GetNumberOfVertices(parent, 1);
	    if nvertices > 0 then
	        ptype := picture.GetPrimitiveType(parent, 1);
		if ptype = PolygonObj then
		    rubband.Create(r, PolyVertexHandles, 0);
		elsif ptype = BSplineObj then
		    rubband.Create(r, SplineVertexHandles, 0);
		else	(* closed spline *)
		    rubband.Create(r, ClosedSplineVertexHandles, 0);
		end;
		new(x, nvertices);
		new(y, nvertices);
		result := picture.GetVertices(
		    parent, 1, x^[0:nvertices], y^[0:nvertices]
		);
		for l := 0 to nvertices - 1 do
		    picture.ConvertToWorldCoord(parent, x^[l], y^[l]);
		end;
		rubband.InitVertexRubberband(r, x^[0:nvertices], y^[0:nvertices]);
		dispose(x);
		dispose(y);
		picture.SetUserData(parent, r);
	    else
	        AttachAHandleRect(view, parent);
	    end;
	end;
    end;
    EndIteration(i);
end ShowStructure;


(* export *)
procedure AlreadySelected(
    const s : Picture;
    const view : DrawingView
) : boolean;
    var i : Iterator;
        test : Picture;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, test) do
        if test = s then
	    EndIteration(i);
	    return true;
	end;
    end;
    EndIteration(i);
    return false;
end AlreadySelected;


(* export *)
procedure InASelection(
    const view : DrawingView;
    const x : XCoord;
    const y : YCoord;
    var   s : Picture
) : boolean;
    var i : Iterator;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
	slop : integer;
begin
    slop := SELECTAREA div 2;
    i := BeginReverseIteration(view^.stateInfo.selecList);
    while PrevElement(i, s) do
        picture.GetBoundingBox(s, x0, y0, x1, y1);
        if PointInBox(x, y, x0 - slop, y0 - slop, x1 + slop, y1 + slop) then
	    EndIteration(i);
	    return true;
	end;
    end;
    EndIteration(i);
    return false;
end InASelection;


(* export *)
procedure HighlightSelections(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        r := Rubberband(picture.GetUserData(s));
	rubband.DrawStatic(r);
    end;
    EndIteration(i);
    Sync();
end HighlightSelections;


(* export *)
procedure HighlightSelectionsInBbox(
    const view : DrawingView;
          left : XCoord;
          bottom : YCoord;
          right : XCoord;
          top : YCoord
);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
	box : Box;
begin
    picture.ConvertToWorldCoord(view^.pict, left, bottom);
    picture.ConvertToWorldCoord(view^.pict, right, top);
    box := view^.pictClipBox;
    picture.ConvertToWorldCoord(view^.world, box.x0, box.y0);
    picture.ConvertToWorldCoord(view^.world, box.x1, box.y1);
    if IntersectBoxes(
        box.x0, box.y0, box.x1, box.y1, left, bottom, right, top
    ) then
        i := BeginIteration(view^.stateInfo.selecList);
        while MoreElements(i, s) do
            r := Rubberband(picture.GetUserData(s));
	    rubband.DrawStaticInBoundingBox(r, box.x0, box.y0, box.x1, box.y1);
        end;
        EndIteration(i);
        Sync();
    end;
end HighlightSelectionsInBbox;


(* export *)
procedure AttachAHandleRect(
    const view : DrawingView;
    const p : Picture
);
    var r : Rubberband;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
begin
    picture.GetBoundingBox(p, x0, y0, x1, y1);
    picture.ConvertToWorldCoord(view^.pict, x0, y0);
    picture.ConvertToWorldCoord(view^.pict, x1, y1);
    rubband.Create(r, rubband.HandleRect, 0);
    rubband.Initialize(r, x0, y0, x1, y1, 0, 0);
    picture.SetUserData(p, r);
end AttachAHandleRect;


(* export *)
procedure CreateRubberbands(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
	AttachAHandleRect(view, s);
    end;
    EndIteration(i);
end CreateRubberbands;


(* export *)
procedure RecalcRubberbands(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
	rtype : RubberbandType;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
	x : dynarray of XCoord;
	y : dynarray of YCoord;
	nvertices, l : integer;
	result : boolean;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        r := Rubberband(picture.GetUserData(s));
	rtype := rubband.GetRubberbandType(r);
	rubband.Destroy(r);
	if 
	    (rtype = PolyVertexHandles) or
	    (rtype = SplineVertexHandles) or
	    (rtype = ClosedSplineVertexHandles)
	then
	    nvertices := picture.GetNumberOfVertices(s, 1);
	    rubband.Create(r, rtype, 0);
	    new(x, nvertices);
	    new(y, nvertices);
	    result := picture.GetVertices(s, 1, x^[0:nvertices], y^[0:nvertices]);
	    for l := 0 to nvertices - 1 do
	        picture.ConvertToWorldCoord(s, x^[l], y^[l]);
	    end;
	    rubband.InitVertexRubberband(r, x^[0:nvertices], y^[0:nvertices]);
	    dispose(x);
	    dispose(y);
	    picture.SetUserData(s, r);
	else
	    AttachAHandleRect(view, s);
	end;
    end;
    EndIteration(i);
end RecalcRubberbands;


(* export *)
procedure DeleteRubberbands(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        r := Rubberband(picture.GetUserData(s));
	rubband.EraseAndDestroy(r);
    end;
    EndIteration(i);
    Sync();
end DeleteRubberbands;


(* export *)
procedure ReleaseRubberbands(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        r := Rubberband(picture.GetUserData(s));
	rubband.Destroy(r);
    end;
    EndIteration(i);
end ReleaseRubberbands;


(* export *)
procedure CopySelectionList(const view : DrawingView) : GenericList;
    var i : Iterator;
        s : Picture;
	newList : GenericList;
begin
    Create(newList);
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
	Append(s, newList);
    end;
    EndIteration(i);
    return newList;
end CopySelectionList;


(* export *)
procedure ReleaseSelections(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
begin
    if SelectionsLogged(view) then
	DeleteRubberbands(view);
	Create(view^.stateInfo.selecList);
    else
	i := BeginIteration(view^.stateInfo.selecList);
	while MoreElements(i, s) do
	    r := Rubberband(picture.GetUserData(s));
	    rubband.EraseAndDestroy(r);
	    DeleteCur(i);
	end;
	EndIteration(i);
	Sync();
    end;
end ReleaseSelections;


(*
 *  Release selections without erasing handles.
 *)
(* export *)
procedure QuickReleaseSelections(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	r : Rubberband;
begin
    if SelectionsLogged(view) then
	ReleaseRubberbands(view);
	Create(view^.stateInfo.selecList);
    else
	i := BeginIteration(view^.stateInfo.selecList);
	while MoreElements(i, s) do
	    r := Rubberband(picture.GetUserData(s));
	    rubband.Destroy(r);
	    DeleteCur(i);
	end;
	EndIteration(i);
    end;
end QuickReleaseSelections;


(* export *)
procedure EmptySelectionList(const view : DrawingView);
    var i : Iterator;
        s : Picture;
begin
    if SelectionsLogged(view) then
	Create(view^.stateInfo.selecList);
    else
	i := BeginIteration(view^.stateInfo.selecList);
	while MoreElements(i, s) do
	    DeleteCur(i);
	end;
	EndIteration(i);
    end;
end EmptySelectionList;


(* export *)
procedure MergeSelectionBoxes(
    const view : DrawingView;
    var   bx0 : XCoord;
    var   by0 : YCoord;
    var   bx1 : XCoord;
    var   by1 : YCoord
);
    var x0, x1 : XCoord;
        y0, y1 : YCoord;
	i : Iterator;
	s : Picture;
begin
    bx0 := bx1;
    by0 := by1;
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
	picture.GetBoundingBox(s, x0, y0, x1, y1);
        MergeBoxes(bx0, by0, bx1, by1, x0 ,y0, x1, y1);
    end;
    EndIteration(i);
end MergeSelectionBoxes;


(* export *)
procedure SelectInRect(
    const view : DrawingView;
	  left : XCoord;
	  bottom : YCoord;
	  right : XCoord;
	  top : YCoord
);
    var pict, test : Picture;
        r : Rubberband;
	pictList : GenericList;
	i : Iterator;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
begin
    picture.ConvertToScreenCoord(view^.pict, left, bottom);
    picture.ConvertToScreenCoord(view^.pict, right, top);
    pictList := picture.SelectWithinBoundingBox(
        view^.pict, left, bottom, right, top
    );
(*
 *  First check if nested (Show Structured) objects that are already selected
 *  are being selected again.
 *)
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, pict) do
        picture.GetBoundingBox(pict, x0, y0, x1, y1);
	if RectInBox(x0, y0, x1, y1, left, bottom, right, top) then
	    if pictList = GenericList(nil) then
	        Create(pictList);
		Append(pict, pictList);
	    elsif not PictInList(pict, pictList) then
	        Append(pict, pictList);
	    end;
	end;
    end;
    EndIteration(i);
    if pictList # GenericList(nil) then
	i := BeginIteration(pictList);
        while MoreElements(i, pict) do
	    if AlreadySelected(pict, view) then
	        Delete(pict, view^.stateInfo.selecList);
	        r := Rubberband(picture.GetUserData(pict));
	        rubband.EraseAndDestroy(r);
	    else
		AttachAHandleRect(view, pict);
		r := Rubberband(picture.GetUserData(pict));
	        rubband.DrawStatic(r);
	        Append(pict, view^.stateInfo.selecList);
	    end;
	end;
	EndIteration(i);
	Sync();
    end;
end SelectInRect;


(* export *)
procedure SelectAll(const view : DrawingView);
begin
    ReleaseSelections(view);
    Release(view^.stateInfo.selecList);
    view^.stateInfo.selecList := picture.SelectAll(view^.pict);
    if view^.stateInfo.selecList = GenericList(nil) then
        Create(view^.stateInfo.selecList);
    else
	CreateRubberbands(view);
	HighlightSelections(view);
    end;
end SelectAll;


(* export *)
procedure TranslateSelections(
    const view : DrawingView;
    const dx, dy : real
);
    var i : Iterator;
        s : Picture;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        picture.Translate(s, dx, dy);
    end;
    EndIteration(i);
end TranslateSelections;


(* export *)
procedure ScaleSelections(
    const view : DrawingView;
    const sx, sy : real
);
    var i : Iterator;
        s : Picture;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        picture.Scale(s, sx, sy);
    end;
    EndIteration(i);
end ScaleSelections;


(* export *)
procedure RotateSelections(
    const view : DrawingView;
    const angle : real
);
    var i : Iterator;
        s : Picture;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        picture.Rotate(s, angle);
    end;
    EndIteration(i);
end RotateSelections;


(* export *)
procedure ScaleSelectionsAboutPoint(
    const view : DrawingView;
    const sx, sy, px, py : real
);
    var i : Iterator;
        s : Picture;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        picture.ScaleAboutPoint(s, sx, sy, px, py);
    end;
    EndIteration(i);
end ScaleSelectionsAboutPoint;


(*
 *  CalcStretchOrigin calculates the point about which an object should be
 *  scaled based on the given stretch code.
 *)
procedure CalcStretchOrigin(
    const x0 : XCoord;  const y0 : YCoord;	(* bounding box *)
    const x1 : XCoord;  const y1 : YCoord;
    const codex : XCoord; const codey : YCoord;
    var x : XCoord;	(* stretch origin *)
    var y : YCoord
);
begin
    if codex = 0 then
        x := x1;
    elsif codex = 2 then
        x := x0;
    else
        x := round(float(x0 + x1) / 2.0);
    end;
    if codey = 0 then
        y := y1;
    elsif codey = 2 then
        y := y0;
    else
        y := round(float(y0 + y1) / 2.0);
    end;
end CalcStretchOrigin;


(* export *)
procedure StretchSelections(
    const view : DrawingView;
    const sx, sy : real;
    const codex : XCoord;
    const codey : YCoord
);
    var i : Iterator;
        s : Picture;
	x, x0, x1 : XCoord;
	y, y0, y1 : YCoord;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
	picture.GetBoundingBox(s, x0, y0, x1, y1);
        CalcStretchOrigin(x0, y0, x1, y1, codex, codey, x, y);
        picture.ScaleAboutPoint(s, sx, sy, float(x), float(y));
    end;
    EndIteration(i);
end StretchSelections;


(* export *)
procedure RotateSelectionsAboutPoint(
    const view : DrawingView;
    const angle, px, py : real
);
    var i : Iterator;
        s : Picture;
begin
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        picture.RotateAboutPoint(s, angle, px, py);
    end;
    EndIteration(i);
end RotateSelectionsAboutPoint;


(* export *)
procedure SendSelectionsToBack(const view : DrawingView);
    var i : Iterator;
        s : Picture;
begin
    i := BeginReverseIteration(view^.stateInfo.selecList);
    while PrevElement(i, s) do
	picture.SendToBack(picture.GetParent(s), s);
    end;
    EndIteration(i);
    RedrawInClipBox(view^.pict, view);
    DrawPageInClipBox(view);
end SendSelectionsToBack;


(* export *)
procedure BringSelectionsToFront(const view : DrawingView);
    var i : Iterator;
        s, parent : Picture;
        x0 : XCoord;
	y0 : YCoord;
begin
    picture.GetOrigin(view^.pict, x0, y0);
    i := BeginIteration(view^.stateInfo.selecList);
    while MoreElements(i, s) do
        parent:= picture.GetParent(s);
	picture.Unnest(s);
	picture.SetOrigin(s, x0, y0);
	DrawInClipBox(s, view);
	picture.Nest(s, parent);
    end;
    EndIteration(i);
    DrawPageInClipBox(view);
end BringSelectionsToFront;


(* export *)
procedure AlignSelectionsToGrid(const view : DrawingView);
    var i : Iterator;
        s : Picture;
	x0, x1, dx, origx : XCoord;
	y0, y1, dy, origy : YCoord;
begin
    if Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	picture.GetOrigin(view^.pict, origx, origy);

        i := BeginIteration(view^.stateInfo.selecList);
        while MoreElements(i, s) do
            picture.GetBoundingBox(s, x0, y0, x1, y1);
	    GetNearestGridPoint(view, float(x0), float(y0), dx, dy);
	    dx := (dx - x0) mod GRIDSIZE;
	    dy := (dy - y0) mod GRIDSIZE;
            picture.Translate(s, float(dx), float(dy));
        end;
        EndIteration(i);
    end;
end AlignSelectionsToGrid;


begin
end dvselect.
