implementation module dvobjtools;

import
    picture,
    menu,
    rubband;

(* export *)
from dvdefs import
    DrawingView;

from dvdefs import
    Cursor, Pattern, CharSet, CharPtr, SELECTAREA, 
    StateType, Box, GRIDSIZE, LineType, VertexObjType, NULL,
    LINEFEED, CARRIAGERTN, BACKSPACE, DELETE, 
    LEFTBUTTON, MIDDLEBUTTON, MAXPOLYPTS;

from dvselect import
    CreateRubberbands, HighlightSelections, ReleaseSelections;

(* export *)
from input import
    InputButton;

from input import
    hourglassCursor, GetCursor, SetCursor, CursorOn, CursorOff, EventType;

from utilities import
    Distance, PointInBox, Deg;
    
(* export *)
from picture import
    Picture, XCoord, YCoord;

from picture import
    Transparent;

from genlists import
    GenericList, Iterator, BeginIteration, MoreElements, EndIteration, 
    Head, Tail, Append, Delete, DeleteCur, Release;

from dvboot import
    ARROWWIDTH, ARROWHEIGHT;

from dvevents import
    HandleEvent, EventPkg, DisposeEventPackage, ChangeCursor;

from pictmacros import
    AddLine, AddVertexObj, CreateSubpict;

from rubband import
    Rubberband;

from dvundo import
    LogInsertion;

from dvupict import
    DrawInClipBox, RedrawInClipBox, DrawPageInClipBox, 
    ClearAndDrawInClipBox, ClipToClipBox, CalcPictClipBox;

from dvwindops import
    CopyIndicatorInfo, MakeDirty;

from vdi import
    AllWritable, SetRendering, Point, Line, RenderingFunction, 
    clearPattern, solidPattern, WHITE, BLACK;

(* export *)
from system import
    Word;


var
    dumX : XCoord;
    dumY : YCoord;


procedure UpdateText(const eventPkg : EventPkg);
    var ch : array [1..1] of char;
        chptr : CharPtr;
        x, x0, x1 : XCoord;
	y, y0, y1 : YCoord;
	charpict : Picture;
	r : Rubberband;
	s : Picture;
	pat : integer;
begin
    ch[1] := eventPkg^.c;
    CursorOff();
    with eventPkg^.view^.stateInfo do
	s := Picture(Tail(selecList));
        if ch[1] in CharSet{BACKSPACE, DELETE} then
	    picture.GetPosition(s, x, y);
	    charpict := picture.GetNestedPicture(s, -1);
	    if charpict = Picture(nil) then	(* at beginning of text *)
	        return;
	    else
	        chptr := CharPtr(Tail(eventPkg^.view^.stateInfo.charList));
		Delete(chptr, eventPkg^.view^.stateInfo.charList);
		eventPkg^.view^.stateInfo.charCount := 
		    eventPkg^.view^.stateInfo.charCount - 1;
		picture.GetBoundingBox(s, x0, y0, x1, y1);
		picture.GetOrigin(s, x1, y1);
		picture.Unnest(charpict);
		picture.SetOrigin(charpict, x1, y1);
		picture.GetBoundingBox(charpict, x, y, x1, y1);
		picture.AlterColor(
		    charpict, picture.GetBackgroundColor(charpict)
		);
		picture.AlterPattern(charpict, solidPattern);
		picture.AlterPaintType(charpict, picture.Opaque);
		picture.Draw(charpict);
		picture.Destroy(charpict);
		if x # x0 then
		    charpict := picture.GetNestedPicture(s, -1);
		    picture.GetBoundingBox(charpict, x0, y, x, y1);
		end;
	    end;
	else
	    new(chptr);
	    chptr^ := ch[1];
	    Append(chptr, eventPkg^.view^.stateInfo.charList);
	    eventPkg^.view^.stateInfo.charCount := 
	        eventPkg^.view^.stateInfo.charCount + 1;
	    picture.GetPosition(s, x, y);
	    picture.Create(charpict);
	    picture.DefaultFont(
	        charpict, picture.GetFont(eventPkg^.view^.pict)
	    );
	    pat := picture.GetPattern(eventPkg^.view^.pict);
	    picture.DefaultPattern(charpict, pat);
	    if pat = clearPattern then
		picture.DefaultBackgroundColor(charpict, BLACK);
	    end;
	    picture.DefaultRenderingStyle(
		charpict, picture.GetRenderingStyle(eventPkg^.view^.pict)
	    );
	    picture.DefaultPaintType(charpict, Transparent);

	    picture.MoveTo(charpict, x, y);
            picture.Text(charpict, ch);
	    picture.GetPosition(charpict, x, y);
	    picture.Nest(charpict, s);
	end;

	if ch[1] in CharSet{LINEFEED, CARRIAGERTN} then
	    picture.Disable(charpict);
	    picture.GetBoundingBox(s, x0, y0, x1, y1);
	    picture.GetPosition(s, x, y);
	    if x0 # x1 then
		x := x0;
	    end;
	    y := y - 
	        picture.GetFontHeight(picture.GetFont(eventPkg^.view^.pict));
	end;
        picture.MoveTo(s, x, y);
	picture.ConvertToWorldCoord(s, x, y);
	r := eventPkg^.view^.stateInfo.rubberband;
	if 
	    not (
		ch[1] in 
		CharSet{LINEFEED, CARRIAGERTN, BACKSPACE, DELETE}
	    )
	then
	    rubband.Erase(r);
	    picture.Draw(charpict);
	end;
	rubband.Draw(r, x, y);
    end;
