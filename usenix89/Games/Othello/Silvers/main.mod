MODULE main;

(*******************************************************)
(* OTHELLO  (c) 1987 Robert Silvers.                   *)
(* University of Lowell.                               *) 
(*                                                     *) 
(* "It was hard to write, so it should be hard to read"*)
(*                                                     *) 
(*                                                     *)
(* VERSION HISTORY:                                    *)
(*                                                     *)
(*                  Started at end of spring break.    *)
(* Ver. 1.0 4/04/87 First totally working version.     *)
(* Ver. 1.1 4/07/87 Computer plays person grabbing     *)
(*                  as many pieces as it can.          *)
(*                  Procedure that shows moves is      *)
(*                  working.                           *)
(* Ver. 1.2 4/08/87 Recognizes end of game.  Tells     *)
(*                  who won.                           *)
(* Ver. 1.3 4/09/87 Goes for corners, sides, etc.      *)
(*          4/11/87 All known bugs removed.            *)
(*          4/11/27 Started Added an extra module.     *)
(*          4/17/27 Finished adding extra module.      *)
(* Ver. 1.4 4/17/87 Added one move look-ahead.         *)
(* Ver. 1.5 4/17/87 Added "SmartFlip" routine.         *)
(* Ver. 1.6 4/19/87 New move algorithm good opponent.  *)
(* Ver. 1.7         Remote Player version.             *)
(*                                                     *)
(*******************************************************)


FROM InOut   IMPORT
   WriteString;

FROM types   IMPORT
   PIECES, BOARD, TEMP, field, temp, flipped, Passed, End;

FROM othello IMPORT
   InitBoard, Flip, DrawBoard, ReDraw, Menu, GameOver;

FROM move    IMPORT
   GetMove, Move;

FROM regis   IMPORT
   ExitGr;

FROM TextIO  IMPORT
   ReadCHAR, WriteCHAR;


FROM Goto    IMPORT
   SetGoto;


VAR
   choice: CHAR; (* Number of players selected *)
   PLAYED: BOOLEAN; (* TRUE if games has started. *)


(* The user has selected a two player game.    *)
PROCEDURE TwoPlayer;

VAR 
   xval  : INTEGER; (* Coords of move.         *)
   yval  : INTEGER;
   player: PIECES ; (* Color now moving.       *)

BEGIN

   PLAYED:= TRUE;
   player:= black;  (* Black starts.           *)
   DrawBoard;       (* Set up graphics.        *)
   InitBoard;       (* Put first four chips on.*)
   ReDraw;          (* Draw the chips.         *)
   REPEAT

      IF player = white THEN            (* Find opposite player. *) 
         player:= black
      ELSE
         player:= white;
      END; 

      GetMove(player, xval, yval); 
      IF NOT Passed THEN
         Flip(player, xval, yval); (* Flip applicable pieces.    *)
      END;
      ReDraw;

   UNTIL GameOver(FALSE); (* False means that it is not a player *)
			  (* Vs. computer game.                  *)
END TwoPlayer;            (* Used to print surprise message if   *)
			  (* TRUE and computer wins.             *)

PROCEDURE OnePlayer;

VAR 
   xval  : INTEGER;       (* Coords of move.       *)
   yval  : INTEGER;
   player: PIECES ;

BEGIN

   PLAYED:= TRUE;
   Passed:= FALSE;        (* Initialize.           *)
   DrawBoard;
   InitBoard;
   ReDraw;
   REPEAT

      GetMove  (white, xval, yval);
      IF NOT Passed THEN
         Flip(white, xval, yval);
      ReDraw;
      END; 
      Move     (black, xval, yval);    (* Computer moves. *)
      IF NOT Passed THEN
         Flip(black, xval, yval);
         ReDraw
      END; 

   UNTIL GameOver(TRUE); (* TRUE means computer did play. *)
			 (* If computer wins, an insult   *)
			 (* will reusult.                 *)

END OnePlayer;


PROCEDURE NoPlayer; (* Computer plays with itself.        *)

VAR 
   xval  : INTEGER; (* Coords of move.                    *)
   yval  : INTEGER;
   player: PIECES ;

BEGIN

   PLAYED:= TRUE;
   Passed:= FALSE;  (* Initialize. *)
   player:= black;
   DrawBoard;
   InitBoard;
   ReDraw;
   REPEAT

      IF player = white THEN
         player:= black
      ELSE
         player:= white;
      END;
      Move(player, xval, yval);        (* Computer move. *)
      IF NOT Passed THEN
         Flip(player, xval, yval);
         ReDraw;
      END; 

   UNTIL GameOver(FALSE);

END NoPlayer;



BEGIN             (* Main Line *)

   PLAYED:= FALSE;
   Menu(choice);  (* Get the number of players. *)

   SetGoto(End);

(* Check if game has been played yet.  If it has, then we have just *)
(* returned from the goto and we should skip the CASE.              *)
IF NOT PLAYED THEN
   CASE choice OF
      '0' : NoPlayer;
     |'1' : OnePlayer;
     |'2' : TwoPlayer;
     |'q' : (* Quit *);
   END; 
END; 
ExitGr;

END main.



