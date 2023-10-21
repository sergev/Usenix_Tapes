IMPLEMENTATION MODULE regis;
(* Lo-level ReGIS graphics commands.                             *)

FROM 	SysStreams	IMPORT	sysOut;
FROM 	CharCodes	IMPORT  EscapeCh;
FROM    TextIO          IMPORT  WriteCHAR;
FROM    InOut           IMPORT  Write;
FROM    InOut           IMPORT  WriteInt;
FROM    InOut           IMPORT  WriteString;


PROCEDURE esc; (* Prints an escape character. *)
BEGIN
  WriteCHAR(sysOut,EscapeCh)
END esc;

PROCEDURE ExitGr; (* Exit ReGIS mode. *)
BEGIN
  esc;
  Write("S");
END ExitGr;

PROCEDURE Reset; (* Sets defaults. *)
BEGIN
   esc;
   WriteString('Pp;S(I0N0A)W(VI7A0S0M1N0P1M2)T(I0A0D0S1)P[0,0]\\');
END Reset;

PROCEDURE Position(x,y:INTEGER); (* Position cursor  at x, y     *)
BEGIN
   esc;
   WriteString('PpP[');
   WriteInt(x ,1);
   Write(',');
   WriteInt(y,1);
   WriteString(']\\');
END Position;

PROCEDURE Circle(x,y:INTEGER); (* Draw circle of width x, hight y. *)
BEGIN
   esc;
   WriteString('PpC[+');
   WriteInt(x,1);
   WriteString(',+');
   WriteInt(y,1);
   WriteString(']\\');
END Circle;

PROCEDURE Arc(x,y,angle:INTEGER); (* Draw an arc. *)
BEGIN
   esc;
   WriteString('PpC(A');
   WriteInt(angle,1);
   WriteString(')[');

   IF x >=0 THEN
      Write('+');
      WriteInt(x, 1);
   ELSE
      Write('-');
      WriteInt(-x, 1);
   END;
   IF y >=0 THEN
      Write('+');
      WriteInt(y, 1);
      WriteString(']\\');
   ELSE
      Write('-');
      WriteInt(-y, 1);
      WriteString(']\\');
   END;
END Arc;

PROCEDURE Pattern(p:String);
BEGIN
  esc;
  WriteString('PpW(P"');
  WriteString(p);
  WriteString('")\\');
END Pattern;

PROCEDURE Plot(x,y:INTEGER); (* Draw one pixel. *)
BEGIN
   Position(x,y);
   esc;
   WriteString('PpV[]\\');
END Plot;

PROCEDURE DrawTo(x,y:INTEGER); (* Used after other command, draws line *)
BEGIN
   esc;
   WriteString('PpV[');
   WriteInt(x, 1);
   Write(',');
   WriteInt(y, 1);
   WriteString(']\\');
END DrawTo;

PROCEDURE Color(colr: CHAR); (* Use R(ed), G(reen), B(lue), or D(ark). *)
BEGIN
   esc;
   WriteString('Pp W(I(H');
   Write(colr);
   WriteString('))\\');
END Color;

PROCEDURE Box(width, height: INTEGER);
BEGIN
   esc;
   WriteString('PpV[+');
   WriteInt(width, 1);
   WriteString(',+');
   WriteInt(0, 1);
   WriteString(']\\');
   WriteString('PpV[+');
   WriteInt(0, 1);
   WriteString(',+');
   WriteInt(height, 1);
   WriteString(']\\');
   WriteString('PpV[-');
   WriteInt(width, 1);
   WriteString(',+');
   WriteInt(0, 1);
   WriteString(']\\');
   WriteString('PpV[+');
   WriteInt(0, 1);
   WriteString(',-');
   WriteInt(height, 1);
   WriteString(']\\');
END Box;

PROCEDURE Scroll(dx,dy:INTEGER);
BEGIN
   esc;
   WriteString('PpS[+');
   WriteInt(dx, 1);
   WriteString(',+');
   WriteInt(dy, 1);
   WriteString(']\\')