end UpdateText;


(* export *)
procedure TextHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	s : Picture;
	cursor : Cursor;
	origCursor : integer;
	originX, x0, x1 : XCoord;
	originY, y0, y1 : YCoord;
	i : Iterator;
	k : integer;
	chptr : CharPtr;
	chararray : dynarray of char;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # TextEntering then
	    ClipToClipBox(view);
	    ReleaseSelections(view);
    	    rubband.Create(r, rubband.SlidingLine, 0);
	    rubband.Initialize(
	        r, x, y,
		x, y + picture.GetFontHeight(picture.GetFont(view^.pict)),
		x, y
	    );
	    rubband.DrawStatic(r);
	    picture.Create(view^.stateInfo.textPict);
	    picture.GetOrigin(view^.pict, originX, originY);
	    picture.SetOrigin(view^.stateInfo.textPict, originX, originY);

	    CopyIndicatorInfo(view^.pict, view^.stateInfo.textPict);
	    picture.DefaultPaintType(view^.stateInfo.textPict, Transparent);

	    picture.ConvertToScreenCoord(view^.pict, x, y);
	    picture.MoveTo(view^.stateInfo.textPict, x, y);
	    s := picture.Copy(view^.stateInfo.textPict);
            view^.stateInfo.rubberband := r;
	    Append(view^.stateInfo.textPict, view^.stateInfo.selecList);
	    Append(s, view^.stateInfo.selecList);
	    view^.stateInfo.charCount := 0;
	    view^.stateInfo.activity := TextEntering;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and not up then
	    origCursor := GetCursor();
	    SetCursor(hourglassCursor);
	    r := view^.stateInfo.rubberband;
	    s := Picture(Tail(view^.stateInfo.selecList));
	    Delete(s, view^.stateInfo.selecList);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    picture.Destroy(s);
	    view^.stateInfo.activity := Idling;
	    AllWritable();
(*
 *  Replace the picture containing the complex text representation (necessary
 *  for implementing editing painlessly) with a simpler one now that editing
 *  is complete.
 *)
	    if view^.stateInfo.charCount > 0 then
	        new(chararray, view^.stateInfo.charCount + 1);
		k := 0;
	    else
	        Delete(view^.stateInfo.textPict, view^.stateInfo.selecList);
	        picture.Destroy(view^.stateInfo.textPict);
	    end;
	    i := BeginIteration(view^.stateInfo.charList);
	    while MoreElements(i, chptr) do
	        chararray^[k] := chptr^;
		DeleteCur(i);
		if chptr^ in CharSet{LINEFEED, CARRIAGERTN} then
		    chararray^[k] := NULL;
		    picture.Text(view^.stateInfo.textPict, chararray^[0:k+1]);
		elsif CharPtr(Head(view^.stateInfo.charList)) = CharPtr(nil) then
		    chararray^[k + 1] := NULL;
		    picture.Text(view^.stateInfo.textPict, chararray^[0:k+2]);
		end;
		if chptr^ in CharSet{LINEFEED, CARRIAGERTN} then
		    picture.GetBoundingBox(view^.stateInfo.textPict, x0, y0, x1, y1);
		    picture.MoveTo(
		        view^.stateInfo.textPict, x0, y0 -
			picture.GetFontHeight(picture.GetFont(view^.pict))
		    );
		    k := 0;
		else
		    k := k + 1;
		end;
	    end;
	    EndIteration(i);
	    if view^.stateInfo.charCount > 0 then
	        picture.Nest(view^.stateInfo.textPict, view^.pict);
		RedrawInClipBox(view^.stateInfo.textPict, view);
		dispose(chararray);
		MakeDirty(view);
                DrawPageInClipBox(view);
	        CreateRubberbands(view);
	        ClipToClipBox(view);
	        HighlightSelections(view);
	        AllWritable();
		LogInsertion(view);
	    else
                DrawPageInClipBox(view);
	    end;
	    view^.stateInfo.textPict := Picture(nil);
	    SetCursor(origCursor);
	    HandleEvent(view, button, up, x, y, c, event);

        elsif event = CHARACTER then
            UpdateText(eventPkg);

        elsif event = MOTION then
	    cursor := Cursor(menu.GetUserData(view^.stateInfo.curTool));
	    CursorOn();
	    picture.ConvertToScreenCoord(view^.world, x, y);
            if 
		PointInBox(
	            x, y, 
	            view^.pictClipBox.x0, view^.pictClipBox.y0,
	            view^.pictClipBox.x1, view^.pictClipBox.y1
	        )
	    then
	        ChangeCursor(cursor^);
	    else
	    	ChangeCursor(view^.defaultCursor);
	    end;
	end;
    end;
    DisposeEventPackage(eventPkg);
