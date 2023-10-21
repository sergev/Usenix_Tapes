implementation module dvscroll;

import
    picture,
    scrollbar;

(* export *)
from picture import
    XCoord, YCoord, Picture;

(* export *)
from dvdefs import
    DrawingView;

from dvdefs import
    StateType, ScrollBarRecord, LEFTBUTTON;

from dvselect import
    RecalcRubberbands;

from dvupict import
    ClearAndDrawInClipBox;

from genlists import
    GenericList, Release, Head;

(* export *)
from input import
    InputButton, EventType;

from utilities import
    round;


(* export *)
procedure InScrollBar(
    const view : DrawingView;
    const x : XCoord;
    const y : YCoord
) : boolean;
    var sblist : GenericList;
begin
    sblist := picture.SelectByPoint(view^.xscroll.scrollBar, x, y);
    if sblist # GenericList(nil) then
        view^.stateInfo.curScroll := view^.xscroll.scrollBar;
	Release(sblist);
	return true;
    end;
    sblist := picture.SelectByPoint(view^.yscroll.scrollBar, x, y);
    if sblist # GenericList(nil) then
        view^.stateInfo.curScroll := view^.yscroll.scrollBar;
	Release(sblist);
	return true;
    end;
    return false;
end InScrollBar;


(* export *)
procedure UpdateScrollBars(const view : DrawingView);
    var pgx0, pgx1 : XCoord;
        pgy0, pgy1 : YCoord;
begin
    picture.GetBoundingBox(view^.page, pgx0, pgy0, pgx1, pgy1);
    picture.ConvertToWorldCoord(view^.page, pgx0, pgy0);
    picture.ConvertToWorldCoord(view^.page, pgx1, pgy1);
    picture.ConvertToScreenCoord(view^.world, pgx0, pgy0);
    picture.ConvertToScreenCoord(view^.world, pgx1, pgy1);

    scrollbar.SetLimits(
        view^.xscroll.scrollBar, 
	pgx0, pgx1, view^.pictClipBox.x0, view^.pictClipBox.x1
    );
    scrollbar.SetLimits(
        view^.yscroll.scrollBar, 
	pgy0, pgy1, view^.pictClipBox.y0, view^.pictClipBox.y1
    );
end UpdateScrollBars;


(* export *)
procedure ScrollPict(
    const view : DrawingView;
    	  amount : integer
);
    var originX : XCoord;
        originY : YCoord;
begin
    if amount # 0 then
	picture.GetOrigin(view^.pict, originX, originY);
	if view^.stateInfo.curScroll = view^.xscroll.scrollBar then
	    originX := originX - XCoord(amount);
	else
	    originY := originY - YCoord(amount);
	end;        
        picture.SetOrigin(view^.pict, originX, originY);
	picture.SetOrigin(view^.page, originX, originY);
	RecalcRubberbands(view);
	UpdateScrollBars(view);
	picture.Redraw(view^.stateInfo.curScroll);
	ClearAndDrawInClipBox(view^.pict, view);
    end;
end ScrollPict;


(* export *)
procedure ScrollHandler(
    const view   : DrawingView;
    const button : InputButton;
    const up	 : boolean;
    	  x	 : XCoord;
    	  y 	 : YCoord;
    const c	 : char;
    const event  : EventType
);
    var sbpart : Picture;;
        sx0, sx1 : XCoord;
	sy0, sy1 : YCoord;
	cx, cy : real;
	amount : integer;
	b : boolean;
	whichsb : ScrollBarRecord;
begin
    view^.stateInfo.activity := Scrolling;        
    if view^.stateInfo.curScroll = view^.xscroll.scrollBar then
        whichsb := view^.xscroll;
    else
        whichsb := view^.yscroll;
    end;
    
    if (event = BUTTON) and (button = LEFTBUTTON) and not up then
        sbpart := Picture(
            Head(picture.SelectByPoint(view^.stateInfo.curScroll, x, y))
        );
    end;

    if (event = BUTTON) and (button = LEFTBUTTON) and up then
        ScrollPict(view, round(scrollbar.ReleaseSlider(whichsb.scrollBar)));
    elsif 
        (event = BUTTON) and (button = LEFTBUTTON) and
	not up and (sbpart # whichsb.slider)
    then
	if sbpart = whichsb.pageArea then
	    picture.GetCenter(whichsb.slider, cx, cy);
	    amount := round(
	        scrollbar.TranslateSlider(
		    whichsb.scrollBar, x - round(cx), y - round(cy)
		)
	    );
	else	(* we're auto scrolling *)
	    picture.GetBoundingBox(whichsb.slider, sx0, sy0, sx1, sy1);
	    sx0 := sx0 - sx1;
	    sy0 := sy0 - sy1;
	    if sbpart = whichsb.autoUpButton then
	        sx0 := -sx0;
		sy0 := -sy0;
	    end;
	    amount := round(
		scrollbar.TranslateSlider(whichsb.scrollBar, sx0, sy0)
	    );
	end;
	ScrollPict(view, amount);
    else	(* we're moving the slider around *)
        b := scrollbar.TrackSlider(whichsb.scrollBar, x, y);
	return;
    end;
    view^.stateInfo.curScroll := Picture(nil);
    view^.stateInfo.activity := Idling;
end ScrollHandler;


begin
end dvscroll.
