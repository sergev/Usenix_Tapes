implementation module align;

(* export *)
from main import
    Picture, XCoord, YCoord, BoundingBox;

from ops import
    Translate;

(*
 * The following two global variables represent a gross hack that makes
 * undo of alignment operations in idraw possible.  Since idraw knows
 * nothing about the details of object alignment, yet needs to undo them,
 * it must at least have information about how much an object was moved when
 * it was aligned.  So when it calls upon a picture routine for alignment,
 * picture sets these global values to reflect the alignment translation.
 * idraw can then store this information for later undo.  Phffft!
 *)
(* public export var aligndx : real; *)
(* public export var aligndy : real; *)


(* public export *)
procedure AlignVertCenters(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := 0.0;
    aligndy := fixedPict^.bbox.center.y - movedPict^.bbox.center.y;
    Translate(movedPict, aligndx, aligndy);
end AlignVertCenters;


(* public export *)
procedure AlignHorizCenters(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := fixedPict^.bbox.center.x - movedPict^.bbox.center.x;
    aligndy := 0.0;
    Translate(movedPict, aligndx, aligndy);
end AlignHorizCenters;


(* public export *)
procedure AlignCenters(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := fixedPict^.bbox.center.x - movedPict^.bbox.center.x;
    aligndy := fixedPict^.bbox.center.y - movedPict^.bbox.center.y;
    Translate(movedPict, aligndx, aligndy);
end AlignCenters;


(* public export *)
procedure AlignLeftSides(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := float(fixedPict^.bbox.lowLeft.x - movedPict^.bbox.lowLeft.x);
    aligndy := 0.0;
    Translate(movedPict, aligndx, aligndy);
end AlignLeftSides;


(* public export *)
procedure AlignRightSides(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := float(fixedPict^.bbox.upRight.x - movedPict^.bbox.upRight.x);
    aligndy := 0.0;
    Translate(movedPict, aligndx, aligndy);
end AlignRightSides;


(* public export *)
procedure AlignTops(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := 0.0;
    aligndy := float(fixedPict^.bbox.upRight.y - movedPict^.bbox.upRight.y);
    Translate(movedPict, aligndx, aligndy);
end AlignTops;


(* public export *)
procedure AlignBottoms(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := 0.0;
    aligndy := float(fixedPict^.bbox.lowLeft.y - movedPict^.bbox.lowLeft.y);
    Translate(movedPict, aligndx, aligndy);
end AlignBottoms;


(* public export *)
procedure AlignLeftToRight(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := float(fixedPict^.bbox.upRight.x - movedPict^.bbox.lowLeft.x);
    aligndy := 0.0;
    Translate(movedPict, aligndx, aligndy);
end AlignLeftToRight;


(* public export *)
procedure AlignRightToLeft(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := float(fixedPict^.bbox.lowLeft.x - movedPict^.bbox.upRight.x);
    aligndy := 0.0;
    Translate(movedPict, aligndx, aligndy);
end AlignRightToLeft;


(* public export *)
procedure AlignBottomToTop(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := 0.0;
    aligndy := float(fixedPict^.bbox.upRight.y - movedPict^.bbox.lowLeft.y);
    Translate(movedPict, aligndx, aligndy);
end AlignBottomToTop;


(* public export *)
procedure AlignTopToBottom(
    const fixedPict : Picture;
    const movedPict : Picture
);   
begin
    aligndx := 0.0;
    aligndy := float(fixedPict^.bbox.lowLeft.y - movedPict^.bbox.upRight.y);
    Translate(movedPict, aligndx, aligndy);
end AlignTopToBottom;


begin
    aligndx := 0.0;
    aligndy := 0.0;
end align.
