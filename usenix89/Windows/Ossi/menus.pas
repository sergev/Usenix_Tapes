
(***************************************************************************)
(***                                                                     ***)
(***                                                                     ***)
(***                     O  S  S  I                                      ***)
(***                     ==========                                      ***)
(***                                                                     ***)
(**)               DEFINITION MODULE SIMenus;                            (**)
(***               =========================                             ***)
(***                                                                     ***)
(***   This module supports interactive selections from menus            ***)
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

(* A menu lets the interactive user select from a number of items. There
   may be a number of menus, several of which may be active. A menu is
   active if it has been activated for the currently active window.
   The active window is specified according to system conventions
   whenever possible. The user can always choose which menu to select
   from. There is in other words a two-level hierarchy: menus on the
   upper level and menu items on the lower level.

   A menu selection can occur if the event menuSelect (SIEvents) has
   been enabled. If the user attempts to select from a menu, the system
   displays the appropriate information. If the selection has been
   completed successfully, the event menuSelect is generated and with
   this event the reference to the selected item is returned. *)


FROM SISystem IMPORT
   SIResult;

FROM SIWindows IMPORT
   Window;


EXPORT QUALIFIED
   Menu,
   MenuItem,
   CreateMenu,
   DisposeMenu,
   AddItem,
   RemoveItem,
   ActivateMenu,
   DeactivateMenu;


TYPE Menu;

     MenuItem;


PROCEDURE CreateMenu (title: ARRAY OF CHAR; VAR menu: Menu;
                      VAR result: SIResult);
(* creates a menu with the given title. menu is an identifier by which the
   menu can be referenced later on. result will be different from SIDone if
   no menu can be created. *)
    
PROCEDURE DisposeMenu (VAR menu: Menu);
(* prevents menu from being selectable any more. If the menu is still
   active for any window it will be deactivated. menu is set to an invalid
   value. *)

PROCEDURE AddItem (menu: Menu; itemName: ARRAY OF CHAR; reference: CARDINAL;
                   VAR item: MenuItem; VAR result: SIResult);
(* adds an item itemName to menu and returns the identifier item, used only
   for RemoveItem. When a menuSelect occurs, the reference will be returned.
   result will be different from SIDone if no more items can be added to the
   menu. *)

PROCEDURE RemoveItem (menu: Menu; VAR item: MenuItem);
(* removes item from the menu. The value returned for item is invalid. *)

PROCEDURE ActivateMenu (w: Window; menu: Menu);
(* makes menu available to the interactive user if w is an active
   window. *)

PROCEDURE DeactivateMenu (w: Window; menu: Menu);
(* makes the menu unavailable when w is an active window. The menu will
   still be available to the user if there are other active windows for
   which this menu has been activated explicitly. *)


END SIMenus.