END Scroll;

PROCEDURE ClearScreen; (* Clears graphic screen *)
BEGIN
   esc;
   WriteString('PpS(E)\\');
END ClearScreen;

PROCEDURE WriteText(message: String); 
BEGIN
   esc;
   WriteString('PpT"');
   WriteString(message);
   WriteString('"\\');
END WriteText;

PROCEDURE WriteNum(num : INTEGER);
BEGIN
   esc;
   WriteString('PpT"');
   WriteInt(num, 1);
   WriteString('"\\');
END WriteNum;

PROCEDURE WriteChr(chr : CHAR);
BEGIN
   esc;
   WriteString('PpT"');
   Write(chr);
   WriteString('"\\');
END WriteChr;

PROCEDURE TextDirection(angle: INTEGER); (* Write vertically, etc. *)
BEGIN
   esc;
   WriteString('PpT(D');
   WriteInt(angle, 1);
   WriteString(')\\');
END TextDirection;

PROCEDURE TextSlant(angle: INTEGER); (* -10 slants to right. *)
BEGIN
   esc;
   WriteString('PpT(I');
   WriteInt(angle, 1);
   WriteString(')\\');
END TextSlant;

PROCEDURE TextSize(size: INTEGER); (* 2 doubles size. *)
BEGIN
   esc;
   WriteString('PpT(S');
   WriteInt(size, 1);
   WriteString(')\\');
END TextSize;

(*
PROCEDURE TextMatrixSize(x,y:INTEGER);
BEGIN
  esc;
  WriteF2(sysOut,'PpT(S[%d,%d])\\',x,y);
END TextMatrixSize;

PROCEDURE TextPixelMultiply(x,y:INTEGER);
BEGIN
  esc;
  WriteF2(sysOut,'[PpT(M[%d,$d])\\',x,y);
END TextPixelMultiply;

PROCEDURE TextAttributes(Width,Height,xMult,yMult,xDir,yDir,Slant : INTEGER);
BEGIN
  WriteF0(sysOut,'PpT(S[%d,%d] M[%d,%d] I%d ) [%d,%d]',
                Width,Height,xMult,yMult,Slant,xDir,yDir);
END TextAttributes;

PROCEDURE TextSpacing(dx,dy:INTEGER);
BEGIN
   esc;
   WriteF2(sysOut,'Pp[%d,%d]\\',dx,dy);
END TextSpacing;

PROCEDURE SelectCharSet(n : INTEGER);
BEGIN
   esc;
   WriteF1(sysOut,'PpT(A%d)\\',n);
END SelectCharSet;

PROCEDURE RedefineChar(Cset : INTEGER; c : CHAR; pattern : String);
VAR i : INTEGER;
BEGIN
   esc;
   WriteF2(sysOut,'PpL(A%d) ''%c'' ',Cset,c);
   FOR i:= 0 to high(pattern) DO 
     esc;
     WriteF1(sysOut,'%c',pattern[i]);
   END;
   esc;
   WriteF0(sysOut,'\\');
END RedefineChar;
*)

PROCEDURE SetFill(y: INTEGER); (* Turns on fill at y. *)
BEGIN
   esc;
   WriteString('PpW(S1)(S[,');
   WriteInt(y,1);
   WriteString(';])\\');
END SetFill;

(*
PROCEDURE SetFillChar(c: CHAR; y:INTEGER);
BEGIN
   esc;
   WriteF2(sysOut,'PpW(S''%c''[,%d])\\',c,y);
END SetFillChar;
*)

PROCEDURE UnsetFill; (* Turns off fill. *)
BEGIN
   esc;
   WriteString('PpW(S0)\\');
END UnsetFill;

PROCEDURE BGColor(colr:CHAR); (* Back ground color. *)
BEGIN
   esc;
   WriteString('PpS(I(H');
   Write(colr);
   WriteString('))\\');
END BGColor;

PROCEDURE NormalText;
BEGIN
   esc;
   WriteString('PpT(E)\\');
END NormalText;


END regis.


