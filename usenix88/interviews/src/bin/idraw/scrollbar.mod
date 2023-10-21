implementation module scrollbar;


import
    picture,
    rubband;

(* export *)
from picture import
    Picture, XCoord, YCoord;
    
from vdi import
    SetColors, Rectangle, stipplePattern1;

from utilities import
    round, PointInBox;


const
    SPACE = 4;
    MINSLIDERLENGTH = 3;


type
    ScrollBar = pointer to ScrollBarRecord;
    ScrollBarRecord = record
	vertical	: boolean;
        autoDownButton  : Picture;
	autoUpButton	: Picture;
	pageArea	: Picture;
	slider		: Picture;
	worldmin	: integer;
	worldmax	: integer;
	windowmin	: integer;
	windowmax	: integer;
	r		: rubband.Rubberband;
	axis		: integer;
	minlimit	: integer;
	maxlimit	: integer;
    end;


(* export *)
procedure Create(
    width, height: integer;
    var autoDownButton, autoUpButton, pageArea, slider : Picture
) : Picture;
    var sb : Picture;
        sbinfo : ScrollBar;
        arrowX : array [1..4] of XCoord;
	arrowY : array [1..4] of YCoord;
	temp : integer;
begin
    new(sbinfo);
    sbinfo^.r := rubband.Rubberband(nil);
    if width < height then
        sbinfo^.vertical := true;
	temp := height;
	height := width;
	width := temp;
    else
        sbinfo^.vertical := false;
    end;
    picture.Create(sb);

    picture.Create(sbinfo^.autoDownButton);
    picture.DefaultRenderingStyle(sbinfo^.autoDownButton, picture.Fill);
    picture.DefaultColor(sbinfo^.autoDownButton, picture.WHITE);
    picture.Rectangle(sbinfo^.autoDownButton, 0, 0, height, height);
    picture.DefaultRenderingStyle(sbinfo^.autoDownButton, picture.Stroke);
    picture.DefaultColor(sbinfo^.autoDownButton, picture.BLACK);
    picture.Rectangle(sbinfo^.autoDownButton, 0, 0, height, height);
    arrowX[1] := SPACE;
    arrowY[1] := height div 2;
    arrowX[2] := height - SPACE;
    arrowY[2] := SPACE;
    arrowX[3] := height - SPACE;
    arrowY[3] := height - SPACE;
    arrowX[4] := arrowX[1];
    arrowY[4] := arrowY[1];
    picture.Polygon(sbinfo^.autoDownButton, arrowX, arrowY);

    sbinfo^.autoUpButton := picture.Copy(sbinfo^.autoDownButton);
    picture.Rotate(sbinfo^.autoUpButton, 180.0);
    picture.Translate(sbinfo^.autoUpButton, float(width), 0.0);
    
    picture.Create(sbinfo^.pageArea);
    picture.DefaultRenderingStyle(sbinfo^.pageArea, picture.Fill);
    picture.DefaultColor(sbinfo^.pageArea, picture.WHITE);
    picture.Rectangle(sbinfo^.pageArea, height, 0, width - height, height);
    picture.DefaultRenderingStyle(sbinfo^.pageArea, picture.Stroke);
    picture.DefaultColor(sbinfo^.pageArea, picture.BLACK);
    picture.Rectangle(sbinfo^.pageArea, height, 0, width - height, height);

    picture.Create(sbinfo^.slider);
    picture.DefaultRenderingStyle(sbinfo^.slider, picture.Fill);
    picture.DefaultPattern(sbinfo^.slider, stipplePattern1);
    picture.Rectangle(sbinfo^.slider, height, SPACE, width - height, height - SPACE);
    picture.DefaultRenderingStyle(sbinfo^.slider, picture.Stroke);
    picture.DefaultColor(sbinfo^.slider, picture.BLACK);
    picture.Rectangle(sbinfo^.slider, height, SPACE, width - height, height - SPACE);

    sbinfo^.worldmin := 0;
    sbinfo^.worldmax := width - 2 * height;
    sbinfo^.windowmin := sbinfo^.worldmin;
    sbinfo^.windowmax := sbinfo^.worldmax;

    picture.SetUserData(sb, sbinfo);
    picture.Nest(sbinfo^.pageArea, sb);
    picture.Nest(sbinfo^.slider, sb);
    picture.Nest(sbinfo^.autoDownButton, sb);
    picture.Nest(sbinfo^.autoUpButton, sb);
    
    autoDownButton := sbinfo^.autoDownButton;
    autoUpButton := sbinfo^.autoUpButton;
    pageArea := sbinfo^.pageArea;
    slider := sbinfo^.slider;    
    
    if sbinfo^.vertical then
        picture.RotateAboutPoint(sb, 90.0, 0.0, 0.0);
	picture.Translate(sb, float(height), 0.0);
    end;
    return sb;
