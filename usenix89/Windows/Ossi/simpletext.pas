
(***************************************************************************)
(***                                                                     ***)
(***                                                                     ***)
(***                     O  S  S  I                                      ***)
(***                     ==========                                      ***)
(***                                                                     ***)
(**)               DEFINITION MODULE SISimpleText;                       (**)
(***               ==============================                        ***)
(***                                                                     ***)
(***   This module supports line/column oriented output to windows.      ***)
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

(* This module supports line/column oriented text output to windows. The text
   is written in a fixed-width standard font. A window used by this module
   has a current text position which is given by (line, column) coordinates
   and corresponds to a character position. Text lines are counted from the
   top starting at 0, columns are counted left to right starting at 0. The
   current text position can be changed by procedures of this module only.
   This text position is different from the pixel position of the module
   SIAdvancedText. The procedures of this module have no influence on the
   pixel position of SIAdvancedText, and the procedures of SIAdvancedText
   have no influence on the text position of this module. Text output by the
   procedures of this module is written at the current text position (thereby
   changing the current text position). When a window is created the text
   position is initialized to (0, 0) (i.e. the character in the top left
   corner of the inner rectangle) and wraparound and highlighting are not
   set. Text may be written outside a window; however, it is clipped to the
   inner rectangle of the window. When scrolling occurs, new "scrolled in"
   lines are blank. Anything scrolled out of the window is lost. *)


FROM SIWindows IMPORT
   Rectangle,
   Window;


EXPORT QUALIFIED
   SetWrapAround,
   WrapAroundIsSet,
   SetHighLight,
   HighLightIsSet,
   GetTextPos,
   SetTextPos,
   GetTextPosRectangle,
   LinesInWindow,
   CharsInLine,
   Write,
   WriteString,
   WriteLn,
   SetTerminalWindow,
   EraseToEOL,
   Scroll;


PROCEDURE SetWrapAround (w: Window; on: BOOLEAN);
(* sets wraparound if on is TRUE, otherwise wraparound is turned off. *)

PROCEDURE WrapAroundIsSet (w: Window): BOOLEAN;
(* returns whether wraparound is set. *)

PROCEDURE SetHighLight (w: Window; on: BOOLEAN);
(* does system dependent highlighting of the characters written
   from now until highlighting is turned off. *)

PROCEDURE HighLightIsSet (w: Window): BOOLEAN;
(* returns whether highlighting is set. *)

PROCEDURE GetTextPos (w: Window; VAR line, column: INTEGER);
(* returns the current text position of w in character units. *)

PROCEDURE SetTextPos (w: Window; line, column: INTEGER);
(* sets the current text position of w in character units. *)

PROCEDURE GetTextPosRectangle (w: Window; VAR r: Rectangle);
(* returns the window coordinates of the current text position.
   This also returns the size of a character, including any
   blank space between lines. *)

PROCEDURE LinesInWindow (w: Window): CARDINAL;
(* returns the number of lines that can be written onto the inner
   rectangle of window w. *)

PROCEDURE CharsInLine (w: Window): CARDINAL;
(* returns the number of characters that can be written onto one line of
   the inner rectangle of window w. *)

PROCEDURE Write (w: Window; ch: CHAR);
(* writes ch at the current text position (l, c) in w using the current paint
   mode of w; the new text position becomes (l, c + 1). If wraparound is set
   and c = wC, l # wL (wL is the last line, wC is the last column of the
   window) the new text position becomes (l + 1, 0). If wraparound is
   set and c = wC, l = wL the window contents are scrolled up by one line and
   the new text position becomes (wL, 0). The following characters, defined
   in SITerminal, are interpreted and force special actions:
      EOL:      Move the current text position to (l + 1, 0). If the
                current position was on the bottom line of the window and
                wraparound is set, scroll window contents up by one line; in
                this case the new text position becomes (wL, 0).
      DEL:      The new text position becomes (l, c - 1). If wraparound is
                set and the current text position was at (l, 0) with l # 0,
                the new text position will be at (l - 1, wC). If wraparound
                is set and the current text position was at (0, 0), scroll
                window contents down by one line; in this case the new text
                position becomes (0, wC). In all these cases the character at
                the new position is erased.
      DelLine:  Erase current line. The new text position will be (l, 0).
      BS:       Move the current text position to (l, c -1). The effect of 
                wraparound is as with DEL.
      FF:       Erase window contents and set current text position to
                (0, 0). *)

PROCEDURE WriteString (w: Window; s: ARRAY OF CHAR);
(* is equivalent to
      FOR i := 0 TO StringLength(s) - 1 DO  Write(w, s[i])  END
   but possibly faster. *)

PROCEDURE WriteLn (w: Window);
(* is equivalent to Write (w, EOL). *)

PROCEDURE SetTerminalWindow (w: Window);
(* assigns the above three procedures to the output procedure variables
   of SITerminal. This causes all output of SITerminal and SINumberIO
   to go to window w. *)

PROCEDURE EraseToEOL (w: Window);
(* erases the display space between current position (l, c) and right hand
   border of the inner rectangle of the window. The current position is not
   modified. If the current position lies outside the inner rectangle of the
   window this procedure has no effect. *)

PROCEDURE Scroll (w: Window; by: INTEGER);
(* Scrolls the window contents upward if by > 0, otherwise scroll downward.
   by is in text lines. by = 0 has no effect. *)


END SISimpleText.
