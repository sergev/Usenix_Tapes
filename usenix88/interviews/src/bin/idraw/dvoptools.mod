implementation module dvoptools;

import
    picture,
    rubband;

(* export *)
from dvdefs import
    DrawingView;

from dvdefs import
    Cursor, Pattern, CharSet, SELECTAREA, 
    StateType, Box, GRIDSIZE, LineType, MAXZOOMIN,
    LINEFEED, CARRIAGERTN, BACKSPACE, LEFTBUTTON, MIDDLEBUTTON, MAXPOLYPTS;

(* export *)
from input import
    InputButton;

from input import
    GetCursor, SetCursor, CursorOn, CursorOff, hourglassCursor, EventType;

from utilities import
    Distance, PointInBox, Deg, round, IntersectBoxes;
    
(* export *)
from picture import
    Picture, XCoord, YCoord;

from genlists import
    GenericList, Iterator, BeginIteration, BeginReverseIteration, 
    MoreElements, EndIteration, Head, Tail, Append,
    Delete, Release;

from dvboot import
    ARROWWIDTH, ARROWHEIGHT;

from dvcmds import
    Zoom;

from dvevents import
    HandleEvent, EventPkg, DisposeEventPackage, ChangeCursor;

from dvselect import
    Group, CreateRubberbands, AlreadySelected, InASelection, 
    ReleaseSelections, CopySelectionList,
    RecalcRubberbands, HighlightSelections, HighlightSelectionsInBbox,
    SelectInRect, MergeSelectionBoxes,
    TranslateSelections, ScaleSelections, ScaleSelectionsAboutPoint,
    StretchSelections, RotateSelections,
    SendSelectionsToBack, BringSelectionsToFront;

from pictmacros import
    AddLine, CreateSubpict;

from rubband import
    Rubberband, RubberbandType, HANDLESIZE;

from dvundo import
    SelectionsLogged, LogMove, LogMoveVertex, LogScale, LogStretch, LogRotate;

from dvupict import
    DrawInClipBox, RedrawInClipBox, DrawPageInClipBox, 
    ClearAndDrawInClipBox, ClipToClipBox, CalcPictClipBox, 
    CenterPointOnPage;

from dvwindops import
    CopyIndicatorInfo, MakeDirty;

from dvscroll import
    UpdateScrollBars;

from vdi import
    AllWritable, SetRendering, FilledRectangle, SetPattern, solidPattern, 
    Point, RenderingFunction;

(* export *)
from system import
    Word;

from math import
    sqrt, atan, log, ldexp;


const MINSCALE = 0.00001;

var
    dumX : XCoord;
    dumY : YCoord;


procedure LimitScale(var s : real);
    var minscale : real;
begin
    minscale := MINSCALE;   (* const reals are 32 bits, while reals are 64 *)
    if s < 0.0 then
	s := -max(-s, minscale);
    else
	s := max(s, minscale);
    end;
end LimitScale;


