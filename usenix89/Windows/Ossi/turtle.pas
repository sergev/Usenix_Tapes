
(***************************************************************************)
(***                                                                     ***)
(***                                                                     ***)
(***                     O  S  S  I                                      ***)
(***                     ==========                                      ***)
(***                                                                     ***)
(**)               DEFINITION MODULE SITurtle;                           (**)
(***               ==========================                            ***)
(***                                                                     ***)
(***   This module supports turtle graphic in windows.                   ***)
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

(* This module supports simple turtle graphics in windows. A window used
   for turtle operations has a current turtle position (in window
   coordinates) as well as a current turtle angle of type INTEGER: this
   angle is measured in degrees clockwise from the horizontal axis (Degrees
   are used for greater simplicity). When a window is created, the turtle
   position is initialized to (0, 0) which is at the top left corner of the
   inner rectangle, while the turtle angle is set to 0 (pointing right).
   Operations supported are setting and inquiring the turtle's position and
   heading and moving the turtle (drawing lines). The turtle may be
   positioned and moved outside a window. However, nothing will be drawn
   in this case. If a line has to be drawn the coordinates of the turtle 
   position are rounded. *)


FROM SIWindows IMPORT
   Window,
   PaintMode;


EXPORT QUALIFIED
   GetTurtlePos,
   MoveTo,
   MoveBy,
   MoveFor,
   GetAngle,
   TurnTo,
   TurnBy,
   DrawTo,
   DrawBy,
   DrawFor;


PROCEDURE GetTurtlePos (w: Window; VAR h, v: INTEGER);
(* returns current turtle position (in window coordinates). *)

PROCEDURE MoveTo (w: Window; h, v: INTEGER);
(* sets turtle position. *)
   
PROCEDURE MoveBy (w: Window; dh, dv: INTEGER);
(* moves turtle relative to the current position. *)
   
PROCEDURE MoveFor (w: Window; distance: INTEGER);
(* moves turtle by distance in the direction given by the turtle's current
   angle. A negative distance moves the turtle in the direction opposite to
   the one given by angle. This distance will be rounded to the nearest
   pixel. *)


(* Inquire and define the current turtle angle. The angle is in degrees
   and when returned has a value between 0 and 359. Angle parameters will
   be converted to be modulo 360. *)

PROCEDURE GetAngle (w: Window): INTEGER;

PROCEDURE TurnTo (w: Window; angle: INTEGER);

PROCEDURE TurnBy (w: Window; by: INTEGER);


PROCEDURE DrawTo (w: Window; h, v: INTEGER; mode: PaintMode);
(* draws a line from the current turtle position to position (h, v) (in
   window coordinates). (h, v) becomes the new turtle position. *)

PROCEDURE DrawBy (w: Window; dh, dv: INTEGER; mode: PaintMode);
(* draws a line from the current turtle position to the new turtle
   position which is given relative to the current position. *)

PROCEDURE DrawFor (w: Window; distance: INTEGER; mode: PaintMode);
(* draws a line from the current turtle position in the direction given by
   the turtle's current angle. The end point of the line becomes the new
   turtle position. This end point is rounded to the nearest pixel. *)


END SITurtle.
