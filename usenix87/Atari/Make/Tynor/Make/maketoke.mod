IMPLEMENTATION MODULE MakeToken;

  (*
   * MAKEMAKE.  Create a MAKEFILE for a MODULA-2 program.
   *
   * Written by Steve Tynor, 30 September 1986.
   *            UUCP  : tynor@gitpyr
   *            USNAIL: 2550 Akers Mill Rd. T-2, Atlanta GA. 30339
   *
   * Permission is granted to distribute, copy and change this program as long
   * as this notice remains...
   *)

IMPORT Streams;

VAR 
  currentCh : CHAR;
  currentStream : Streams.Stream;
  
  (*======================================================================*)
  MODULE StreamStack;
  IMPORT Streams;
  EXPORT Pop, Push;

  CONST
    StackSize = 20;
  VAR
    StrStack  : ARRAY [1 .. StackSize] OF Streams.Stream;
    CharStack : ARRAY [1 .. StackSize] OF CHAR;
    StackPtr  : CARDINAL;

    (*--------------------------------------------------------------------*)
    PROCEDURE Push (str : Streams.Stream; ch : CHAR);
    BEGIN
      IF StackPtr + 1 <= StackSize THEN
        INC (StackPtr);
        StrStack [StackPtr] := str;
        CharStack[StackPtr] := ch;
      END; (* IF *)
    END Push;

    (*--------------------------------------------------------------------*)
    PROCEDURE Pop (VAR str : Streams.Stream; VAR ch : CHAR);
    BEGIN
      IF StackPtr > 0 THEN
        str := StrStack [StackPtr];
        ch  := CharStack[StackPtr];
        DEC (StackPtr);
      END; (* IF *)
    END Pop;

  BEGIN
    StackPtr := 0;
  END StreamStack;
  (*======================================================================*)
 

  (*----------------------------------------------------------------------*)
  PROCEDURE OpenFile (VAR filename : ARRAY OF CHAR;
                      VAR success  : BOOLEAN);
  VAR
    result : INTEGER;
  BEGIN
    Push (currentStream, currentCh);
    Streams.OpenStream(currentStream, filename, Streams.READ, result);
    currentCh := ' ';
    success := result = 0;
  END OpenFile;

  (*----------------------------------------------------------------------*)
  PROCEDURE CloseFile;
  VAR
    result : INTEGER;
  BEGIN
    Streams.CloseStream(currentStream, result);
    Pop (currentStream, currentCh);
  END CloseFile;


  (*----------------------------------------------------------------------*)
  PROCEDURE SeparatingChar (ch : CHAR) : BOOLEAN;
    (* we're just worried about a subset of the real separating chars... *)
  BEGIN
    RETURN NOT (((ORD(ch) >= ORD('a')) AND (ORD(ch) <= ORD('z'))) OR
                ((ORD(ch) >= ORD('A')) AND (ORD(ch) <= ORD('Z'))) OR
                ((ORD(ch) >= ORD('0')) AND (ORD(ch) <= ORD('9'))));
  END SeparatingChar;


  (*----------------------------------------------------------------------*)
  PROCEDURE NextToken (VAR token : ARRAY OF CHAR;
                       VAR eof   : BOOLEAN);
  VAR
    c : CARDINAL;
  BEGIN
    c := 0;
    IF currentCh = ';' THEN
      token[0] := ';';
      token[1] := 0C;
      eof := Streams.EOS(currentStream);
      Streams.Read8Bit(currentStream, currentCh);
      RETURN;
    END; (* IF *)
    WHILE SeparatingChar (currentCh) DO
      Streams.Read8Bit(currentStream, currentCh);
      IF currentCh = ';' THEN
        token[0] := ';';
        token[1] := 0C;
        eof := Streams.EOS(currentStream);
        Streams.Read8Bit(currentStream, currentCh);
        RETURN;
      ELSIF currentCh = '(' THEN
        token[c] := currentCh;
        Streams.Read8Bit(currentStream, currentCh);
        IF currentCh = '*' THEN
          SkipComment;
        ELSE
          token[0] := '(';
          token[1] := currentCh;
          c := 2;
        END; (* IF *)
      END; (* IF *)
    END; (* WHILE *)
    REPEAT
      IF currentCh = '(' THEN
        token[c] := currentCh;
        Streams.Read8Bit(currentStream, currentCh);
        IF currentCh = '*' THEN
          IF c > 0 THEN
            token[c+1] := 0C;
            SkipComment;
            RETURN;
          ELSE
            SkipComment;
          END; (* IF *)
        END; (* IF *)
      END; (* IF *)
      token[c] := currentCh;
      INC (c);
      Streams.Read8Bit(currentStream, currentCh);
    UNTIL Streams.EOS(currentStream) OR 
          (c > HIGH (token)) OR 
          SeparatingChar (currentCh);
    token[c] := 0C;
    eof := Streams.EOS (currentStream);
  END NextToken;


  (*----------------------------------------------------------------------*)
  PROCEDURE SkipTillSemicolon (VAR eof   : BOOLEAN);
  VAR
    ch : CHAR;
  BEGIN
    REPEAT
      IF (NOT Streams.EOS(currentStream)) AND (currentCh = '(') THEN
        Streams.Read8Bit(currentStream, currentCh);
        IF (NOT Streams.EOS (currentStream)) AND (currentCh = '*') THEN
          SkipComment;
        END; (* IF *)
      ELSE
        Streams.Read8Bit(currentStream, currentCh);
      END; (* IF *)
    UNTIL Streams.EOS (currentStream) OR (currentCh = ';');
  END SkipTillSemicolon;


  (*----------------------------------------------------------------------*)
  PROCEDURE SkipComment;
  VAR
    level : CARDINAL;
    done  : BOOLEAN;
  BEGIN
    level := 1;
    Streams.Read8Bit(currentStream, currentCh);
    done := FALSE;
    REPEAT
      IF (NOT Streams.EOS(currentStream)) AND (currentCh = '*') THEN
        Streams.Read8Bit(currentStream, currentCh);
        IF (NOT Streams.EOS(currentStream)) AND (currentCh = ')') THEN
          DEC(level);
          done := level = 0;
        END; (* IF *)
      ELSIF (NOT Streams.EOS(currentStream)) AND (currentCh = '(') THEN
        Streams.Read8Bit(currentStream, currentCh);
        IF (NOT Streams.EOS (currentStream)) AND (currentCh = '*') THEN
          INC(level);
        END; (* IF *)
      ELSE
        Streams.Read8Bit(currentStream, currentCh);
      END; (* IF *)
    UNTIL Streams.EOS(currentStream) OR done;
  END SkipComment;

BEGIN
  currentCh := ' ';
END MakeToken.