(* export *)
procedure SelectorHandler(const eventWord : Word);
    var eventPkg : EventPkg;
	r : Rubberband;
        s, test : Picture;
	x0, x1, screenX : XCoord;
	y0, y1, screenY : YCoord;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        screenX := x;
	screenY := y;
        picture.ConvertToScreenCoord(view^.pict, screenX, screenY);
        if
	    (view^.stateInfo.activity = Idling) and
	    (event = BUTTON) and (button = LEFTBUTTON) and not up and
	    (picture.SelectByPoint(view^.pict, screenX, screenY) 
	        = GenericList(nil))
	then
	    if SelectionsLogged(view) then
		view^.stateInfo.selecList := CopySelectionList(view);
	    end;
	    ClipToClipBox(view);
            rubband.Create(r, rubband.RubberRect, 0);
	    rubband.Initialize(r, x, y, 0, 0, x, y);
	    SetRendering(INVERT);
	    SetPattern(solidPattern);
	    FilledRectangle(
		x - HANDLESIZE, y - HANDLESIZE,
		x + HANDLESIZE, y + HANDLESIZE
	    );
            view^.stateInfo.rubberband := r;
	    view^.stateInfo.activity := Selecting;
        elsif
	    (view^.stateInfo.activity = Selecting) and
	    (event = BUTTON) and (button = LEFTBUTTON) and up 
	then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x0, y0, dumX, dumY, dumX, dumY);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    SetRendering(INVERT);
	    SetPattern(solidPattern);
	    FilledRectangle(
		x0 - HANDLESIZE, y0 - HANDLESIZE,
		x0 + HANDLESIZE, y0 + HANDLESIZE
	    );
	    SelectInRect(view, min(x, x0), min(y, y0), max(x, x0), max(y, y0));
	    AllWritable();
	    view^.stateInfo.selecSorted := false;
	    view^.stateInfo.activity := Idling;
        elsif
	    (view^.stateInfo.activity = Selecting) and (event = MOTION)
	then
            rubband.Draw(view^.stateInfo.rubberband, x, y);

        elsif 
	    (view^.stateInfo.activity # Selecting) and
	    GenericHandler(eventWord)
	then
(*
 *  If we get this far, we selected an object directly (instead of
 *  drawing a rubberband around a bunch and selecting them that way)
 *)
	    if SelectionsLogged(view) then
		view^.stateInfo.selecList := CopySelectionList(view);
	    end;
	    s := view^.stateInfo.selection;
	    ClipToClipBox(view);
	    if AlreadySelected(s, view) then
		Delete(s, view^.stateInfo.selecList);
		r := Rubberband(picture.GetUserData(s));
	        rubband.EraseAndDestroy(r);
	    else
	        picture.GetBoundingBox(s, x0, y0, x1, y1);
		picture.ConvertToWorldCoord(view^.pict, x0, y0);
		picture.ConvertToWorldCoord(view^.pict, x1, y1);
		rubband.Create(r, rubband.HandleRect, 0);
		rubband.Initialize(r, x0, y0, x1, y1, 0, 0);
		rubband.DrawStatic(r);
	        picture.SetUserData(s, r);
		Append(s, view^.stateInfo.selecList);
	    end;
	    AllWritable();
	    view^.stateInfo.selecSorted := false;
        end;
    end;
    DisposeEventPackage(eventPkg);
end SelectorHandler;


procedure OnAVertex(
    const v : DrawingView;
    const x : XCoord;
    const y : YCoord;
    var   p : Picture;
    var   nvertex : integer
) : boolean;
    var j : Iterator;
        r : Rubberband;
        rtype : RubberbandType;
	vx : dynarray of XCoord;
	vy : dynarray of YCoord;
	dx, dxmin : XCoord;
	dy, dymin : YCoord;
	result, dummy, initialized : boolean;
	arraySize, nvertices, i : integer;
	s : Picture;
begin
    result := false;
    initialized := false;
    arraySize := 0;
    j := BeginIteration(v^.stateInfo.selecList);
    while MoreElements(j, s) do
        r := Rubberband(picture.GetUserData(s));
        rtype := rubband.GetRubberbandType(r);
        if
	    (rtype = PolyVertexHandles) or
	    (rtype = SplineVertexHandles) or
	    (rtype = ClosedSplineVertexHandles)
        then
	    nvertices := picture.GetNumberOfVertices(s, 1);
	    if nvertices > arraySize then
		if initialized then
		    dispose(vx);
		    dispose(vy);
		end;
		arraySize := nvertices;
	        new(vx, arraySize);
	        new(vy, arraySize);
	    end;
	    dummy := picture.GetVertices(
	        s, 1, vx^[0:arraySize], vy^[0:arraySize]
	    );
	    for i := 0 to nvertices - 1 do
	        dx := abs(vx^[i] - x);
	        dy := abs(vy^[i] - y);
		if not initialized then
		    dxmin := dx + 1;
		    dymin := dy + 1;
		    initialized := true;
		end;
	        if
	            (dx <= SELECTAREA * 2) and (dy <= SELECTAREA * 2) and
		    (dx*dx + dy*dy < dxmin*dxmin + dymin*dymin)
	        then
	            result := true;
		    dxmin := dx;
		    dymin := dy;
		    nvertex := i;
		    p := s;
	        end;
	    end;
        end;
    end;
    EndIteration(j);
    if initialized then
        dispose(vx);
        dispose(vy);
    end;
    return result;
end OnAVertex;


(* export *)
procedure MoveHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	rtype : RubberbandType;
	x0, x1, origx0, dx : XCoord;
	y0, y1, origy0, dy : YCoord;
	sel : GenericList;
	s, test : Picture;
	onAVertex, result : boolean;
	nvertex : integer;
	b : Box;
	fdx, fdy : real;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
            picture.ConvertToScreenCoord(view^.pict, x, y);
	    sel := picture.SelectByBoundingBox(
	        view^.pict,
	        x - SELECTAREA div 2, y - SELECTAREA div 2,
	        x + SELECTAREA div 2, y + SELECTAREA div 2
	    );
	    if sel # GenericList(nil) then
	        s := Picture(Head(sel));
	        Release(sel);
	        if s # Picture(nil) then
		    ClipToClipBox(view);
		    onAVertex := false;
		    if not InASelection(view, x, y, test) then
			ReleaseSelections(view);
			Append(s, view^.stateInfo.selecList);
			CreateRubberbands(view);
			HighlightSelections(view);
		    else
		        onAVertex := OnAVertex(view, x, y, test, nvertex);
		    end;
		    if onAVertex then
		        r := Rubberband(picture.GetUserData(test));
			rubband.SetRubberVertex(r, nvertex);
			view^.stateInfo.selection := test;
		    else
			MergeSelectionBoxes(view, x0, y0, x1, y1);
			picture.ConvertToWorldCoord(view^.pict, x0, y0);
			picture.ConvertToWorldCoord(view^.pict, x1, y1);
                	picture.ConvertToWorldCoord(view^.pict, x, y);
			rubband.Create(r, rubband.SlidingRect, 0);
	        	rubband.Initialize(r, x0, y0, x1, y1, x, y);
		    end;
                    view^.stateInfo.rubberband := r;
		    view^.stateInfo.activity := Rubberbanding;
		end;
	    end;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
	    rtype := rubband.GetRubberbandType(r);
	    if
		(rtype = PolyVertexHandles) or
		(rtype = SplineVertexHandles) or
		(rtype = ClosedSplineVertexHandles)
	    then
		rubband.Erase(r);
		result := rubband.GetRubberVertex(r, nvertex);
		rubband.GetOrigin(r, dumX, dumY, dumX, dumY, x0, y0);
		dx := x - x0;
		dy := y - y0;
		picture.MoveVertex(view^.stateInfo.selection, 1, nvertex, dx, dy);
		if
		    picture.GetPrimitiveType(view^.stateInfo.selection, 2)
		    # picture.None
		then
		    picture.MoveVertex(
		        view^.stateInfo.selection, 2, nvertex, dx, dy
		    );
		end;
		LogMoveVertex(view, view^.stateInfo.selection, nvertex, dx, dy);
		if rubband.GetAffectedArea(r, x0, y0, x1, y1) then
		    picture.ConvertToScreenCoord(view^.pict, x0, y0);
		    picture.ConvertToScreenCoord(view^.pict, x1, y1);
		    rubband.ResetRubberVertex(r);
		    b := CalcPictClipBox(view^.pict, view);
		    AllWritable();
		    if IntersectBoxes(x0, y0, x1, y1, b.x0, b.y0, b.x1, b.y1) then
		        HighlightSelectionsInBbox(view, x0, y0, x1, y1);
		        RecalcRubberbands(view);
		        picture.RedrawInBoundingBox(view^.pict, x0, y0, x1, y1);
			DrawPageInClipBox(view);
		        HighlightSelectionsInBbox(view, x0, y0, x1, y1);
			MakeDirty(view);
		    end;			
		else
		    rubband.ResetRubberVertex(r);
		end;
		    
	    else
                rubband.GetOrigin(r, x0, y0, x1, y1, dumX, dumY);
	        MergeSelectionBoxes(view, origx0, origy0, dumX, dumY);
	        picture.ConvertToWorldCoord(view^.pict, origx0, origy0);
		fdx := float(x0 - origx0);
		fdy := float(y0 - origy0);
	        TranslateSelections(view, fdx, fdy);
	        rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	        HighlightSelections(view);
	        RecalcRubberbands(view);
                AllWritable();
	        RedrawInClipBox(view^.pict, view);
	        DrawPageInClipBox(view);
	        ClipToClipBox(view);
	        HighlightSelections(view);
	        AllWritable();
	        MakeDirty(view);
		LogMove(view, fdx, fdy);
	    end;
	    view^.stateInfo.activity := Idling;

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end MoveHandler;


(* export *)
procedure ScaleHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	x0, x1, x2, x3 : XCoord;
	y0, y1, y2, y3 : YCoord;
	sxy : real;
	sel : GenericList;
	test : Picture;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
            picture.ConvertToScreenCoord(view^.pict, x, y);
	    sel := picture.SelectByBoundingBox(
	        view^.pict,
	        x - SELECTAREA div 2, y - SELECTAREA div 2,
	        x + SELECTAREA div 2, y + SELECTAREA div 2
	    );
	    if sel # GenericList(nil) then
	        view^.stateInfo.selection := Picture(Head(sel));
	        Release(sel);
	        if view^.stateInfo.selection # Picture(nil) then
		    ClipToClipBox(view);
		    if not InASelection(view, x, y, test) then
			ReleaseSelections(view);
			Append(view^.stateInfo.selection, view^.stateInfo.selecList);
			CreateRubberbands(view);
			HighlightSelections(view);
		    else
		        view^.stateInfo.selection := test;
		    end;
		    picture.GetBoundingBox(view^.stateInfo.selection, x0, y0, x1, y1);
		    picture.ConvertToWorldCoord(view^.pict, x0, y0);
		    picture.ConvertToWorldCoord(view^.pict, x1, y1);
                    picture.ConvertToWorldCoord(view^.pict, x, y);
                    rubband.Create(r, rubband.ScalingRect, 0);
	            rubband.Initialize(r, x0, y0, x1, y1, x0, y0);
                    view^.stateInfo.rubberband := r;
	            view^.stateInfo.activity := Rubberbanding;
		end;
	    end;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x2, y2, x3, y3, dumX, dumY);
	    picture.GetBoundingBox(view^.stateInfo.selection, x0, y0, x1, y1);
	    picture.ConvertToWorldCoord(view^.pict, x0, y0);
	    picture.ConvertToWorldCoord(view^.pict, x1, y1);
	    if x0 # x1 then
	        sxy := float(x3 - x2) / float(x1 - x0);
	    elsif y0 # y1 then
	        sxy := float(y3 - y2) / float(y1 - y0);
	    else
	        sxy := 1.0;
	    end;
	    LimitScale(sxy);
	    ScaleSelections(view, sxy, sxy);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    HighlightSelections(view);
	    RecalcRubberbands(view);
            AllWritable();
	    RedrawInClipBox(view^.pict, view);
	    DrawPageInClipBox(view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
	    MakeDirty(view);
	    view^.stateInfo.activity := Idling;
	    LogScale(view, sxy, sxy);

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end ScaleHandler;


(*
 *  CalcStretchCode returns 2 values representing the 9 possible ways that an
 *  object can be stretched.  (0,0) means the lower left corner is stretched,
 *  and (2,2) means the upper right corner.
 *)
procedure CalcStretchCode(
    const x0 : XCoord;  const y0 : YCoord;	(* bounding box *)
    const x1 : XCoord;  const y1 : YCoord;
    const x : XCoord;  const y : YCoord;	(* stretch point *)
    var codex : XCoord;
    var codey : YCoord
);
    var xbound, ybound, cx, cy : real;
begin
    xbound := float(x1 - x0) / 6.0;
    ybound := float(y1 - y0) / 6.0;
    cx := float(x0 + x1) / 2.0;
    cy := float(y0 + y1) / 2.0;

    if 6.0 * xbound <= 1.0 then
        codex := 1;
    elsif float(x) >= cx - xbound then	(* not left *)
        if float(x) <= cx + xbound then	(* middle *)
	    codex := 1;
	else	(* right *)
	    codex := 2;
	end;
    else	(* left *)
        codex := 0;
    end;

    if 6.0 * ybound <= 1.0 then
        codey := 1;
    elsif float(y) >= cy - ybound then	(* not bottom *)
        if float(y) <= cy + ybound then	(* middle *)
	    codey := 1;
	else	(* top *)
    	    codey := 2;
	end;
    else	(* bottom *)
        codey := 0;
    end;
end CalcStretchCode;


procedure InitStretchRubband(
    var r : Rubberband;
    const x0 : XCoord; const y0 : YCoord;
    const x1 : XCoord; const y1 : YCoord;
    const x : XCoord; const y : YCoord
);
    var rtype : RubberbandType;
	codex, origx : XCoord;
	codey, origy : YCoord;
begin
    rtype := RubberRect;
    CalcStretchCode(x0, y0, x1, y1, x, y, codex, codey);

    if codex = 0 then	(* left *)
        origx := x1;
    elsif codex = 1 then	(* middle *)
        rtype := StretchingRect;
	origx := round(float(x0 + x1) / 2.0);
    else	(* right *)
        origx := x0;
    end;

    if codey = 0 then	(* bottom *)
        origy := y1;
    elsif (codey = 1) and (rtype = StretchingRect) then	(* middle *)
        r := Rubberband(nil);	(* stretching from middle has no effect *)
	return;
    elsif codey = 1 then
	rtype := StretchingRect;
	origy := round(float(y0 + y1) / 2.0);
    else	(* top *)
        origy := y0;
    end;
    
    rubband.Create(r, rtype, 0);
    if rtype = StretchingRect then
        rubband.Initialize(r, x0, y0, x1, y1, origx, origy);
    else
        rubband.Initialize(r, origx, origy, 0, 0, origx, origy);
    end;
end InitStretchRubband;


procedure CalcStretchScaling(
    const r : Rubberband;
    const x0 : XCoord; const y0 : YCoord;
    const x1 : XCoord; const y1 : YCoord;
    const x : XCoord; const y : YCoord;
    var codex : XCoord;
    var codey : YCoord;
    var sx, sy : real
);
    var oldwidth, oldheight, newwidth, newheight : integer;
        dumx, origx : XCoord;
	dumy, origy : YCoord;
begin
    rubband.GetOrigin(r, dumx, dumy, dumx, dumy, origx, origy);
    CalcStretchCode(x0, y0, x1, y1, origx, origy, codex, codey);
    codex := 2 - codex;	(* need to "invert" code since we need stretch 	*)
    codey := 2 - codey;	(* POINT, not stretch ORIGIN			*)
    oldwidth := x1 - x0;
    oldheight := y1 - y0;
    sx := 1.0;
    sy := 1.0;

    if codex # 1 then
        if codex = 0 then
	    newwidth := origx - x;
	else
	    newwidth := x - origx;
	end;
	if oldwidth # 0 then
	    sx := float(newwidth) / float(oldwidth);
	end;
    end;
    if codey # 1 then
        if codey = 0 then
	    newheight := origy - y;
	else
	    newheight := y - origy;
	end;
	if oldheight # 0 then
	    sy := float(newheight) / float(oldheight);
	end;	
    end;
end CalcStretchScaling;


(* export *)
procedure StretchHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	x0, x1, x2, x3, codex : XCoord;
	y0, y1, y2, y3, codey : YCoord;
	sx, sy : real;
	sel : GenericList;
	test : Picture;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
            picture.ConvertToScreenCoord(view^.pict, x, y);
	    sel := picture.SelectByBoundingBox(
	        view^.pict,
	        x - SELECTAREA div 2, y - SELECTAREA div 2,
	        x + SELECTAREA div 2, y + SELECTAREA div 2
	    );
	    if sel # GenericList(nil) then
	        view^.stateInfo.selection := Picture(Head(sel));
	        Release(sel);
	        if view^.stateInfo.selection # Picture(nil) then
		    ClipToClipBox(view);
		    if not InASelection(view, x, y, test) then
			ReleaseSelections(view);
			Append(view^.stateInfo.selection, view^.stateInfo.selecList);
			CreateRubberbands(view);
			HighlightSelections(view);
		    else
		        view^.stateInfo.selection := test;
		    end;
		    picture.GetBoundingBox(view^.stateInfo.selection, x0, y0, x1, y1);
		    picture.ConvertToWorldCoord(view^.pict, x0, y0);
		    picture.ConvertToWorldCoord(view^.pict, x1, y1);
                    picture.ConvertToWorldCoord(view^.pict, x, y);
	            InitStretchRubband(r, x0, y0, x1, y1, x, y);
		    if r # Rubberband(nil) then
			view^.stateInfo.rubberband := r;
			view^.stateInfo.activity := Rubberbanding;
		    else
		        AllWritable();
		    end;
		end;
	    end;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
	    picture.GetBoundingBox(view^.stateInfo.selection, x0, y0, x1, y1);
	    picture.ConvertToWorldCoord(view^.pict, x0, y0);
	    picture.ConvertToWorldCoord(view^.pict, x1, y1);
	    CalcStretchScaling(r, x0, y0, x1, y1, x, y, codex, codey, sx, sy);
	    LimitScale(sx);
	    LimitScale(sy);
	    StretchSelections(view, sx, sy, codex, codey);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    HighlightSelections(view);
	    RecalcRubberbands(view);
            AllWritable();
	    RedrawInClipBox(view^.pict, view);
	    DrawPageInClipBox(view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
	    MakeDirty(view);
	    view^.stateInfo.activity := Idling;
	    LogStretch(view, sx, sy, codex, codey);

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end StretchHandler;


(* export *)
procedure RotateHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	xc, yc, x1r, y1r, x2r, y2r, num, denom, angle : real;
	x0, x1, x2, x3, origx0 : XCoord;
	y0, y1, y2, y3, origy0 : YCoord;
	sel : GenericList;
	test : Picture;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
            picture.ConvertToScreenCoord(view^.pict, x, y);
	    sel := picture.SelectByBoundingBox(
	        view^.pict,
	        x - SELECTAREA div 2, y - SELECTAREA div 2,
	        x + SELECTAREA div 2, y + SELECTAREA div 2
	    );
	    if sel # GenericList(nil) then
	        view^.stateInfo.selection := Picture(Head(sel));
	        Release(sel);
	        if view^.stateInfo.selection # Picture(nil) then
		    ClipToClipBox(view);
		    if not InASelection(view, x, y, test) then
			ReleaseSelections(view);
			Append(view^.stateInfo.selection, view^.stateInfo.selecList);
			CreateRubberbands(view);
			HighlightSelections(view);
		    else
		        view^.stateInfo.selection := test;
		    end;
		    picture.GetBoundingBox(view^.stateInfo.selection, x0, y0, x1, y1);
		    picture.ConvertToWorldCoord(view^.pict, x0, y0);
		    picture.ConvertToWorldCoord(view^.pict, x1, y1);
                    picture.ConvertToWorldCoord(view^.pict, x, y);
                    rubband.Create(r, rubband.RotatingRect, 0);
	            rubband.Initialize(r, x0, y0, x1, y1, x, y);
                    view^.stateInfo.rubberband := r;
	            view^.stateInfo.activity := Rubberbanding;
		end;
	    end;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x0, y0, x1, y1, origx0, origy0);
	    if (origx0 # x) or (origy0 # y) then
	        picture.GetBoundingBox(view^.stateInfo.selection, x2, y2, x3, y3);
		xc := float(x2 + x3) / 2.0;
		yc := float(y2 + y3) / 2.0;
		picture.GetOrigin(view^.pict, x0, y0);
		xc := xc + float(x0);
		yc := yc + float(y0);
	        x1r := float(origx0) - xc;
	        y1r := float(origy0) - yc;
	        x2r := float(x) - xc;
	        y2r := float(y) - yc;
	        num := x1r*y2r - x2r*y1r;
		denom := x1r*x2r + y1r*y2r;
	        if denom # 0.0 then
		    angle := Deg(atan(num / denom));
		elsif num < 0.0 then
		    angle := -90.0;
		else
		    angle := 90.0;
		end;
		if denom < 0.0 then
		    angle := angle + 180.0
		end;
		RotateSelections(view, angle);
		LogRotate(view, angle);
	    end;
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    HighlightSelections(view);
	    RecalcRubberbands(view);
            AllWritable();
	    RedrawInClipBox(view^.pict, view);
	    DrawPageInClipBox(view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
	    MakeDirty(view);
	    view^.stateInfo.activity := Idling;

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end RotateHandler;


(* export *)
procedure MagnifyHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	x0 : XCoord;
	y0 : YCoord;
	rend : picture.RenderingStyle;
	origCursor, power2 : integer;
	enlargeFactor : real;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
	    ClipToClipBox(view);
            rubband.Create(r, rubband.RubberRect, 0);
	    rubband.Initialize(r, x, y, 0, 0, x, y);
            view^.stateInfo.rubberband := r;
	    view^.stateInfo.activity := Rubberbanding;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x0, y0, dumX, dumY, dumX, dumY);
	    if view^.stateInfo.zoom < MAXZOOMIN then
        	origCursor := GetCursor();
	        SetCursor(hourglassCursor);
		power2 := 0;
		if (x = x0) or (y = y0) then
		    power2 := MAXZOOMIN - view^.stateInfo.zoom;
		else
		    enlargeFactor := min(
		        float(abs(view^.pictClipBox.x1 - view^.pictClipBox.x0)) / 
		        float(abs(x - x0)),
		        float(abs(view^.pictClipBox.y1 - view^.pictClipBox.y0)) / 
		        float(abs(y - y0))
		    );
		    if enlargeFactor > 1.0 then
		        power2 := trunc(log(enlargeFactor) / log(2.0));
		        power2 := min(MAXZOOMIN - view^.stateInfo.zoom, power2);
		    end;
		end;
		if power2 > 0 then
		    x0 := (x + x0) div 2;
		    y0 := (y + y0) div 2;
		    picture.ConvertToScreenCoord(view^.page, x0, y0);
		    CenterPointOnPage(view, float(x0), float(y0));
		    rubband.Destroy(view^.stateInfo.rubberband);
		    AllWritable();
		    Zoom(view, ldexp(1.0, power2));
		    view^.stateInfo.zoom := view^.stateInfo.zoom + power2;
		end;
	        SetCursor(origCursor);
	    end;
	    if view^.stateInfo.rubberband # Rubberband(nil) then
	        rubband.EraseAndDestroy(view^.stateInfo.rubberband);
		AllWritable();
	    end;
	    view^.stateInfo.activity := Idling;

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end MagnifyHandler;


procedure GenericHandler(const eventWord : Word) : boolean;
    var eventPkg : EventPkg;
        x0, x1 : XCoord;
	y0, y1 : YCoord;
	r : Rubberband;
	sel : GenericList;
	s, p : Picture;
	somethingSelected, inASelection : boolean;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        picture.ConvertToScreenCoord(view^.pict, x, y);
        if view^.stateInfo.activity # Rubberbanding then
	    inASelection := InASelection(view, x, y, s);
	    if not inASelection then
		sel := picture.SelectByPoint(view^.pict, x, y);
		if sel # GenericList(nil) then
		    s := Picture(Head(sel));
		    Release(sel);
		    inASelection := true;
		end;
	    end;
	    if inASelection then
		ClipToClipBox(view);
	        picture.GetBoundingBox(s, x0, y0, x1, y1);
		picture.ConvertToWorldCoord(view^.pict, x0, y0);
		picture.ConvertToWorldCoord(view^.pict, x1, y1);
		rubband.Create(r, rubband.SlidingRect, 0);
		rubband.Initialize(r, x0, y0, x1, y1, 0, 0);
		rubband.DrawStatic(r);
		view^.stateInfo.rubberband := r;
	        view^.stateInfo.selection := s;
	        view^.stateInfo.activity := Rubberbanding;
	    end;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    view^.stateInfo.activity := Idling;
	    if view^.stateInfo.rubberband = Rubberband(nil) then
		AllWritable();
	    else
	        rubband.EraseAndDestroy(view^.stateInfo.rubberband);
		AllWritable();
		picture.GetBoundingBox(view^.stateInfo.selection, x0, y0, x1, y1);
		return PointInBox(x, y, x0, y0, x1, y1) 
	    end;

        elsif event = MOTION then
	    s := view^.stateInfo.selection;
	    inASelection := InASelection(view, x, y, p);
	    if not inASelection then
	        sel := picture.SelectByPoint(view^.pict, x, y);
	        if sel = GenericList(nil) then
	            somethingSelected := false;
		    p := Picture(nil);
	        else
		    p := Picture(Head(sel));
		    Release(sel);
		end;
	    end;
	    if inASelection or (p # Picture(nil)) then
		if view^.stateInfo.rubberband = Rubberband(nil) then
		    somethingSelected := true;
		else
		    somethingSelected := (s # p);
		end;
	    end;
	    if
	        (view^.stateInfo.rubberband # Rubberband(nil)) and
		(somethingSelected or (p = Picture(nil)))
	    then
		rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    end;
	    if somethingSelected then
		picture.GetBoundingBox(p, x0, y0, x1, y1);
	        picture.ConvertToWorldCoord(view^.pict, x0, y0);
	        picture.ConvertToWorldCoord(view^.pict, x1, y1);
		rubband.Create(r, rubband.SlidingRect, 0);
		rubband.Initialize(r, x0, y0, x1, y1, 0, 0);
		rubband.DrawStatic(r);
		view^.stateInfo.rubberband := r;
	        view^.stateInfo.selection := p;
	        view^.stateInfo.activity := Rubberbanding;
	    end;
	end;
    end;
    return false;
end GenericHandler;


begin
end dvoptools.
