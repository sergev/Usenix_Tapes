IMPLEMENTATION MODULE othello;

(*************************************************************************)
(* OTHELLO.  (c) 1987 Robert Silvers.  All rights reserved.              *)
(*                                                                       *)
(* This is the main set of routines that runs the othello game.  The     *)
(* game is controlled from the driver.  The procedures in this section   *)
(* set up the board, get the moves, check if it is valid, if so, flip    *)
(* the pieces.  It then counts the pieces, and redraws the board.        *)
(*************************************************************************)

FROM types           IMPORT
   PIECES, BOARD, TEMP, field, temp, flipped, Passed;

FROM InOut           IMPORT
   Read, WriteInt;

FROM InOutExtensions IMPORT
   GetEscapeSequence, GetOneChar, ReadLn;

FROM regis           IMPORT
   Reset, Position, ClearScreen, Plot, DrawTo, Circle, Box, Color, SetFill,
   TextSize, WriteNum, WriteText, UnsetFill, BGColor, TextSlant, WriteChr,
   Scroll, TextDirection;

FROM move            IMPORT
   CanPass, Validate, Count, HowMany;

(*************************************************************************)
(* This function gets the game choice with the opening menu.             *)
(* RETURNS the number of players.                                        *)
(*************************************************************************)
PROCEDURE Menu(VAR choice: CHAR);

VAR 
   x: INTEGER;

BEGIN

   choice:= ' ';

   ClearScreen;

   (* (* Alternative opening screen *)
   TextSlant(-1);
   Color("R");
   FOR x:= 1 TO 10 BY 2 DO
      Position(80 + (x * 15), x * x * 3); 
      TextSize(x);
      WriteText("OTHELLO");
   END;
   TextSlant(0);
   *)

   Position(80, 0); (* Draw word OTHELLO 3 times. *)
   TextSize(10);
   Color("R");
   WriteText("OTHELLO");

   Position(80, 120);
   TextSize(10);
   Color("G");
   WriteText("OTHELLO");

   Position(80, 240);
   TextSize(10);
   Color("B");
   WriteText("OTHELLO");

   Position(80, 400);
   Color("G");
   TextSize(2);
   WriteText("Enter the number of players (0-2) : ");

   (* Get the number of players. *)
   REPEAT
      Read(choice);
   UNTIL ((choice >= '0') AND (choice <= '2')) OR (choice= 'q');

END Menu;


(*************************************************************************)
(* This procedure sets all of the places to empty, except the center     *)
(* four.                                                                 *)
(*************************************************************************)
PROCEDURE InitBoard();

VAR x, y: INTEGER;(* Position on the surface.*)

BEGIN

   FOR x:= 1 TO 8 DO    (* Set all spaces to empty. *)
      FOR y:= 1 TO 8 DO
	 field[x][y]  := none;
	 flipped[x][y]:= FALSE;
      END;
   END; 

   field   [4][4]:= white; (* Put in first four pieces. *)
   field   [5][4]:= black;
   field   [4][5]:= black;
   field   [5][5]:= white;

   flipped [4][4]:= TRUE;  (* Turn on flipped flag.     *)
   flipped [5][4]:= TRUE;
   flipped [4][5]:= TRUE;
   flipped [5][5]:= TRUE;

END InitBoard; 


(*************************************************************************)
(* This procedure draws the opening graphics.                            *)
(*************************************************************************)
PROCEDURE DrawBoard;

VAR
   x, y: INTEGER;