end Create;


procedure UpdateScrollBar(const sb : Picture);
    var pgx0, pgx1, sx0, sx1 : XCoord;
        pgy0, pgy1, sy0, sy1 : YCoord;
	ratio : real;
	sbinfo : ScrollBar;
begin
    sbinfo := ScrollBar(picture.GetUserData(sb));
    with sbinfo^ do
        picture.GetBoundingBox(pageArea, pgx0, pgy0, pgx1, pgy1);
        picture.GetBoundingBox(slider, sx0, sy0, sx1, sy1);
    
        if not vertical then
	    sx1 := max(sx1 - sx0, MINSLIDERLENGTH) + sx0;
            ratio := float(pgx1 - pgx0) / float(worldmax - worldmin);
	    picture.ScaleAboutPoint(
	        slider, 
	        ratio * float(windowmax - windowmin) / float(sx1 - sx0), 1.0,
	        float(sx0), float(sy0)
	    );
	    picture.Translate(
	        slider,
		ratio * float(windowmin - worldmin) + float(pgx0 - sx0), 0.0
	    );
	else
	    sy1 := max(sy1 - sy0, MINSLIDERLENGTH) + sy0;
            ratio := float(pgy1 - pgy0) / float(worldmax - worldmin);
	    picture.ScaleAboutPoint(
	        slider, 
	        1.0, ratio * float(windowmax - windowmin) / float(sy1 - sy0),
	        float(sx0), float(sy0)
	    );
	    picture.Translate(
	        slider,
		0.0, ratio * float(windowmin - worldmin) + float(pgy0 - sy0)
	    );
	end;
    end;
end UpdateScrollBar;


(* export *)
procedure SetLimits(
    const sb : Picture;
    const worldmin, worldmax, windowmin, windowmax : integer
);
    var sbinfo : ScrollBar;
begin
    sbinfo := ScrollBar(picture.GetUserData(sb));
    sbinfo^.worldmin := worldmin;
    sbinfo^.worldmax := worldmax;
    sbinfo^.windowmin := max(windowmin, worldmin);
    sbinfo^.windowmax := min(windowmax, worldmax);
    UpdateScrollBar(sb);
end SetLimits;


(* export *)
procedure GetLimits(
    const sb : Picture;
    var   worldmin, worldmax, windowmin, windowmax : integer
);
    var sbinfo : ScrollBar;
begin
    sbinfo := ScrollBar(picture.GetUserData(sb));
    worldmin := sbinfo^.worldmin;
    worldmax := sbinfo^.worldmax;
    windowmin := sbinfo^.windowmin;
    windowmax := sbinfo^.windowmax;
end GetLimits;


(* export *)
procedure ChangeLength(
    const sb : Picture;
    const length : integer
);
    var pgx0, pgx1, ax0, ax1 : XCoord;
        pgy0, pgy1, ay0, ay1 : YCoord;
	sbinfo : ScrollBar;
begin
    sbinfo := ScrollBar(picture.GetUserData(sb));
    picture.GetBoundingBox(sbinfo^.pageArea, pgx0, pgy0, pgx1, pgy1);
    picture.GetBoundingBox(sbinfo^.autoDownButton, ax0, ay0, ax1, ay1);
    
    if not sbinfo^.vertical then
        picture.ScaleAboutPoint(
	    sbinfo^.pageArea,
	    float(length - 2*(ax1 - ax0)) / float(pgx1 - pgx0), 1.0,
	    float(pgx0), float(pgy0)
	);
	picture.AlignLeftToRight(sbinfo^.pageArea, sbinfo^.autoUpButton);
    else
        picture.ScaleAboutPoint(
	    sbinfo^.pageArea,
	    1.0, float(length - 2*(ay1 - ay0)) / float(pgy1 - pgy0),
	    float(pgx0), float(pgy0)
	);
	picture.AlignBottomToTop(sbinfo^.pageArea, sbinfo^.autoUpButton);
    end;
    UpdateScrollBar(sb);
end ChangeLength;


(* export *)
procedure TranslateSlider(
    const sb : Picture;
    	  amountX, amountY : integer
) : real;
    var sbinfo : ScrollBar;
        sx0, sx1, pgx0, pgx1 : XCoord;
	sy0, sy1, pgy0, pgy1 : YCoord;
