
(***************************************************************************)
(***                                                                     ***)
(***                                                                     ***)
(***                        O  S  S  I                                   ***)
(***                        ==========                                   ***)
(***                                                                     ***)
(**)               DEFINITION MODULE SIFullScreen;                       (**)
(***               ==============================                        ***)
(***                                                                     ***)
(***   this module allows controlled access to alphanumeric              ***)
(***   terminals with full 2-dimensional cursor-positioning              ***)
(***   capabilities                                                      ***)
(***                                                                     ***)
(***---------------------------------------------------------------------***)
(***                                                                     ***)
(***   Hardware:             independent                                 ***)
(***   Operating System:     independent                                 ***)
(***   Compiler:             independent                                 ***)
(***                                                                     ***)
(***   Version:      4.0                                                 ***)
(***   Implemented:  see copyright                                       ***)
(***   Date:         1986-11-20                                          ***)
(***                                                                     ***)
(***---------------------------------------------------------------------***)
(***                                                                     ***)
(***   Copyright 1984, 1985, 1986 by                                     ***)
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

(* This module works on terminals that provide a text cursor which may
   be set under control of the program, and which determines where on
   the screen characters get displayed, as well as a visible cursor
   (possibly the same as the text cursor) which may be moved by the
   interactive user to indicate different positions on the screen. It
   must be possible to track or query the position of this cursor under
   control of the program.
   
   If more than one type of terminal may be used on the same machine,
   it must be possible for the machine to determine what type of terminal
   is being used.

   This module implements a virtual terminal. This includes providing
   the actual width and height, in columns and rows, of the terminal
   being used, as determined at run time. FullScreen also defines a
   coordinate system with origin in the upper left corner of the screen,
   corresponding to row 0 and line 0.

   This module also defines two different cursors, a user-input cursor
   (input cursor) and an output-position cursor (output cursor). The
   input cursor is guaranteed to be visible when the program is
   expecting an input from the user. The output cursor may or may not
   be visible. This cursor may be positioned outside the window: in
   this case output is suppressed. The input cursor is a marker on the
   screen which the end user can position (generally by using cursor
   keys).

   FullScreen provides output functions to write characters and
   strings, and special functions to scroll the screen and to
   clear the screen or a single line. The output functions are
   compatible with those defined in SITerminal, as are also the
   character read procedures. Input procedures also provide for
   event input, which lets the program wait until the interactive
   user has either pressed a key or moved the cursor on the screen. *)


EXPORT QUALIFIED
  InitScreen,
  ResetScreen,
  LinesOnScreen,
  CharsInLine,
  SetWrapAround,
  WrapAroundIsSet,
  SetHighLight,
  HighLightIsSet,
  SetOutputPos,
  GetOutputPos,
  Write,
  WriteString,
  WriteLn,
  ClearScreen,
  EraseLine,
  EraseToEOL,
  Scroll,
  WaitForEvent,
  SetInputPos,
  GetInputPos,
  Read,
  ReadString,
  ReadWithoutEcho,
  ReadAgain,
  Keypress,
  SetTerminal;


PROCEDURE InitScreen;
(* must be called before any other procedure from this module.
   initializes the terminal, which at the very least clears the
   screen, and depending on the system may set any necessary modes.
   This procedure turns off wrap-around and highlighting and does
   other system-dependent terminal initialization. *)

PROCEDURE ResetScreen;
(* must be called after all calls of procedures from this module
   are finished. Resets the terminal and, depending on the system,
   may do other things such as clearing the screen. *)

PROCEDURE LinesOnScreen (): CARDINAL;
(* returns the number of lines that can be written onto the screen. *)

PROCEDURE CharsInLine (): CARDINAL;
(* returns the number of characters that can be written onto one line of
   the screen. *)

PROCEDURE SetWrapAround (on: BOOLEAN);
(* sets wraparound if on is TRUE, otherwise wraparound is turned off. *)

PROCEDURE WrapAroundIsSet (): BOOLEAN;
(* returns whether wraparound is set. *)

PROCEDURE SetHighLight (on: BOOLEAN);
(* does system dependent highlighting of the characters written
   from now until highlighting is turned off. *)

