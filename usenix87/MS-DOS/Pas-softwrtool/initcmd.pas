{initcmd.pas}

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

PROCEDURE INITCMD;
VAR
  FD:FILEDESC;
  FNAME:XSTRING;
  FT:FILTYP;
  IDX:1..MAXSTR;
  I,JSKIP:INTEGER;
  JUNK:BOOLEAN;


BEGIN
  CMDFIL[STDIN]:=STDIO;
  CMDFIL[STDOUT]:=STDIO;
  CMDFIL[STDERR]:=STDIO;
  FOR FD:=SUCC(STDERR) TO MAXOPEN DO
    CMDFIL[FD]:=CLOSED;
  WRITELN;
  write('$ ');
  FOR FT:= FIL1 TO FIL4 DO
    CMDOPEN[FT]:=FALSE;
  KBDN:=0;
  if (not getline(cmdlin,STDIN,MAXSTR)) then error('NO CMDLINE');
CMDARGS:=0;
  JSKIP:=0;
  IDX:=1;
  WHILE ((CMDLIN[IDX]<>ENDSTR)
    AND(CMDLIN[IDX]<>NEWLINE)) DO BEGIN
      WHILE((CMDLIN[IDX]=BLANK)AND(JSKIP MOD 2 <>1))DO
        IDX:=IDX+1;
      IF(CMDLIN[IDX]<>NEWLINE) THEN BEGIN
        CMDARGS:=CMDARGS+1;
        CMDIDX[CMDARGS]:=IDX-JSKIP;
        WHILE((CMDLIN[IDX]<>NEWLINE)AND
          ((CMDLIN[IDX]<>BLANK)OR(JSKIP MOD 2 <>0)))DO BEGIN
              IF (CMDLIN[IDX]=DQUOTE)THEN BEGIN
                JSKIP:=JSKIP+1;
                IDX:=IDX+1
              END
              ELSE BEGIN
                CMDLIN[IDX-JSKIP]:=CMDLIN[IDX];
                IDX:=IDX+1
              END

            END;
        CMDLIN[IDX-JSKIP]:=ENDSTR;
        IDX:=IDX+1;
        IF (CMDLIN[CMDIDX[CMDARGS]]=LESS) THEN BEGIN
          XCLOSE(STDIN);
          CMDIDX[CMDARGS]:=CMDIDX[CMDARGS]+1;
          JUNK:=GETARG(CMDARGS,FNAME,MAXSTR);
          FD:=MUSTOPEN(FNAME,IOREAD);
          CMDARGS:=CMDARGS-1;
        END
        ELSE IF (CMDLIN[CMDIDX[CMDARGS]]=GREATER) THEN BEGIN
          XCLOSE(STDOUT);
          CMDIDX[CMDARGS]:=CMDIDX[CMDARGS]+1;
          JUNK:=GETARG(CMDARGS,FNAME,MAXSTR);
          FD:=MUSTCREATE(FNAME,IOWRITE);
          CMDARGS:=CMDARGS-1;
        END
      END
    END;
END;