begin
    sbinfo := ScrollBar(picture.GetUserData(sb));
    picture.GetBoundingBox(sbinfo^.slider, sx0, sy0, sx1, sy1);
    picture.GetBoundingBox(sbinfo^.pageArea, pgx0, pgy0, pgx1, pgy1);

    if sbinfo^.vertical then
        if amountY = 0 then
	    return 0.0;
        elsif amountY < 0 then
	   amountY := max(amountY, pgy0 - sy0);
	else
           amountY := min(amountY, pgy1 - sy1);
	end;
	picture.Translate(sbinfo^.slider, 0.0, float(amountY));
        return 
	    float(sbinfo^.worldmax - sbinfo^.worldmin) / float(pgy1 - pgy0)
	    * float(amountY);
    else
	if amountX = 0 then
	    return 0.0;
        elsif amountX < 0 then
            amountX := max(amountX, pgx0 - sx0);
        else
	    amountX := min(amountX, pgx1 - sx1);
	end;
	picture.Translate(sbinfo^.slider, float(amountX), 0.0);
        return 
	    float(sbinfo^.worldmax - sbinfo^.worldmin) / float(pgx1 - pgx0)
	    * float(amountX);
    end;
end TranslateSlider;


(* export *)
procedure TrackSlider(
    const sb : Picture;
     	  x : XCoord;
    	  y : YCoord
) : boolean;
    var sbinfo : ScrollBar;
        sx0, sx1, pgx0, pgx1 : XCoord;
	sy0, sy1, pgy0, pgy1 : YCoord;
begin
    sbinfo := ScrollBar(picture.GetUserData(sb));
    picture.ConvertToWorldCoord(sb, x, y);

    if sbinfo^.r = rubband.Rubberband(nil) then
        picture.GetBoundingBox(sbinfo^.slider, sx0, sy0, sx1, sy1);
	picture.ConvertToWorldCoord(sb, sx0, sy0);
	picture.ConvertToWorldCoord(sb, sx1, sy1);
	if not PointInBox(x, y, sx0, sy0, sx1, sy1) then
	    return false;
	end;

	picture.GetBoundingBox(sbinfo^.pageArea, pgx0, pgy0, pgx1, pgy1);
	picture.ConvertToWorldCoord(sb, pgx0, pgy0);
	picture.ConvertToWorldCoord(sb, pgx1, pgy1);
	
    	rubband.Create(sbinfo^.r, rubband.SlidingRect, 0);
	rubband.Initialize(sbinfo^.r, sx0, sy0, sx1, sy1, x, y);
	if sbinfo^.vertical then
	    sbinfo^.axis := integer(x);
	    sbinfo^.minlimit := integer(pgy0 + y - sy0);
	    sbinfo^.maxlimit := integer(pgy1 + y - sy1);
	else
	    sbinfo^.axis := integer(y);
	    sbinfo^.minlimit := integer(pgx0 + x - sx0);
	    sbinfo^.maxlimit := integer(pgx1 + x - sx1);
	end;

    elsif sbinfo^.vertical then
        rubband.Draw(
	    sbinfo^.r, sbinfo^.axis, 
	    min(sbinfo^.maxlimit, max(sbinfo^.minlimit, y))
	);
    elsif not sbinfo^.vertical then
    	rubband.Draw(
	    sbinfo^.r, min(sbinfo^.maxlimit, max(sbinfo^.minlimit, x)),
	    sbinfo^.axis
	);
    end;
    return true;
end TrackSlider;


(* export *)
procedure ReleaseSlider(const sb : Picture) : real;
    var sbinfo : ScrollBar;
        x, pgx0, pgx1, sx0, sx1 : XCoord;
	y, pgy0, pgy1, sy0, sy1 : YCoord;
begin 
    sbinfo := ScrollBar(picture.GetUserData(sb));
    rubband.GetOrigin(sbinfo^.r, x, y, pgx0, pgy0, pgx1, pgy1);
    rubband.EraseAndDestroy(sbinfo^.r);
    picture.ConvertToScreenCoord(sb, x, y);
    picture.GetBoundingBox(sbinfo^.pageArea, pgx0, pgy0, pgx1, pgy1);
    picture.GetBoundingBox(sbinfo^.slider, sx0, sy0, sx1, sy1);
    
    if sbinfo^.vertical then
	picture.Translate(sbinfo^.slider, 0.0, float(y - sy0));
        return 
	    float(sbinfo^.worldmax - sbinfo^.worldmin) / float(pgy1 - pgy0)
	    * float(y - sy0);
    else
	picture.Translate(sbinfo^.slider, float(x - sx0), 0.0);
        return 
	    float(sbinfo^.worldmax - sbinfo^.worldmin) / float(pgx1 - pgx0)
	    * float(x - sx0);
    end;
end ReleaseSlider;
    


begin
end scrollbar.
