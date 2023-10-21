
(***************************************************************************)
(***                                                                     ***)
(***                                                                     ***)
(***                     O  S  S  I                                      ***)
(***                     ==========                                      ***)
(***                                                                     ***)
(**)               DEFINITION MODULE SIEvents;                           (**)
(***               ==========================                            ***)
(***                                                                     ***)
(***   This module handles cursors bound to a pointing device            ***)
(***   using events                                                      ***)
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

(* The cursor is a small pattern that is moved around on the screen.
   It is bound to some pointing device (mouse, lightpen, tablet, or even 
   cursor keys) and is completely under control of the user, that is, the
   programmer cannot decide where to place it. The cursor may be visible
   or hidden. Tracking the cursor is done by the system. The user may
   select a point on the screen, select from a menu etc. by means of
   buttons connected with the pointing device (which may be simulated by
   special keys on the keyboard). The way this is done should follow host
   system conventions.

   Keyboards normally provide alphabetic (ASCII) keys and usually also some
   function keys. The two kinds of key produce two different kinds of event
   when pressed. Function keys are numbered in a system dependent way; the
   meaning of each function key is not standard. There may be some system-
   dependent module which on each system defines the meanings of the
   function keys. In other words, the use of function keys will almost
   certainly make programs nonportable. Function keys are only included
   here so all input is conveniently available in one module. Their use is
   discouraged.

   Events are caused by user actions. The application program can request
   the next event, if any. Events can be disabled, that is they are
   prevented from occurring. *)


FROM SISystem IMPORT
   SIResult;
  
FROM SIWindows IMPORT
   ScrollPos,
   Window,
   PaintMode;


EXPORT QUALIFIED
   Event,
   Events,
   EventDescriptor,
   EnableEvents,
   GetEnabledEvents,
   WaitForEvent,
   GetEvent,
   GetCursorPos,
   HideCursor,
   ShowCursor,
   CursorIsVisible;


TYPE Event = (nullEvent,        (* nothing has happened *)
              startSelect,      (* selection has been started *)
              endSelect,        (* selection has been finished *)
              cancelSelect,     (* selection has been canceled *)
              menuSelect,       (* menu selection has occurred*)
              enterChar,        (* char has been entered from the keyboard *)
              functionKey,      (* function key has been pressed *)
              horScroll,        (* horizontal scroll has occurred *)
              verScroll,        (* vertical scroll has occurred *)
              deleteWindow,     (* window has been deleted *)
              openWindow,       (* window has been opened *)
              closeWindow,      (* window has been closed *)
              moveWindow,       (* window has been moved *)
              changeWindow);    (* window has been changed *)

     Events = SET OF Event;

     EventDescriptor = RECORD
                         CASE kind: Event OF
                           startSelect,
                           endSelect,
                           menuSelect,
                           enterChar,
                           functionKey,
                           horScroll,
                           verScroll,
                           deleteWindow,
                           openWindow,
                           closeWindow,
                           moveWindow,
                           changeWindow:
                             w: Window;
                         ELSE
                           (* nullEvent and cancelSelect return no window *)
                         END (* CASE : Event OF *);
                         CASE : Event OF
                           startSelect,
                           endSelect:
                             h, v: INTEGER;
                         | menuSelect:
                             reference: CARDINAL;
                         | enterChar:
                             ch: CHAR;
                         | functionKey:
                             keyNumber: CARDINAL;
                         | horScroll,
                           verScroll:
                             scrollPos: ScrollPos;
                         | changeWindow:
                              newWidth,
                              newHeight: CARDINAL;
                         ELSE
                         END (* CASE : Event OF *);
                       END (* EventDescriptor *);


     (* Each selection by the user is considered to consist of two events,
        a startSelect (e.g. a button press) and an endSelect (e.g. a button
        release). An endSelect must follow directly a startSelect: a
        cancelSelect is generated if the user performs an action different
        from endSelect (e.g. typing a character) after a startSelect.
        An endSelect performed by the user outside all windows is
        not returned as an event. An endSelect performed by the user without
        a previous startSelect is ignored and not passed as an event.
        In general, if the cursor coordinates of the startSelect and the
        endSelect are the same, a single point on the screen has been
        selected. The startSelect and endSelect events can be used among
        other things to select a rectangular area (the application program
        can drag a rectangle between the occurrences of the startSelect and
        the endSelect events) or to drag an object from a given position
        (which is selected by startSelect) to another position (which is
        determined by endSelect). This dragging can be done by calling the
        procedure GetCursorPos to get the latest cursor position.

        Most of the events (except nullEvent and cancelSelect) return the
        window which was active when the event occurred. In some cases (see
        GetEvent) an action is performed on these windows when GetEvent is
        called.

        The reference to the menu item is returned for a menuSelect event.

        If a moveWindow has occurred the new window position can
        be obtained by calling SIWindows.GetOuterRectangle.

        In the case of a changeWindow event the size and the location of the
        window may have changed. The event record only returns the new size
        of the inner rectangle since this will be sufficient in most cases.
        The position of the window can be obtained by calling
        SIWindows.GetOuterRectangle. *)
        

PROCEDURE EnableEvents (w: Window; eventSet: Events);
(* enables eventSet events for the given window. Initially, all events
   (except the nullEvent) are disabled; the nullEvent is always enabled even
   if it is not included in eventSet. In order for the interactive user to be
   able to use the pointing device, some of the events must be enabled. Any
   events enabled before the call but not included in eventSet are disabled. *)

PROCEDURE GetEnabledEvents (w: Window): Events;
(* returns the events enabled for the given window. *)

PROCEDURE WaitForEvent ();
(* waits for an enabled event (other than nullEvent). It does not check
   whether any events are enabled. If no events are enabled for any window
   this procedure will never return! *)

PROCEDURE GetEvent (VAR event: EventDescriptor);
(* returns the latest event, or nullEvent if nothing happened. Buffering of
   events is system dependent; if a new event happens before the previous
   event was requested by a call to GetEvent, either the old or the new event
   may be lost. If the event is a horScroll, verScroll, deleteWindow,
   openWindow, closeWindow, moveWindow or changeWindow, the corresponding
   action (e.g. deleting or changing a window) is performed automatically
   by this module during the call to GetEvent but not before. *)

PROCEDURE GetCursorPos (VAR inWindow, inInnerRectangle: BOOLEAN;
                        VAR w: Window; VAR h, v: INTEGER);
(* returns whether the cursor is contained in a window, without waiting for
   an event to occur. If inWindow and inInnerRectangle are both TRUE, the
   window the cursor is currently in and the cursor position within this
   window (in window coordinates) are returned. If inInnerRectangle is FALSE
   and inWindow is TRUE, the correct window is returned but the coordinates
   will be (0, 0). If the underlying system does not allow to access the
   cursor at this particular time, the last valid position is returned. *)

PROCEDURE HideCursor (w: Window);
(* makes the system cursor invisible if it moves into the inner rectangle of
   window w. *)

PROCEDURE ShowCursor (w: Window);
(* makes the system cursor visible if it is moved into the inner rectangle of
   window w. *)

PROCEDURE CursorIsVisible (w: Window): BOOLEAN;
(* returns TRUE iff the cursor will be visible if it is moved into the inner
   rectangle of window w. *)

END SIEvents.
