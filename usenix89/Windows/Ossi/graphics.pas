
(***************************************************************************)
(***                                                                     ***)
(***                                                                     ***)
(***                     O  S  S  I                                      ***)
(***                     ==========                                      ***)
(***                                                                     ***)
(**)               DEFINITION MODULE SIGraphics;                         (**)
(***               ============================                          ***)
(***                                                                     ***)
(***   This module supports graphic output to windows.                   ***)
(***                                                                     ***)
(***---------------------------------------------------------------------***)
(***                                                                     ***)
(***   Hardware:          independent                                    ***)
(***   Operating System:  independent                                    ***)
(***   Compiler:          independent                                    ***)
(***                                                                     ***)
(***   Version:      4.0                                                 ***)
(***   Implemented:  see copyright                                       ***)
(***   Date:         1986-11-17                                          ***)
(***                                                                     ***)
(***---------------------------------------------------------------------***)
(***                                                                     ***)
(***   Copyright 1986 by                                                 ***)
(***      E. S. Biagioni                                                 ***)
(***      G. Heiser                                                      ***)
(***      K. Hinrichs                                                    ***)
(***      C. Muller                                                      ***)
(***                                                                     ***)
(***   Institut fuer Informatik                                          ***)
(***   ETH Zuerich                                                       ***)
(***   CH 8092 Zuerich                                                   ***)
(***   Switzerland                                                       ***)
(***                                                                     ***)
(***   Department of Computer Science                                    ***)
(***   University of North Carolina                                      ***)
(***   Chapel Hill, North Carolina 27514                                 ***)
(***   U.S.A.                                                            ***)
(***                                                                     ***)
(*** Permission to copy without fee all of this material is granted      ***)
(*** provided that the copies are not made or distributed for direct     ***)
(*** commercial advantage, that this OSSI copyright notice is            ***)
(*** included in the copy, that the module is not modified in any way    ***)
(*** except where necessary for compilation on a particular system,      ***)
(*** and that the module is always distributed in its original form.     ***)
(*** Distribution of this module in a modified form without including    ***)
(*** the original version is a violation of this copyright notice.       ***)
(***                                                                     ***)
(***---------------------------------------------------------------------***)
(***                                                                     ***)
(***   Updates:                                                          ***)
(***                                                                     ***)
(***                                                                     ***)
(***************************************************************************)

(* This module supports graphic output to windows.

   Operations supported are reading and writing individual pixels, drawing
   lines, rectangles, circles, ellipses and parts of ellipses, and filling
   rectangles, circles and ellipses and sectors with a specified pattern.

   The window can be thought of as being divided by a rectangular grid into
   small cells, each one the size of a pattern. When an area is filled with
   a pattern, a pattern is written into each cell (clipping as appropriate).
   This implies that when filling adjacent areas with the same pattern,
   there will be no discontinuity at the border.

   All objects drawn are clipped to the inner rectangle of the corresponding
   window. *)


FROM SIWindows IMPORT
   Rectangle,
   Window,
   Bitmap,
   PaintMode;


EXPORT QUALIFIED
   Point,
   BorderMode,
   EmptyPattern,
   FullPattern,
   GreyPattern,
   PixelIsSet,
   DrawPixel,
   DrawRectangle,
   DrawPolygon,
   DrawCircle,
   DrawEllipse,
   DrawArc,
   FillRectangle,
   FillPolygon,
   FillCircle,
   FillEllipse,
   FillSector;


TYPE Point = RECORD
               h, v: INTEGER;
             END (* Point *);

     BorderMode = (noBorder, solidBorder, backgroundBorder);
     (* An area (rectangle, oval) has a border and an interior.
        When filling an area with a pattern, the interior of the area
        is filled with the pattern according to the specified paint mode.
        The drawing mode of the border is specified by the BorderMode:
           noBorder:
              no explicit border is drawn. The border line is filled
              with the pattern.
           solidBorder:
              the border is drawn as a solid line with the window's default
              paint mode.
           backgroundBorder:
              the border is not drawn, that is the border pixels are left
              unmodified. *)


(* If the procedure SIWindows.DisposeBitmap is called for the following
   predefined patterns the result is undefined! Empty pattern is the
   color of the window's default background, full pattern the opposite
   pattern, and grey pattern is somewhere in between. *)

PROCEDURE EmptyPattern (): Bitmap;

PROCEDURE FullPattern (): Bitmap;

PROCEDURE GreyPattern (): Bitmap;


PROCEDURE PixelIsSet (w: Window; h, v: INTEGER): BOOLEAN;
(* returns whether pixel (h, v) (in window coordinates) is set.
   False is returned if the point lies outside the inner rectangle. *)

PROCEDURE DrawPixel (w: Window; h, v: INTEGER; mode: PaintMode);
(* sets pixel (h, v) (in window coordinates) if mode = paint, and erases it
   if mode = erase. The other modes behave as expected for s = 1. *)


PROCEDURE DrawLine (w: Window; from, to: Point; mode: PaintMode);
(* draws a line from point from to point to. *)

PROCEDURE DrawRectangle (w: Window; r: Rectangle; mode: PaintMode);
(* draws rectangle border given by left, right, top, bottom (in window
   coordinates) using mode. *)

PROCEDURE DrawPolygon (w: Window;
                       polygon: ARRAY OF Point; nbrPoints: CARDINAL;
                       mode: PaintMode);
(* draws closed polygon consisting of nbrPoints points using mode.
   If HIGH (polygon) < nbrPoints - 1 the result is unspecified.
   The last point is connected to the first point. *)

PROCEDURE DrawCircle (w: Window;
                      center: Point; radius: CARDINAL; mode: PaintMode);
(* draws circle with radius and center using mode. *)

PROCEDURE DrawEllipse (w: Window;
                       center: Point; hHalfAxis, vHalfAxis: CARDINAL;
                       mode: PaintMode);
(* hHalfaxis and vHalfaxis are the lengths of the half axes, center
   is the center of the ellipse. *)

PROCEDURE DrawArc (w: Window;
                   center: Point; hHalfAxis, vHalfAxis: CARDINAL;
                   fromAngle, toAngle: REAL; mode: PaintMode);
(* draws an arc, i.e. part of an ellipse defined as above by center,
   hHalfAxis, vHalfAxis. fromAngle and toAngle are given in radians
   measured clockwise from the h-axis and delimit the arc to be
   drawn. *)


(* The following procedures fill interior of area with pattern using mode.
   The border is drawn according to bmode. *)

PROCEDURE FillRectangle (w: Window; r: Rectangle; pattern: Bitmap;
                         mode: PaintMode; bmode: BorderMode);

PROCEDURE FillPolygon (w: Window;
                       polygon: ARRAY OF Point; nbrPoints: CARDINAL;
                       pattern: Bitmap; mode: PaintMode; bmode: BorderMode);

PROCEDURE FillCircle (w: Window; center: Point; radius: CARDINAL;
                      pattern: Bitmap; mode: PaintMode; bmode: BorderMode);

PROCEDURE FillEllipse (w: Window; center: Point;
                       hHalfAxis, vHalfAxis: CARDINAL;
                       pattern: Bitmap; mode: PaintMode; bmode: BorderMode);

PROCEDURE FillSector (w: Window; center: Point; hHalfAxis, vHalfAxis: CARDINAL;
                      fromAngle, toAngle: REAL;
                      pattern: Bitmap; mode: PaintMode; bmode: BorderMode);

END SIGraphics.