BEGIN

   Reset;            (* Set all to default.     *)
   BGColor('D');     (* Set background to black.*) 
   ClearScreen;

   Color('B');       (* Draw blue border.       *)
   Position(0, 0);
   SetFill(150);  
   Box(410, 411);
   UnsetFill;

   Color('G');       (* Draw green background.  *)
   Position(10, 9);
   SetFill(150);     
   Box(390,391);
   UnsetFill;

   Color("D"); (* A touch of shadow. *)
   Plot(9, 402);
   DrawTo(399, 402);
   Plot(9, 10);
   DrawTo(9, 402);
   

   Color('D');       (* Draw black lines.*)
   FOR x:= 55 TO 400 BY 50 DO
      Plot(x, 9);
      DrawTo(x, 400);
      Plot(409 - x , 9);
      DrawTo(409 - x, 400);
      Plot(9, x);
      DrawTo(400, x);
      Plot(9, 409 - x);
      DrawTo(400, 409-  x);
   END; (* FOR *)

   Position(419, 3); (* Write OTHELLO    *)
   TextSize(5);
   Color('B');
   WriteText('OTHELLO');
   Position(421, 1); 
   TextSize(5);
   Color('G');
   WriteText('OTHELLO');
   Position(422, 0); 
   TextSize(5);
   Color('R');
   WriteText('OTHELLO');

   TextSize(1);      (* Give credit where credit is due. *)
   Color('B');
   Position(646, 64);
   TextSlant(-10); 
   WriteText('By Robert Silvers');
   TextSlant(0); 

   Color('R');       (* Write out score board. *)
   Position(420, 90);
   WriteText('Blue: ');
   Position(512, 90);
   WriteText('Black: ');

   Color('B');       (* Give basic instructions. *)
   Position(420, 350);
   WriteText('Use arrow keys to move.  RETURN to enter.');   
   Color('B'); 
   Position(420, 370);
   WriteText('1 drops a blue, 2 a black, 3 to remove.');
   Color('B'); (* Blue. *)
   Position(420, 390);
   WriteText('Press S to show moves.  P to pass.');

END DrawBoard;


(*************************************************************************)
(* This procedure flips the pieces for the player after a move.          *)
(*************************************************************************)
PROCEDURE Flip (player : PIECES; xcords, ycords: INTEGER);

BEGIN

   Flipper(player, xcords, ycords, 1, 0);
   Flipper(player, xcords, ycords, -1, 0);
   Flipper(player, xcords, ycords, 0, 1);
   Flipper(player, xcords, ycords, 0, -1);
   Flipper(player, xcords, ycords, 1, -1);
   Flipper(player, xcords, ycords, 1, 1);
   Flipper(player, xcords, ycords, -1, 1);
   Flipper(player, xcords, ycords, -1, -1);

END Flip; 


PROCEDURE Flipper(player: PIECES; xcords, ycords, xdir, ydir: INTEGER);

VAR
   opplayer: PIECES ; (* The opposite player.                 *)
   x       : INTEGER; (* Used for loop counters.              *)
   y       : INTEGER;
   foundop : BOOLEAN; (* Found opposite player.               *)
   canflip : BOOLEAN; (* TRUE is at least one flip is needed. *)
   founds  : BOOLEAN; (* Found space.                         *) 
   foundp  : BOOLEAN; (* Found player.                        *) 

