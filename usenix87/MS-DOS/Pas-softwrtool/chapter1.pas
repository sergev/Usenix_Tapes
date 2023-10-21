{chapter1.pas}

{
        Copyright (c) 1981
        By:     Bell Telephone Laboratories, Inc. and
                Whitesmith's Ltd.,

        This software is derived from the book
                "Software Tools in Pascal", by
                Brian W. Kernighan and P. J. Plauger
                Addison-Wesley, 1981
                ISBN 0-201-10342-7

        Right is hereby granted to freely distribute or duplicate this
        software, providing distribution or duplication is not for profit
        or other commercial gain and that this copyright notice remains
        intact.
}

PROCEDURE COPY;
VAR C:CHARACTER;
BEGIN
  WHILE(GETC(C)<>ENDFILE)DO
    PUTC(C)
END;


PROCEDURE CHARCOUNT;
VAR
  NC:INTEGER;
  C:CHARACTER;
BEGIN
  NC:=0;
  WHILE (GETC(C)<>ENDFILE)DO
     NC:=NC+1;
  PUTDEC(NC,1);
  PUTC(NEWLINE)
END;

PROCEDURE LINECOUNT;
VAR
  N1:INTEGER;
  C:CHARACTER;
BEGIN
  N1:=0;
  WHILE(GETC(C)<>ENDFILE)DO
    IF(C=NEWLINE)THEN
      N1:=N1+1;
  PUTDEC(N1,1);
  PUTC(NEWLINE)
END;

PROCEDURE WORDCOUNT;
VAR
  NW:INTEGER;
  C:CHARACTER;
  INWORD:BOOLEAN;
BEGIN
  NW:=0;
  INWORD:=FALSE;
  WHILE(GETC(C)<>ENDFILE)DO
    IF(C=BLANK)OR(C=NEWLINE)OR(C=TAB) THEN
      INWORD:=FALSE
    ELSE IF (NOT INWORD)THEN BEGIN
      INWORD:=TRUE;
      NW:=NW+1
    END;
  PUTDEC(NW,1);
  PUTC(NEWLINE)
END;

PROCEDURE DETAB;
CONST
  MAXLINE=1000;
TYPE
  TABTYPE=ARRAY[1..MAXLINE] OF BOOLEAN;
VAR
  C:CHARACTER;
  COL:INTEGER;
  TABSTOPS:TABTYPE;

FUNCTION TABPOS(COL:INTEGER;VAR TABSTOPS:TABTYPE)
  :BOOLEAN;
BEGIN
  IF(COL>MAXLINE)THEN
    TABPOS:=TRUE
  ELSE
    TABPOS:=TABSTOPS[COL]
END;

PROCEDURE SETTABS(VAR TABSTOPS:TABTYPE);
CONST
  TABSPACE=4;
VAR
  I:INTEGER;
BEGIN
  FOR I:=1 TO MAXLINE DO
    TABSTOPS[I]:=(I MOD TABSPACE = 1)
END;

BEGIN
  SETTABS(TABSTOPS);
  COL:=1;
  WHILE(GETC(C)<>ENDFILE)DO
    IF(C=TAB)THEN
     REPEAT
      PUTC(BLANK);
      COL:=COL+1
     UNTIL(TABPOS(COL,TABSTOPS))
    ELSE IF(C=NEWLINE)THEN BEGIN
      PUTC(NEWLINE);
      COL:=1
    END
    ELSE BEGIN
      PUTC(C);
      COL:=COL+1
    END
END;