end TextHandler;


(* export *)
procedure PerpLineHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	x0 : XCoord;
	y0 : YCoord;
	subpict : Picture;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
   	    ClipToClipBox(view);
	    ReleaseSelections(view);
            rubband.Create(r, rubband.RubberPerpLine, 0);
	    rubband.Initialize(r, x, y, 0, 0, x, y);
            view^.stateInfo.rubberband := r;
	    view^.stateInfo.activity := Rubberbanding;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x0, y0, dumX, dumY, dumX, dumY);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    AllWritable();
	    subpict := CreateSubpict(view^.pict);
	    Append(subpict, view^.stateInfo.selecList);
            picture.ConvertToScreenCoord(view^.pict, x, y);
            picture.ConvertToScreenCoord(view^.pict, x0, y0);
	    if abs(x0 - x) < abs(y0 - y) then
	        AddLine(
	            subpict, view^.stateInfo.lineType,
		    x0, y0, x0, y
  	        );
	    else
	        AddLine(
	            subpict, view^.stateInfo.lineType,
	            x0, y0, x, y0
	        );
	    end;
	    DrawInClipBox(subpict, view);
	    CreateRubberbands(view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
	    LogInsertion(view);
	    MakeDirty(view);
	    view^.stateInfo.activity := Idling;

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end PerpLineHandler;


(* export *)
procedure LineHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	x0 : XCoord;
	y0 : YCoord;
	subpict : Picture;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
	    ClipToClipBox(view);
	    ReleaseSelections(view);
            rubband.Create(r, rubband.RubberLine, 0);
	    rubband.Initialize(r, x, y, 0, 0, x, y);
            view^.stateInfo.rubberband := r;
	    view^.stateInfo.activity := Rubberbanding;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x0, y0, dumX, dumY, dumX, dumY);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    AllWritable();
	    subpict := CreateSubpict(view^.pict);
	    Append(subpict, view^.stateInfo.selecList);
            picture.ConvertToScreenCoord(view^.pict, x, y);
            picture.ConvertToScreenCoord(view^.pict, x0, y0);
	    AddLine(
	        subpict, view^.stateInfo.lineType,
	        x0, y0, x, y
  	    );
	    DrawInClipBox(subpict, view);
	    CreateRubberbands(view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
	    LogInsertion(view);
	    MakeDirty(view);
	    view^.stateInfo.activity := Idling;

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end LineHandler;


(* export *)
procedure CircleHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	x0 : XCoord;
	y0 : YCoord;
	distance : integer;
	subpict : Picture;
	rend : picture.RenderingStyle;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
	    ClipToClipBox(view);
	    ReleaseSelections(view);
            rubband.Create(r, rubband.RubberCircle, 0);
	    rubband.Initialize(r, x, y, 0, 0, x, y);
            view^.stateInfo.rubberband := r;
	    view^.stateInfo.activity := Rubberbanding;
	    SetRendering(INVERT);
	    Point(x, y);

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x0, y0, dumX, dumY, dumX, dumY);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    SetRendering(INVERT);
	    Point(x0, y0);
	    AllWritable();
	    subpict := CreateSubpict(view^.pict);
	    Append(subpict, view^.stateInfo.selecList);
            picture.ConvertToScreenCoord(view^.pict, x, y);
            picture.ConvertToScreenCoord(view^.pict, x0, y0);
	    distance := Distance(x0, y0, x, y);
	    rend := picture.GetRenderingStyle(subpict);
	    if not
		((rend = picture.Stroke) and (view^.stateInfo.lineType = None))
	    then
	        picture.Circle(subpict, x0, y0, distance);
		if
		    (rend # picture.Stroke) and
		    (view^.stateInfo.lineType # None)
		then
	            picture.DefaultRenderingStyle(subpict, picture.Stroke);
	            picture.Circle(subpict, x0, y0, distance);
		end;
	    end;
	    DrawInClipBox(subpict, view);
	    CreateRubberbands(view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
	    LogInsertion(view);
	    MakeDirty(view);
	    view^.stateInfo.activity := Idling;

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end CircleHandler;


(* export *)
procedure EllipseHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	x0 : XCoord;
	y0 : YCoord;
	r1, r2 : integer;
	subpict : Picture;
	rend : picture.RenderingStyle;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
	    ClipToClipBox(view);
	    ReleaseSelections(view);
            rubband.Create(r, rubband.RubberEllipse, 0);
	    rubband.Initialize(r, x, y, 0, 0, x, y);
            view^.stateInfo.rubberband := r;
	    view^.stateInfo.activity := Rubberbanding;
	    SetRendering(INVERT);
	    Point(x, y);

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x0, y0, dumX, dumY, dumX, dumY);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    SetRendering(INVERT);
	    Point(x0, y0);
	    AllWritable();
	    subpict := CreateSubpict(view^.pict);
	    Append(subpict, view^.stateInfo.selecList);
            picture.ConvertToScreenCoord(view^.pict, x, y);
            picture.ConvertToScreenCoord(view^.pict, x0, y0);
	    r1 := abs(x0 - x);
	    r2 := abs(y0 - y);
	    rend := picture.GetRenderingStyle(subpict);
	    if not
		((rend = picture.Stroke) and (view^.stateInfo.lineType = None))
	    then
	        picture.Ellipse(subpict, x0, y0, r1, r2);
		if
		    (rend # picture.Stroke) and
		    (view^.stateInfo.lineType # None)
		then
	            picture.DefaultRenderingStyle(subpict, picture.Stroke);
	            picture.Ellipse(subpict, x0, y0, r1, r2);
		end;
	    end;
	    DrawInClipBox(subpict, view);
	    CreateRubberbands(view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
	    LogInsertion(view);
	    MakeDirty(view);
	    view^.stateInfo.activity := Idling;

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end EllipseHandler;


(* export *)
procedure RectHandler(const eventWord : Word);
    var eventPkg : EventPkg;
        r : Rubberband;
	x0 : XCoord;
	y0 : YCoord;
	subpict : Picture;
	rend : picture.RenderingStyle;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
	    ClipToClipBox(view);
	    ReleaseSelections(view);
            rubband.Create(r, rubband.RubberRect, 0);
	    rubband.Initialize(r, x, y, 0, 0, x, y);
            view^.stateInfo.rubberband := r;
	    view^.stateInfo.activity := Rubberbanding;

        elsif (event = BUTTON) and (button = LEFTBUTTON) and up then
	    r := view^.stateInfo.rubberband;
            rubband.GetOrigin(r, x0, y0, dumX, dumY, dumX, dumY);
	    rubband.EraseAndDestroy(view^.stateInfo.rubberband);
	    AllWritable();
	    subpict := CreateSubpict(view^.pict);
	    Append(subpict, view^.stateInfo.selecList);
            picture.ConvertToScreenCoord(view^.pict, x, y);
            picture.ConvertToScreenCoord(view^.pict, x0, y0);
	    rend := picture.GetRenderingStyle(subpict);
	    if not
	        ((rend = picture.Stroke) and (view^.stateInfo.lineType = None))
	    then
		picture.Rectangle(subpict, x0, y0, x, y);
		if
		    (rend # picture.Stroke) and
		    (view^.stateInfo.lineType # None)
		then
	            picture.DefaultRenderingStyle(subpict, picture.Stroke);
	            picture.Rectangle(subpict, x0, y0, x, y);
		end;
	    end;
	    DrawInClipBox(subpict, view);
	    CreateRubberbands(view);
	    ClipToClipBox(view);
	    HighlightSelections(view);
	    AllWritable();
	    LogInsertion(view);
	    MakeDirty(view);
	    view^.stateInfo.activity := Idling;

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end RectHandler;


procedure VertexObjHandler(
    const vertexObjType : VertexObjType;
    const eventWord : Word
);
    var eventPkg : EventPkg;
        r : Rubberband;
	subpict, rerase : Picture;
	rend : picture.RenderingStyle;
	x0, x1 : XCoord;
	y0, y1 : YCoord;
begin
    eventPkg := EventPkg(eventWord);
    with eventPkg^ do
        if view^.stateInfo.activity # Rubberbanding then
	    ClipToClipBox(view);
	    ReleaseSelections(view);
            rubband.Create(r, rubband.RubberLine, 0);
	    rubband.Initialize(r, x, y, 0, 0, x, y);
            view^.stateInfo.rubberband := r;
	    view^.stateInfo.activity := Rubberbanding;
            picture.ConvertToScreenCoord(view^.pict, x, y);
	    view^.stateInfo.px[1] := x;
	    view^.stateInfo.py[1] := y;
	    view^.stateInfo.count := 1;

        elsif (event = BUTTON) and not up then
	    if 
	        (button = MIDDLEBUTTON) or 
		(view^.stateInfo.count = MAXPOLYPTS)
	    then
	        r := view^.stateInfo.rubberband;
		rubband.EraseAndDestroy(view^.stateInfo.rubberband);
		SetRendering(INVERT);
		x0 := view^.stateInfo.px[1];
		y0 := view^.stateInfo.py[1];
		x1 := view^.stateInfo.px[view^.stateInfo.count];
		y1 := view^.stateInfo.py[view^.stateInfo.count];
		picture.ConvertToWorldCoord(view^.pict, x0, y0);
		picture.ConvertToWorldCoord(view^.pict, x1, y1);
		Line(x0, y0, x1, y1);
		AllWritable();
		subpict := CreateSubpict(view^.pict);
		Append(subpict, view^.stateInfo.selecList);

		picture.Create(rerase);		(* erase rubberbandings *)
		picture.GetOrigin(subpict, x0, y0);
		picture.SetOrigin(rerase, x0, y0);
		picture.DefaultPaintType(rerase, picture.Xor);
		picture.DefaultRenderingStyle(rerase, picture.Stroke);
		picture.Polygon(
		    rerase,
		    view^.stateInfo.px[1:view^.stateInfo.count],
		    view^.stateInfo.py[1:view^.stateInfo.count]
		);
		DrawInClipBox(rerase, view);
		picture.Destroy(rerase);

		rend := picture.GetRenderingStyle(subpict);
		AddVertexObj(
		    subpict, vertexObjType, view^.stateInfo.lineType,
		    view^.stateInfo.px[1:view^.stateInfo.count],
		    view^.stateInfo.py[1:view^.stateInfo.count]
		);
		DrawInClipBox(subpict, view);
		CreateRubberbands(view);
	        ClipToClipBox(view);
	        HighlightSelections(view);
	        AllWritable();
		LogInsertion(view);
	        MakeDirty(view);
		view^.stateInfo.activity := Idling;
	    elsif button = LEFTBUTTON then
		view^.stateInfo.count := view^.stateInfo.count + 1;
		rubband.Initialize(view^.stateInfo.rubberband, x, y, 0, 0, x, y);
		picture.ConvertToScreenCoord(view^.pict, x, y);
		view^.stateInfo.px[view^.stateInfo.count] := x;
	    	view^.stateInfo.py[view^.stateInfo.count] := y;
	    end;	    

        elsif event = MOTION then
            rubband.Draw(view^.stateInfo.rubberband, x, y);
	end;
    end;
    DisposeEventPackage(eventPkg);
end VertexObjHandler;


(* export *)
procedure PolygonHandler(const eventWord : Word);
begin
    VertexObjHandler(Polygon, eventWord);
end PolygonHandler;


(* export *)
procedure SplineHandler(const eventWord : Word);
begin
    VertexObjHandler(Spline, eventWord);
end SplineHandler;


(* export *)
procedure ClosedSplineHandler(const eventWord : Word);
begin
    VertexObjHandler(ClosedSpline, eventWord);
end ClosedSplineHandler;
   
 
begin
end dvobjtools.