BEGIN

   IF player= white THEN     (* Get the other player's color. *)
      opplayer:= black
   ELSE
      opplayer:= white;   
   END; 

   foundop:= FALSE; (* Flip from right to left and up. *)
   canflip:= FALSE;
   founds := FALSE;
   foundp := FALSE;
   x:= xcords;
   y:= ycords;
   LOOP
      IF (x= 0) OR (y= 0) OR (x= 9) OR (y= 9) THEN
	 EXIT;
      END; (* IF *)
      IF (field[x][y]= opplayer) THEN
	  foundop:= TRUE;
      END; (* IF *);
      IF ((field[x][y]= none)) AND ((foundop= FALSE) OR
          (foundp= FALSE)) THEN
	   founds:= TRUE;
      END; (* IF *);
      IF (field[x][y]= player) AND (foundop) AND (founds= FALSE) THEN
	  canflip:= TRUE;
      END; (* IF *);
      x:= x + xdir;
      y:= y + ydir;
   END; (* LOOP *)
   x:= xcords;
   y:= ycords;
   IF canflip THEN
      LOOP
         x:= x + xdir;
         y:= y + ydir;
      IF (x= 0) OR (y= 0) OR (x= 9) OR (y= 9) THEN
	    EXIT;
         END; (* IF *)
         IF (field[x][y]= player) THEN
	    EXIT;
         END; (* IF *);
         IF (field[x][y]= opplayer) THEN
	    field[x][y]:= player;
	    flipped[x][y]:= TRUE;
         END; (* IF *);
      END; (* LOOP *)
   END; (* IF *)

END Flipper;


(*************************************************************************)
(* This procedure updates the board after a move.                        *)
(* It used a boolean field to only ReDraw where a piece has been changed.*)
(*************************************************************************)
PROCEDURE ReDraw;

VAR
   x   : INTEGER; (* Coordinates. *)
   y   : INTEGER;       
   wnum: INTEGER; (* Number of each player on board. *)
   bnum: INTEGER;    

BEGIN

   Count(wnum, bnum); (* Update the number of pieces on the screen. *)

   Position(470, 90); (* Draw new number of pieces.                 *)
   Color('D') ;
   SetFill(90);
   Box(20, 15);
   UnsetFill  ;
   Color('R') ; 
   WriteNum(wnum);

   Position(570, 90);
   Color('D') ;
   SetFill(90);
   Box(20, 15);
   UnsetFill  ;
   Color('R') ; 
   WriteNum(bnum);

   FOR y:= 1 TO 8 DO; (* Go to every square and flip if TRUE.        *)
      FOR x:= 1 TO 8 DO;
	 IF flipped[x][y]  = TRUE  THEN (* If the piece has changed. *)
	    IF field[x][y] = black THEN (* Set color of piece.       *)
	       Color('D')                                  (* Black. *)
            ELSIF field[x][y] = white THEN
	       Color('B')                                  (* Red.   *)
            ELSE                               (* field[x][y]:= none *)
	       Color('G');                                 (* Green. *)
            END; (* IF *)
	    Position((x * 50) - 20, (y * 50) - 20); (* Draw circle.  *)
	    SetFill ((y * 50) - 20);
	    Circle(13, 13);
	    UnsetFill;
	    flipped[x][y]:= FALSE; (* Reset to FALSE *)
         END; (* IF *)
      END; (* FOR *)
   END; (* FOR *)

END ReDraw;


(* Shows what moves are open to player.  Tells them how many pieces *)
(* they would get if they move there also.                          *)
PROCEDURE ShowMoves(player: PIECES);

VAR
   x, y  : INTEGER;
   number: INTEGER; (* Number of flips possible *)
   dummy : INTEGER; (* Absorbs extra parameter. *)

BEGIN

   TextSize(1);
   FOR y:= 1 TO 8 DO
      FOR x:= 1 TO 8 DO
         IF Validate(player, x, y) THEN
	    Color("R");
	    Position((x * 50) -20, (y * 50) - 20);
	    SetFill ((y * 50) -20);
	    Circle(7, 7);         (* Draws red circle where valid. *)
	    UnsetFill;
	    Color("D");
	    HowMany(player, x, y, number, dummy); (* Get number of flips *)
	    Position((x * 50) -25, (y * 50) - 28);
	    WriteNum(number);          (* Display number of flips. *)
	    flipped[x][y]:= TRUE;
         END; (* IF *)
      END; (* FOR *)
   END; (* FOR *)

END ShowMoves;


(* Returns TRUE if the game is over.       *)
PROCEDURE GameOver (computer: BOOLEAN): BOOLEAN;

VAR
   whitenum, blacknum: INTEGER; (* Used to see who won. *)
   
BEGIN

   (* Game is over if both players have no moves. *)
   IF CanPass(black) AND CanPass(white) THEN
      Position(420, 150);
      Color("G");
      TextSize(3);
      WriteText("Game Over.");
      TextSize(2);
      Count(whitenum, blacknum);
      IF whitenum > blacknum THEN
	 Position(420, 200);
	 Color("R");
	 WriteText("Blue wins.")
      ELSIF blacknum > whitenum THEN
	 Position(420, 200);
	 Color("R");
	 WriteText("Black wins.");
	 IF computer THEN (* Computer played in game. *)
	    Color("B");
	    TextSize(1);
	    Position(420, 240);
	    WriteText("Artificial intelligence is better");
	    Position(420, 260);
	    WriteText("than none at all...");

	    Color("R"); (* Redraw with different color. *)
	    Position(421, 239);
	    WriteText("Artificial intelligence is better");
	    Position(421, 259);
	    WriteText("than none at all...");
         END (* IF *)
      ELSE
	 Position(420, 200);
	 Color("R");
	 WriteText("Its a tie!")
      END; (* IF *)
      Position(180, 180);
      RETURN TRUE   (* Game over.     *)
   END; (* IF *)

   RETURN FALSE;    (* Game not over. *)

END GameOver;


END othello.