PROCEDURE HighLightIsSet (): BOOLEAN;
(* returns whether highlighting is set. *)

PROCEDURE SetOutputPos (line, column: INTEGER);
(* sets the current output position in character units. *)

PROCEDURE GetOutputPos (VAR line, column: INTEGER);
(* returns the current output position in character units. *)


(* the following 3 procedures are equivalent to the same procedures in
   SITerminal. These procedures may be assigned to the procedure variables
   declared in SITerminal. *)

PROCEDURE Write (ch: CHAR);
(* writes ch at the current output position (l, c); the new output position
   becomes (l, c + 1). If wraparound is set and c = sC, l # sL (sL is the
   last line, sC is the last column of the screen) the new output position
   becomes (l + 1, 0). If wraparound is set and c = sC, l = sL the screen
   contents are scrolled up by one line and the new output position becomes
   (sL, 0). The following characters, defined in SITerminal, are interpreted
   and force special actions:
      EOL:      Move the current output position to (l + 1, 0). If the
                current position was on the bottom line of the screen and
                wraparound is set, scroll screen contents up by one line;
                in this case the new output position becomes (sL, 0).
      DEL:      The new output position becomes (l, c - 1). If wraparound
                is set and the current output position was at (l, 0) with
                l # 0, the new output position will be at (l - 1, sC). If
                wraparound is set and the current output position was at
                (0, 0), scroll screen contents down by one line; in this
                case the new output position becomes (0, sC). In all these
                cases the character at the new position is erased.
      DelLine:  Erase current line. The new output position will be (l, 0).
      BS:       Move the current output position to (l, c -1). The effect
                of wraparound is as with DEL.
      FF:       Erase screen contents and set current output position to
                (0, 0). *)

PROCEDURE WriteString (s: ARRAY OF CHAR);
(* is equivalent to
      FOR i := 0 TO StringLength(s) - 1 DO  Write (s[i])  END
   but possibly faster. *)

PROCEDURE WriteLn;
(* is equivalent to Write (EOL). *)

PROCEDURE ClearScreen;
(* is equivalent to Write (FF). *)

PROCEDURE EraseLine;
(* erases the current output line. *)

PROCEDURE EraseToEOL;
(* erases the display space between current position (l, c) and right
   edge of the screen. The current position is not modified. If the current
   position lies outside the screen this procedure has no effect. *)

PROCEDURE Scroll (by: INTEGER);
(* Scrolls the screen contents upward if by > 0, otherwise scroll downward.
   by is in text lines. by = 0 has no effect. *)


(* The input procedures follow. An event is either a cursor movement
   or a normal key press. When WaitForEvent returns, an event has
   occurred. If a normal key was pressed, Keypress will return TRUE;
   if the input cursor has been moved, the position will be different
   from what it was before. These two types of events are treated
   independently and orthogonally: a cursor key never gets 'read' and
   a character input does not change the current input cursor position.
   Characters entered by the user are echoed (if at all) at the current
   output position, which is thereby changed. *)

PROCEDURE WaitForEvent;
(* waits for either a key press or for the user to change the input
   position. *)

PROCEDURE SetInputPos (line, column: INTEGER);
(* sets the current input position in character units. *)

PROCEDURE GetInputPos (VAR line, column: INTEGER);
(* returns the current input position in character units. *)

(* The following 5 procedures are equivalent to the same procedures in
   SITerminal. These procedures may be assigned to the procedure variables
   declared in SITerminal. The function of each of these procedures
   is described in SITerminal. *)

PROCEDURE Read (): CHAR;
PROCEDURE ReadString (VAR s: ARRAY OF CHAR);
PROCEDURE ReadWithoutEcho (): CHAR;
PROCEDURE ReadAgain ();
PROCEDURE Keypress (): BOOLEAN;


PROCEDURE SetTerminal;
(* assigns the procedures
        Write, WriteString, WriteLn, Read, ReadString,
        ReadWithoutEcho, ReadAgain, Keypress;
   to the corresponding procedure variables of SITerminal. *)
 

END SIFullScreen.
