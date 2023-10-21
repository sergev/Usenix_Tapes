IMPLEMENTATION MODULE move;

(*************************************************************************)
(* OTHELLO.  (c) 1987 Robert Silvers.  All rights reserved.              *)
(*                                                                       *)
(* This is the main set of routines that runs the othello game.  The     *)
(* game is controlled from the driver.  The procedures in this section   *)
(* set up the board, get the moves, check if it is valid, if so, flip    *)
(* the pieces.  It then counts the pieces, and redraws the board.        *)
(*************************************************************************)


FROM InOut   IMPORT
   WriteString, Read, WriteInt;

FROM InOutExtensions IMPORT
   GetEscapeSequence, GetOneChar, ReadLn;

FROM regis   IMPORT
   Reset, Position, ClearScreen, Plot, DrawTo, Circle, Box, Color, SetFill,
   TextSize, WriteNum, WriteText, UnsetFill, BGColor, TextSlant;

FROM othello IMPORT
   Flip, ShowMoves, GameOver;

FROM types   IMPORT
   PIECES, BOARD, TEMP, field, temp, flipped, Passed, End;

FROM Goto    IMPORT
   LongGoto;


(* NOTE: The constants below are for adjusting the way the game plays. *)
(* The computer uses them to decide how much to weigh each move.       *)

(* Constants in HowMany *)

CONST
   EDGEFLIP= 03;(* When it flips an edge, it adds this number to value.*)
   TWOAWF  = 07;(* When it flips a piece two away from corner, adds it.*)

(* In Value *)

   CORNER  = 25;(* Add this when a move can be made in the corner.     *)
   EDGE    = 05;(* Add this when you can flip on an edge.              *)
   ADJACENT= 15;(* Subtract this when you can move next to a corner.   *)
		(* Note: not subtracted if corner already occupied.    *)
   TWOAW   = 10;(* Add this if move is two away from corner.           *)

