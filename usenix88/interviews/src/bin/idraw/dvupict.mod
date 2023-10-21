implementation module dvupict;

import
    picture;

(* export *)
from dvdefs import
    DrawingView, Box;

from dvdefs import
    GRIDSIZE;

from dvselect import
    HighlightSelectionsInBbox, RecalcRubberbands;

from dialog import
    GetPicture;

(* export *)
from picture import
    Picture, XCoord, YCoord, WHITE;

from vdi import
    SetColors, SetRendering, RenderingFunction, FilledRectangle, Writable;

from utilities import
    round;


(*
 *  Determine the size of the clipBox based on the user's drawing window.
 *  Return the result (in world coords).
 *)
(* export *)
procedure CalcClipBox(const view : DrawingView) : Box;
    var clipBox : Box;
begin
    picture.GetBoundingBox(
        picture.GetNestedPicture(view^.world, -1), (* last obj is clipBox *)
	clipBox.x0, clipBox.y0, clipBox.x1, clipBox.y1
    );
    return clipBox;
end CalcClipBox;


(*
 *  Convert the clipBox to the given picture's coordinate system and
 *  return the result.
 *)
(* export *)
procedure CalcPictClipBox(
    const pict : Picture;
    const view : DrawingView
) : Box;
    var box : Box;
begin
    box := view^.pictClipBox;
    picture.ConvertToWorldCoord(view^.world, box.x0, box.y0);
    picture.ConvertToWorldCoord(view^.world, box.x1, box.y1);
    picture.ConvertToScreenCoord(pict, box.x0, box.y0);
    picture.ConvertToScreenCoord(pict, box.x1, box.y1);
    box.x0 := box.x0 + 1;
    box.y0 := box.y0 + 1;
    box.x1 := box.x1 - 1;
    box.y1 := box.y1 - 1;
    return box;
end CalcPictClipBox;


(* export *)
procedure CalcCenterPictClipBox(
    const view : DrawingView;
    var   x, y : real
);
    var box : Box;
begin
    box := view^.pictClipBox;
    picture.ConvertToWorldCoord(view^.world, box.x0, box.y0);
    picture.ConvertToWorldCoord(view^.world, box.x1, box.y1);
    picture.ConvertToScreenCoord(view^.pict, box.x0, box.y0);
    picture.ConvertToScreenCoord(view^.pict, box.x1, box.y1);
    x := float(box.x0 + box.x1) / 2.0;
    y := float(box.y0 + box.y1) / 2.0;
end CalcCenterPictClipBox;


(* export *)
procedure CenterInClipBox(
    const pict : Picture;
    const view : DrawingView
);
    var cx, cy, px, py : real;
begin
    CalcCenterPictClipBox(view, cx, cy);
    picture.GetCenter(pict, px, py);
    picture.Translate(pict, cx - px, cy - py);
end CenterInClipBox;


(* export *)
procedure ClipToClipBox(const view : DrawingView);
    var box : Box;
begin
    box := view^.pictClipBox;
    picture.ConvertToWorldCoord(view^.world, box.x0, box.y0);
    picture.ConvertToWorldCoord(view^.world, box.x1, box.y1);
    Writable(box.x0 + 1, box.y0 + 1, box.x1 - 1, box.y1 - 1);
end ClipToClipBox;


(* export *)
procedure ClearAndDrawInClipBox(
    const pict : Picture;
    const view : DrawingView
);
    var box : Box;
begin
    box := view^.pictClipBox;
    box.x0 := box.x0 + 1;
    box.y0 := box.y0 + 1;
    box.x1 := box.x1 - 1;
    box.y1 := box.y1 - 1;
    picture.ConvertToWorldCoord(view^.world, box.x0, box.y0);
    picture.ConvertToWorldCoord(view^.world, box.x1, box.y1);
    SetColors(WHITE, WHITE);
    SetRendering(PAINTFGBG);
    FilledRectangle(box.x0, box.y0, box.x1, box.y1);
    picture.ConvertToScreenCoord(pict, box.x0, box.y0);
    picture.ConvertToScreenCoord(pict, box.x1, box.y1);
    picture.DrawInBoundingBox(pict, box.x0, box.y0, box.x1, box.y1);
    picture.DrawInBoundingBox(view^.page, box.x0, box.y0, box.x1, box.y1);
    HighlightSelectionsInBbox(view, box.x0, box.y0, box.x1, box.y1);
end ClearAndDrawInClipBox;    


(* export *)
procedure DrawInClipBox(
    const pict : Picture;
    const view : DrawingView
);
    var box : Box;
begin
    box := CalcPictClipBox(pict, view);
    picture.DrawInBoundingBox(pict, box.x0, box.y0, box.x1, box.y1);
    picture.DrawInBoundingBox(view^.page, box.x0, box.y0, box.x1, box.y1);
end DrawInClipBox;    


(* export *)
procedure RedrawInClipBox(
    const pict : Picture;
    const view : DrawingView
);
    var box : Box;
begin
    box := CalcPictClipBox(pict, view);
    picture.RedrawInBoundingBox(pict, box.x0, box.y0, box.x1, box.y1);
    picture.DrawInBoundingBox(view^.page, box.x0, box.y0, box.x1, box.y1);
end RedrawInClipBox;    


(* export *)
procedure DrawPageInClipBox(const view : DrawingView);
    var box : Box;
begin
    box := CalcPictClipBox(view^.page, view);
    picture.DrawInBoundingBox(view^.page, box.x0, box.y0, box.x1, box.y1);
end DrawPageInClipBox;    


(* export *)
procedure CenterPointOnPage(
    const view : DrawingView;
    const x, y : real
);
    var dx, ox : XCoord;
	dy, oy : YCoord;
	cx, cy : real;
begin
    CalcCenterPictClipBox(view, cx, cy);
    dx := round(cx - x);
    dy := round(cy - y);
    picture.GetOrigin(view^.page, ox, oy);
    ox := ox + GRIDSIZE * (dx div GRIDSIZE);
    oy := oy + GRIDSIZE * (dy div GRIDSIZE);
    picture.SetOrigin(view^.page, ox, oy);
    picture.SetOrigin(view^.pict, ox, oy);
    RecalcRubberbands(view);
end CenterPointOnPage;


(* export *)
procedure CenterPage(const view : DrawingView);
    var x, y : real;
begin
    picture.GetCenter(view^.page, x, y);
    CenterPointOnPage(view, x, y);
end CenterPage;


(* export *)
procedure GetNearestGridPoint(
    const view : DrawingView;
    const x, y : real;		(* expects screen coords *)
    var   gpx : XCoord;
    var   gpy : YCoord
);
    var origx : XCoord;
        origy : YCoord;
begin
    picture.GetOrigin(view^.pict, origx, origy);
    gpx := round(x);
    gpy := round(y);
    picture.ConvertToWorldCoord(view^.pict, gpx, gpy);
    gpx := GRIDSIZE * (gpx div GRIDSIZE) + origx mod GRIDSIZE;
    gpy := GRIDSIZE * (gpy div GRIDSIZE) + origy mod GRIDSIZE;
    picture.ConvertToScreenCoord(view^.pict, gpx, gpy);
end GetNearestGridPoint;


begin
end dvupict.