(* In FindBest *)

   MULF    = 03;(* Multiply this by computer's move weight before      *)
		(* subtracting humans supposed (look-ahead) move.      *)

VAR
   Look: BOOLEAN;

(*************************************************************************)
(* This procedure gets the move from the player.                         *)
(*************************************************************************)
PROCEDURE GetMove (player: PIECES; VAR xcords, ycords: INTEGER);

VAR 
   ch         : ARRAY    [0..2] OF CHAR;
   passed     : BOOLEAN; (* TRUE if player passed.               *)
   illeg      : BOOLEAN; (* TRUE if illegal move.                *)
   TriedToPass: BOOLEAN; (* TRUE if message needs to be cleared. *)
   Delay      : INTEGER;

BEGIN
   xcords:= 4;       (* Set to middle position of board. *)
   ycords:= 4;       (* Initialize. *)
   passed:= FALSE;
   illeg := FALSE;
   TriedToPass := FALSE;

   IF player= white THEN  (* Print out whose move it is. *)
      Color('D');         (* But first, erase previous.  *)
      Position(420, 120);
      WriteText('Blacks move.');
      Color("R");
      Position(420, 120);
      WriteText('Blues move.');
      Position(180, 180);
   ELSE
      Color('D');
      Position(420, 120);
      WriteText('Blues move.');
      Color("R");
      Position(420, 120);
      WriteText('Blacks move.');
      Position(180, 180);
   END; (* IF *);

   LOOP

      LOOP

         Position((xcords * 50) - 20, (ycords * 50) - 20);
	 GetEscapeSequence(ch);         (* Read keyboard. *)

	 IF illeg THEN (* Erase old message if nessesary. *)
	    illeg:= FALSE;
            Color('D');
            TextSize(2);
            Position(420, 150); 
            WriteText('ILLEGAL MOVE!');
	    TextSize(1);
         END; (* IF *)
	 IF TriedToPass THEN (* Erase message if TRUE.    *)
	    TriedToPass:= FALSE;
	    Color("D");
	    Position(420, 150);
	    WriteText('Sorry, you cannot pass.');
         END; (* IF *)

         IF   (ch[2]= 'D') AND (xcords > 1) THEN (* Get the moves. *)
	    DEC(xcords);
         ELSIF(ch[2]= 'C') AND (xcords < 8) THEN
	    INC(xcords);
         ELSIF(ch[2]= 'B') AND (ycords < 8) THEN
	    INC(ycords);
         ELSIF(ch[2]= 'A') AND (ycords > 1) THEN
	    DEC(ycords);
         ELSIF(ch[0]= 's') OR  (ch[0]= 'S') THEN
	    ShowMoves(player)
         ELSIF(ch[0]= 'p') OR  (ch[0]= 'P') THEN
	    IF CanPass(player) THEN
	       passed:= TRUE;  (* Display passed and exit. *)
	       Passed:= TRUE;
	       Position(420, 150);
	       Color("G");
	       WriteText('PASSED');
               Position((xcords * 50) - 20, (ycords * 50) - 20);
	       FOR Delay:= 1 TO 300000 DO
	       END;
	       Position(420, 150);
	       Color("D");
	       WriteText('PASSED');
	       EXIT; (* Leave first loop *)
            END; (* IF *)
	    Color("G");
	    Position(420, 150);
	    WriteText('Sorry, you cannot pass.');
            Position((xcords * 50) - 20, (ycords * 50) - 20);
	    TriedToPass:= TRUE;
         ELSIF(ch[0]= '1') THEN (* Drop a white piece. *)
	    field[xcords][ycords]:= white;
	    Color("B");
	    SetFill(ycords);
	    Circle(13, 13);
	    UnsetFill
         ELSIF(ch[0]= '2') THEN (* Drop a black piece *)
	    field[xcords][ycords]:= black;
	    Color("D");
	    SetFill(ycords);
	    Circle(13, 13);
	    UnsetFill
         ELSIF(ch[0]= '3') THEN (* Clear the space. *)
	    field[xcords][ycords]:= none;
	    Color("G");
	    SetFill(ycords);
	    Circle(13, 13);
	    UnsetFill
         ELSIF ch[0]= 'q' THEN (* Quit. *)
	    LongGoto(End); (* Jump to end of program. *)
         ELSIF ch[0]= 12C (*RETURN*) THEN  (* Move was entered. *)
            EXIT; (* Lets get outta' here. *)
         END; (* IF *)

      END; (* INNER LOOP *)

      IF passed THEN (* If no valid moves and person has passed. *)
	 EXIT;
      END; 
      IF Validate(player, xcords, ycords) THEN     (* Good move. *)
	 EXIT;
      END; 

      Color('G'); (* If it has gotton this far, must be bad move. *)
      TextSize(2);
      Position(420, 150);    (* Print out that move is not valid. *)
      WriteText('ILLEGAL MOVE!');
      illeg:= TRUE;
      Position((xcords * 50) - 20, (ycords * 50) - 20);
      TextSize(1);

   END; (* outer LOOP *)
   IF NOT passed THEN (* Good move. *)
      Passed:= FALSE;
      field[xcords][ycords]:= player;
      Position((xcords * 50) - 20, (ycords * 50) - 20);
      IF player= white THEN (* Find color.  Put piece on screen. *)
         Color('B')
      ELSE
         Color('D');
      END; (* IF *)
      SetFill((ycords * 50) - 20);
      Circle(13, 13);
      UnsetFill;
   END; (* IF *)

END GetMove;


(*************************************************************************)
(* This function reports if a move is valid for the player.              *)
(*************************************************************************)
PROCEDURE Validate (player: PIECES; xcords, ycords: INTEGER): BOOLEAN;           
BEGIN


   (* Reject right away if space is occupied. *)
   IF (field[xcords][ycords]= white) OR (field[xcords][ycords]= black) THEN
      RETURN FALSE;
   END; (* IF *)


   IF Check(player, xcords, ycords, 1, 0)  OR
      Check(player, xcords, ycords, -1, 0) OR
      Check(player, xcords, ycords, 0, 1)  OR
      Check(player, xcords, ycords, 0, -1) OR
      Check(player, xcords, ycords, 1, -1) OR
      Check(player, xcords, ycords, 1, 1)  OR
      Check(player, xcords, ycords, -1, 1) OR
      Check(player, xcords, ycords, -1,-1) THEN
         RETURN TRUE;
   END;

   RETURN FALSE;

END Validate;

(* Scans in the 8 directions called in Validate. *)
PROCEDURE Check(player: PIECES; xcords, ycords, xdirection, ydirection:INTEGER):
                BOOLEAN;

VAR
   x, y    : INTEGER; (* Screen coordinates.     *)
   opplayer: PIECES ; (* Opponent.               *)
   found   : BOOLEAN; (* TRUE if other is found. *)

BEGIN

   IF player= white THEN                (* Get the other player's color. *)
      opplayer:= black
   ELSE
      opplayer:= white;   
   END; (* IF *);

   found:= FALSE;
   x:= xcords + xdirection;
   y:= ycords + ydirection;

   LOOP
      IF (y= 0) OR (x= 0) OR (y= 9) OR (x= 9) THEN
	 EXIT;
      END; 
      IF field[x][y]= none THEN
	 EXIT;
      END; 
      IF (field[x][y]= opplayer) THEN
      	 found:= TRUE;
      END; 
      IF (found= TRUE) AND (field[x][y]= player) THEN
      	 RETURN  TRUE;
      END; 
      IF field[x][y]= player THEN
	 EXIT;
      END; 
      x:= x + xdirection;
      y:= y + ydirection;
   END; (* LOOP *);
   RETURN FALSE;

END Check;


(*************************************************************************)
(* This procedure counts the number of pieces on the board.              *)
(* It is called from the redraw procedure where it is displayed.         *)
(*************************************************************************)
PROCEDURE Count (VAR White, Black: INTEGER);

VAR
   x, y: INTEGER;

BEGIN

   White:= 0; (* Init. *)
   Black:= 0;
   FOR x:= 1 TO 8 DO;
      FOR y:= 1 TO 8 DO; 
	 IF    field[x][y] = white THEN
	    INC(White)
         ELSIF field[x][y] = black THEN
	    INC(Black);
         END; (* IF *)
      END; (* FOR *)
   END; (* FOR *)

END Count;


(*************************************************************************)
(* This function returns the number of pieces for a given move.          *)
(* A modified version of Flip.                                           *)
(*************************************************************************)
PROCEDURE HowMany(player : PIECES; xcords, ycords: INTEGER; VAR count, weight:
		  INTEGER);

BEGIN
   count := 0;
   weight:= 0;
   NumFlips(player, xcords, ycords, 1,  0, count, weight);
   NumFlips(player, xcords, ycords,-1,  0, count, weight);
   NumFlips(player, xcords, ycords, 0,  1, count, weight);
   NumFlips(player, xcords, ycords, 0, -1, count, weight);
   NumFlips(player, xcords, ycords, 1, -1, count, weight);
   NumFlips(player, xcords, ycords, 1,  1, count, weight);
   NumFlips(player, xcords, ycords,-1,  1, count, weight);
   NumFlips(player, xcords, ycords,-1, -1, count, weight);

END HowMany; 


(* This procedure finds the number of flips as well as the value of  *)
(* the pieces that get fliped.  For example, it will report a larger *)
(* number for flipping an edge piece than a middle piece.            *)
(* Count is the number of pieces only.  weight is count + the total  *)
(* weight.                                                           *)
PROCEDURE NumFlips(player: PIECES; xcords, ycords, xdir, ydir: INTEGER;
		   VAR count, weight:INTEGER);

VAR
   opplayer: PIECES ; (* Opponent.                            *)
   x       : INTEGER; (* Used in counters.                    *)
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

   field[xcords][ycords]:= player;    (* Put in dummy player. *)

   foundop:= FALSE; 
   canflip:= FALSE;
   founds := FALSE;
   foundp := FALSE;
   x:= xcords;
   y:= ycords;
   LOOP
      IF (x= 0) OR (y= 0) OR (x= 9) OR (y= 9) THEN
	 EXIT;
      END; 
      IF (field[x][y]= opplayer) THEN
	  foundop:= TRUE;
      END; 
      IF ((field[x][y]= none)) AND ((foundop= FALSE) OR
          (foundp= FALSE)) THEN
	   founds:= TRUE;
      END; 
      IF (field[x][y]= player) AND (foundop) AND (founds= FALSE) THEN
	  canflip:= TRUE;
      END; 
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
         END; 
         IF (field[x][y]= player) THEN
	    EXIT;
         END; 
         IF (field[x][y]= opplayer) THEN
	    INC(count);
	    INC(weight); (* Find true weight of flipped pieces. *)
	    IF Edge(x, y) THEN
	       weight:= weight + EDGEFLIP;
            END; 
	    IF TwoAway(x, y) THEN
	       weight:= weight + TWOAWF;
            END; 
         END; (* IF *);
      END; (* LOOP *)
   END; (* IF *)

   field[xcords][ycords]:= none; (* Clear player. *)

END NumFlips;


(* TRUE if corner piece. *)
PROCEDURE Corner (x, y: INTEGER): BOOLEAN; 
BEGIN

   IF ((x=1) OR (x= 8)) AND ((y=1) OR (y=8)) THEN
      RETURN TRUE
   END;

   RETURN FALSE;

END Corner;


(* TRUE if edge piece. *)
PROCEDURE Edge (x, y: INTEGER): BOOLEAN; 
BEGIN

   IF ((x=1) OR (x=8)) OR ((y=1) OR (y=8)) THEN
      RETURN TRUE;
   END; (* IF *)

   RETURN FALSE;

END Edge;


(* TRUE if ajacent to corner. *)
PROCEDURE NextToCorner (x, y: INTEGER): BOOLEAN; 

VAR
   empty: BOOLEAN;
   xsearch, ysearch: INTEGER;

BEGIN

    empty:= FALSE;
    FOR xsearch:= -1 TO 1  DO
       FOR ysearch:= -1 TO 1 DO
          IF Corner(x + xsearch, y + ysearch) AND 
	     (field[x + xsearch][y + ysearch]= none) THEN
	        empty:= TRUE;
          END; (* IF *)
       END; (* FOR *)
    END; (* FOR *)

    FOR xsearch:= -1 TO 1  DO
       FOR ysearch:= -1 TO 1 DO
	  IF (Corner(x + xsearch, y + ysearch)) AND (NOT Corner(x, y)) AND
	     (empty) THEN
	      RETURN TRUE;
          END; (* IF *)
       END; (* FOR *)
    END; (* FOR *)

RETURN FALSE;

END NextToCorner;


(* TRUE if two pieces away. *)
PROCEDURE TwoAway (x, y: INTEGER): BOOLEAN; 

BEGIN

   IF (Corner(x - 2, y ))    OR
      (Corner(x - 2, y - 2)) OR
      (Corner(x, y - 2))     OR
      (Corner(x + 2, y ))    OR
      (Corner(x + 2, y - 2)) OR
      (Corner(x, y - 2))     OR
      (Corner(x, y + 2 ))    OR
      (Corner(x - 2, y + 2)) OR
      (Corner(x - 2, y - 2)) OR
      (Corner(x + 2, y))     OR
      (Corner(x + 2, y + 2)) OR
      (Corner(x, y + 2 ))    THEN 
      RETURN TRUE;
   END; (* IF *)
   RETURN FALSE;

END TwoAway;


(* Used all of the procedures above to find out the value of any *)
(* spot on the board.                                            *)
PROCEDURE Value(player: PIECES; x, y: INTEGER): INTEGER;

VAR
   number: INTEGER;
   dummy : INTEGER;
   WHITE : INTEGER;
   BLACK : INTEGER;

BEGIN

   IF Look THEN
      HowMany(player, x, y, dummy, number) 
      (* dummy is the number of pieces that can be flipped. *)
      (* number is that plus the weight of the flipped pieces. *)
   ELSE
      Count(WHITE, BLACK);
      HowMany(player, x, y, dummy, number); (* How many will flip *)

      (* This area does not take into consideration how many pieces it *)
      (* will flip until there are 30 pieces on thye board.            *)

      IF WHITE + BLACK < 30 THEN
         number:= number - dummy (* Does not care how many flips. *)
      (*   number:= number - (2 * dummy) Goes for least num.  *)
      ELSE
         number:= number + dummy; 
      END;
   END;

    (* Add in the weights for the corners, etc. *)
    IF Corner(x, y) THEN
       number:= number + CORNER; 
    END;
    IF Edge(x, y) THEN
       number:= number + EDGE;
    END;
    IF NextToCorner(x, y) THEN
       number:= number - ADJACENT;
    END;
    IF TwoAway(x, y) THEN
       number:= number + TWOAW;
    END;
    RETURN number;

END Value;


(* Used to find the best move.  Uses Value and Howmany, plus it *)
(* looks one move ahead.                                        *)
PROCEDURE FindBest(player: PIECES; VAR xcords, ycords: INTEGER);

VAR
   x, y  : INTEGER; (* Used in FOR loop. *)
   xx, yy: INTEGER; (* Used in FOR loop. *)
   total : INTEGER; (* Total value of current move *)
   value1: INTEGER; (* Value of computers move.    *)
   value2: INTEGER; (* Value of humans best counter move *)
   best  : INTEGER; (* The highest number so far. *)
   best1 : INTEGER; (* The highest number so far for humans move. *)
   opplayer: PIECES;(* Opponent. *)


BEGIN

   best  := -10000; (* Set low so that any move is better than it. *)
   xcords:= -1;

   IF player= black THEN (* Find opposite player. *)
      opplayer:= white
   ELSE
      opplayer:= black;
   END; (* IF *)

   FOR x:= 1 TO 8 DO
      FOR y:= 1 TO 8 DO 
	 IF Validate(player, x, y) THEN (* If valid move. *)
	    value1:= Value(player, x, y); 
	    CopyToTemp; (* Back up play surface. *)
	    Look:= TRUE;
	    field[x][y]:= player; (* Make theoretical move. *)
	    Flip(player, x, y); (* Flip pieces temporarily. *)
            best1 := -10000;
            FOR xx:= 1 TO 8 DO (* Check humans best move *)
               FOR yy:= 1 TO 8 DO 
	          IF Validate(opplayer, xx, yy) THEN
	             value2:= Value(opplayer, xx, yy); 
                     IF value2 >= best1 THEN (* Get best human move. *)
   	                best1:= value2;
                     END (* IF *)
		  END; (* IF *)
	       END; (* FOR *)
	    END; (* FOR *)
	    
	    total:= (value1 * MULF) - (best1 * 2); 
		           	    (* Get the value of a move by   *)
				    (* finding the weight of the    *)
				    (* computers best move and then *)
				    (* subtract the weight of the   *)
				    (* humans best opposing move.   *)

	    CopyBack;               (* Put the game board back *)
	    Look:= FALSE;

	    (* Find best move. *)
            IF total > best THEN
   	       best:= total;
	       xcords:= x;
	       ycords:= y;
            END (* IF *)
         END; (* IF *)
      END; (* FOR *)
   END; (* FOR *)

   (*
   Position(0, 420); (* Optional: display the weight of the computers *)
   TextSize(5);      (* move.                                         *)
   Color("G");
   WriteNum(best);
   Position(0, 420);
   Color("D");
   WriteNum(best);
   TextSize(1);
   Position(180, 180);
   *)

END FindBest;


(******************************************************************)
(* This procedure allows the computer to make a move.             *)
(*                                                                *)
(* First it finds all valid moves.  For each one, it adds up the  *)
(* number of pieces that it will flip for taking this square.     *)
(* If the space it can move to is on the edge, it weighs this     *)
(* more.  If it is a corner, it weighs it a lot more.  If it is   *)
(* adjacent to a corner, it avoids it.  If it is two squares away *)
(* from a corner, it weighs it more than a side alone.  I doesn't *)
(* avoid the adjacent pieces if the corner is already taken.      *)
(* Note: it takes the above things into account for the pieces    *)
(* that it will flip also.  For each valid move, it takes it's    *)
(* best shot at the counter move.  It then subtracts the two.     *)
(* eg.  If it has to move to a piece adjacent to a corner, it     *)
(* take the one that will give the opponent the least advantage.  *)
(* It will, for example try to keep him from getting the corner.  *)
(*                                                                *)
(******************************************************************)
PROCEDURE Move (player: PIECES; VAR xcords, ycords: INTEGER);

VAR
   pass  : BOOLEAN;
   x, y  : INTEGER; (* Used in FOR loop. *)
   number: INTEGER; (* The number of pieces gained for this move. *)
   best  : INTEGER; (* The highest number so far. *)
   valid : BOOLEAN;
   Delay : INTEGER;

BEGIN

   IF player= white THEN  (* Print out whose move it is. *)
      Color('D');
      Position(420, 120);
      WriteText('Blacks move.');
      Color("R");
      Position(420, 120);
      WriteText('Blues move.');
      Position(180, 180);
   ELSE
      Color('D');
      Position(420, 120);
      WriteText('Blues move.');
      Color("R");
      Position(420, 120);
      WriteText('Blacks move.');
      Position(180, 180);
   END; (* IF *);

   FindBest(player, xcords, ycords); (* Get the best move. *)
   
   IF xcords > 0 THEN      (* If legal move has been found.*)
      field  [xcords][ycords]:= player; (* Make it.        *)
      flipped[xcords][ycords]:= TRUE;
      Color("R");
      Position((xcords * 50) -20, (ycords * 50) -20);
      SetFill((ycords * 50) - 20);
      Circle(13, 13);
      UnsetFill;
      Passed:= FALSE;
   ELSE                        (* No legal moves. *)
      Passed:= TRUE;
      xcords:= 1;
      Position(420, 150);
      Position(420, 150);
      Color("G");
      WriteText('I pass.');
      Position(180, 180);
      FOR Delay:= 1 TO 300000 DO
      END;
      Color("D");
      Position(420, 150);
      WriteText('I pass.')
   END; (* IF *)

END Move;


(* TRUE if a player has no legal moves, and can pass. *)
PROCEDURE CanPass (player: PIECES): BOOLEAN;

VAR
   canpass: BOOLEAN;
   x, y   : INTEGER;

BEGIN

   canpass:= TRUE;
   FOR x:= 1 TO 8 DO
      FOR y:= 1 TO 8 DO
         IF Validate(player, x, y) THEN (* Just one good move, and you *)
	    canpass:= FALSE;            (* cannot pass.                *)
         END; (* IF *)
      END; (* FOR *)
   END; (* FOR *)
   RETURN canpass;

END CanPass;


(* Copies board to temp.  Used to make theoretical moves. *)
PROCEDURE CopyToTemp;
VAR
   x, y: INTEGER;

BEGIN
   FOR x:= 1 TO 8 DO
      FOR y:= 1 TO 8 DO
	 temp[x][y]:= field[x][y];
      END; (* FOR *)
   END; (* FOR *)  
END CopyToTemp;


PROCEDURE CopyBack; (* Replace board. *)
VAR
   x, y: INTEGER;

BEGIN
   FOR x:= 1 TO 8 DO
      FOR y:= 1 TO 8 DO
	 field[x][y]:= temp[x][y];
      END; (* FOR *)
   END; (* FOR *)  
END CopyBack;


END move.



