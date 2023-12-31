{chapter8.pas}

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

PROCEDURE MACRO;
CONST
  BUFSIZE=1000;
  MAXCHARS=500;
  MAXPOS=500;
  CALLSIZE=MAXPOS;
  ARGSIZE=MAXPOS;
  EVALSIZE=MAXCHARS;
  MAXDEF=MAXSTR;
  MAXTOK=MAXSTR;
  HASHSIZE=53;
  ARGFLAG=DOLLAR;
TYPE
  CHARPOS=1..MAXCHARS;
  CHARBUF=ARRAY[1..MAXCHARS]OF CHARACTER;
  POSBUF=ARRAY[1..MAXPOS]OF CHARPOS;
  POS=0..MAXPOS;
  STTYPE=(DEFTYPE,MACTYPE,IFTYPE,SUBTYPE,
  EXPRTYPE,LENTYPE,CHQTYPE);
  NDPTR=^NDBLOCK;
  NDBLOCK=RECORD
    NAME:CHARPOS;
    DEFN:CHARPOS;
    KIND:STTYPE;
    NEXTPTR:NDPTR
   END;

VAR
  BUF:ARRAY[1..BUFSIZE]OF CHARACTER;
  BP:0..BUFSIZE;
  HASHTAB:ARRAY[1..HASHSIZE]OF NDPTR;
  NDTABLE:CHARBUF;
  NEXTTAB:CHARPOS;
  CALLSTK:POSBUF;
  CP:POS;
  TYPESTK:ARRAY[1..CALLSIZE]OF STTYPE;
  PLEV:ARRAY[1..CALLSIZE]OF INTEGER;
  ARGSTK:POSBUF;
  AP:POS;
  EVALSTK:CHARBUF;
  EP:CHARPOS;
  (*BUILTINS*)
  DEFNAME:XSTRING;
  EXPRNAME:XSTRING;
  SUBNAME,IFNAME,LENNAME,CHQNAME:XSTRING;
  NULL:XSTRING;
  LQUOTE,RQUOTE:CHARACTER;
  DEFN,TOKEN:XSTRING;
  TOKTYPE:STTYPE;
  T:CHARACTER;
  NLPAR:INTEGER;
PROCEDURE PUTCHR(C:CHARACTER);
BEGIN
  IF(CP<=0) THEN
    PUTC(C)
  ELSE BEGIN
    IF(EP>EVALSIZE)THEN
      ERROR('MACRO:EVALUATION STACK OVERFLOW');
    EVALSTK[EP]:=C;
    EP:=EP+1
  END
END;

PROCEDURE PUTTOK(VAR S:XSTRING);
VAR
  I:INTEGER;
BEGIN
  I:=1;
  WHILE(S[I]<>ENDSTR) DO BEGIN
    PUTCHR(S[I]);
    I:=I+1
  END
END;


FUNCTION PUSH(EP:INTEGER;VAR ARGSTK:POSBUF;AP:INTEGER):INTEGER;
BEGIN
  IF(AP>ARGSIZE)THEN
    ERROR('MACRO:ARGUMENT STACK OVERFLOW');
  ARGSTK[AP]:=EP;
  PUSH:=AP+1
END;

PROCEDURE SCCOPY(VAR S:XSTRING;VAR CB:CHARBUF;
I:CHARPOS);
VAR J:INTEGER;
BEGIN
  J:=1;
  WHILE(S[J]<>ENDSTR)DO BEGIN
    CB[I]:=S[J];
    J:=J+1;
    I:=I+1
  END;
  CB[I]:=ENDSTR
END;

PROCEDURE CSCOPY(VAR CB:CHARBUF;I:CHARPOS;
  VAR S:XSTRING);
VAR J:INTEGER;
BEGIN
  J:=1;
  WHILE(CB[I]<>ENDSTR)DO BEGIN
    S[J]:=CB[I];
    I:=I+1;
    J:=J+1
  END;
  S[J]:=ENDSTR
END;


PROCEDURE PUTBACK(C:CHARACTER);
BEGIN
  IF(BP>=BUFSIZE)THEN
    WRITELN('TOO MANY CHARACTERS PUSHED BACK');
  BP:=BP+1;
  BUF[BP]:=C
END;

FUNCTION GETPBC(VAR C:CHARACTER):CHARACTER;
BEGIN
  IF(BP>0)THEN
    C:=BUF[BP]
  ELSE BEGIN
    BP:=1;
    BUF[BP]:=GETC(C)
  END;
  IF(C<>ENDFILE)THEN
    BP:=BP-1;
  GETPBC:=C
END;

FUNCTION GETTOK(VAR TOKEN:XSTRING;TOKSIZE:INTEGER):
  CHARACTER;
VAR I:INTEGER;
    DONE:BOOLEAN;
BEGIN
  I:=1;
  DONE:=FALSE;
  WHILE(NOT DONE) AND (I<TOKSIZE) DO
    IF(ISALPHANUM(GETPBC(TOKEN[I]))) THEN
      I:=I+1
    ELSE
      DONE:=TRUE;
  IF(I>=TOKSIZE)THEN
    WRITELN('DEFINE:TOKEN TOO LONG');
  IF(I>1) THEN BEGIN (*SOME ALPHA WAS SEEN*)
    PUTBACK(TOKEN[I]);
    I:=I-1
  END;
  (*ELSE SINGLE NON-ALPHANUMERIC*)
  TOKEN[I+1]:=ENDSTR;
  GETTOK:=TOKEN[1]
END;

PROCEDURE PBSTR (VAR S:XSTRING);
VAR I:INTEGER;
BEGIN
  FOR I:=XLENGTH(S) DOWNTO 1 DO
    PUTBACK(S[I])
END;


FUNCTION HASH(VAR NAME:XSTRING):INTEGER;
VAR
  I,H:INTEGER;
BEGIN
  H:=0;
  FOR I:=1 TO XLENGTH(NAME) DO
    H:=(3*H+NAME[I]) MOD HASHSIZE;
  HASH:=H+1
END;

FUNCTION HASHFIND(VAR NAME:XSTRING):NDPTR;
VAR
  P:NDPTR;
  TEMPNAME:XSTRING;
  FOUND:BOOLEAN;
BEGIN
  FOUND:=FALSE;
  P:=HASHTAB[HASH(NAME)];
  WHILE (NOT FOUND) AND (P<>NIL) DO BEGIN
    CSCOPY(NDTABLE,P^.NAME,TEMPNAME);
    IF(EQUAL(NAME,TEMPNAME)) THEN
      FOUND:=TRUE
    ELSE
      P:=P^.NEXTPTR
  END;
  HASHFIND:=P
END;

PROCEDURE INITHASH;
VAR I:1..HASHSIZE;
BEGIN
  NEXTTAB:=1;
  FOR I:=1 TO HASHSIZE DO
    HASHTAB[I]:=NIL
END;

FUNCTION LOOKUP(VAR NAME,DEFN:XSTRING; VAR T:STTYPE)
 :BOOLEAN;
VAR P:NDPTR;
BEGIN
  P:=HASHFIND(NAME);
  IF(P=NIL)THEN
    LOOKUP:=FALSE
  ELSE BEGIN
    LOOKUP:=TRUE;
    CSCOPY(NDTABLE,P^.DEFN,DEFN);
    T:=P^.KIND
  END
END;


PROCEDURE INSTALL(VAR NAME,DEFN:XSTRING;T:STTYPE);
VAR
  H,DLEN,NLEN:INTEGER;
  P:NDPTR;
BEGIN
  NLEN:=XLENGTH(NAME)+1;
  DLEN:=XLENGTH(DEFN)+1;
  IF(NEXTTAB + NLEN +DLEN > MAXCHARS) THEN BEGIN
    PUTSTR(NAME,STDERR);
    ERROR(':TOO MANY DEFINITIONS')
  END
  ELSE BEGIN
    H:=HASH(NAME);
    NEW(P);
    P^.NEXTPTR:=HASHTAB[H];
    HASHTAB[H]:=P;
    P^.NAME:=NEXTTAB;
    SCCOPY(NAME,NDTABLE,NEXTTAB);
    NEXTTAB:=NEXTTAB+NLEN;
    P^.DEFN:=NEXTTAB;
    SCCOPY(DEFN,NDTABLE,NEXTTAB);
    NEXTTAB:=NEXTTAB+DLEN;
    P^.KIND:=T
  END
END;



PROCEDURE DODEF(VAR ARGSTK:POSBUF;I,J:INTEGER);
VAR
  TEMP1,TEMP2 : XSTRING;
BEGIN
  IF(J-I>2) THEN BEGIN
    CSCOPY(EVALSTK,ARGSTK[I+2],TEMP1);
    CSCOPY(EVALSTK,ARGSTK[I+3],TEMP2);
    INSTALL(TEMP1,TEMP2,MACTYPE)
  END
END;
  

PROCEDURE DOIF(VAR ARGSTK:POSBUF;I,J:INTEGER);
VAR
  TEMP1,TEMP2,TEMP3:XSTRING;
BEGIN
  IF(J-I>=4) THEN BEGIN
    CSCOPY(EVALSTK,ARGSTK[I+2],TEMP1);
    CSCOPY(EVALSTK,ARGSTK[I+3],TEMP2);
    IF(EQUAL(TEMP1,TEMP2))THEN
      CSCOPY(EVALSTK,ARGSTK[I+4],TEMP3)
    ELSE IF (J-I>=5) THEN
      CSCOPY(EVALSTK,ARGSTK[I+5],TEMP3)
    ELSE
      TEMP3[I]:=ENDSTR;
    PBSTR(TEMP3)
  END
END;

PROCEDURE PBNUM(N:INTEGER);
VAR
  TEMP:XSTRING;
  JUNK:INTEGER;
BEGIN
  JUNK:=ITOC(N,TEMP,1);
  PBSTR(TEMP)
END;
FUNCTION EXPR(VAR S:XSTRING;VAR I:INTEGER):INTEGER;FORWARD;

PROCEDURE DOEXPR(VAR ARGSTK:POSBUF;I,J:INTEGER);
VAR
  JUNK:INTEGER;
  TEMP:XSTRING;
BEGIN
  CSCOPY(EVALSTK,ARGSTK[I+2],TEMP);
  JUNK:=1;
  PBNUM(EXPR(TEMP,JUNK))
END;

FUNCTION EXPR;
VAR
  V:INTEGER;
  T:CHARACTER;
  
FUNCTION GNBCHAR(VAR S:XSTRING;VAR I:INTEGER):CHARACTER;
BEGIN
  WHILE(S[I]IN[BLANK,TAB,NEWLINE])DO
    I:=I+1;
  GNBCHAR:=S[I]
END;

FUNCTION TERM(VAR S:XSTRING;VAR I:INTEGER):INTEGER;
VAR
  V:INTEGER;
  T:CHARACTER;

FUNCTION FACTOR (VAR S:XSTRING;VAR I:INTEGER):
  INTEGER;
BEGIN
  IF(GNBCHAR(S,I)=LPAREN) THEN BEGIN
    I:=I+1;
    FACTOR:=EXPR(S,I);
    IF(GNBCHAR(S,I)=RPAREN) THEN
      I:=I+1
    ELSE
      WRITELN('MACRO:MISSING PAREN IN EXPR')
  END
  ELSE
    FACTOR:=CTOI(S,I)
END;(*FACTOR*)

BEGIN(*TERM*)
  V:=FACTOR(S,I);
  T:=GNBCHAR(S,I);
  WHILE(T IN [STAR,SLASH,PERCENT]) DO BEGIN
    I:=I+1;
    CASE T OF
      STAR:V:=V*FACTOR(S,I);
    SLASH:
      V:=V DIV FACTOR(S,I);
    PERCENT:
      V:=V MOD FACTOR(S,I)
    END;
    T:=GNBCHAR(S,I)
  END;
  TERM:=V
END;(*TERM*)

BEGIN(*EXPR*)
  V:=TERM(S,I);
  T:=GNBCHAR(S,I);
  WHILE(T IN [PLUS,MINUS])DO BEGIN
    I:=I+1;
    IF(T IN [PLUS]) THEN
      V:=V+TERM(S,I)
    ELSE(*MINUS*)
      V:=V-TERM(S,I);
    T:=GNBCHAR(S,I)
  END;
  EXPR:=V
END;

PROCEDURE DOLEN(VAR ARGSTK:POSBUF;I,J:INTEGER);
VAR
  TEMP:XSTRING;
BEGIN
  IF(J-I>1)THEN BEGIN
    CSCOPY(EVALSTK,ARGSTK[I+2],TEMP);
    PBNUM(XLENGTH(TEMP))
  END
  ELSE
    PBNUM(0)
END;
  

PROCEDURE DOSUB(VAR ARGSTK:POSBUF;I,J:INTEGER);
VAR
  AP,FC,K,NC:INTEGER;
  TEMP1,TEMP2:XSTRING;
BEGIN
  IF(J-I>=3) THEN BEGIN
    IF(J-I<4) THEN
      NC:=MAXTOK
    ELSE BEGIN
      CSCOPY(EVALSTK,ARGSTK[I+4],TEMP1);
      K:=1;
      NC:=EXPR(TEMP1,K)
    END;
    CSCOPY(EVALSTK,ARGSTK[I+3],TEMP1);
    AP:=ARGSTK[I+2];
    K:=1;
    FC:=AP+EXPR(TEMP1,K)-1;
    CSCOPY(EVALSTK,AP,TEMP2);
    IF(FC>=AP) AND (FC<AP+XLENGTH(TEMP2)) THEN BEGIN
      CSCOPY(EVALSTK,FC,TEMP1);
      FOR K:=FC+MIN(NC,XLENGTH(TEMP1))-1 DOWNTO FC DO
        PUTBACK(EVALSTK[K])
      END
    END
  END;
  
  PROCEDURE DOCHQ(VAR ARGSTK:POSBUF;I,J:INTEGER);
  VAR
    TEMP:XSTRING;
    N:INTEGER;
  BEGIN
    CSCOPY(EVALSTK,ARGSTK[I+2],TEMP);
    N:=XLENGTH(TEMP);
    IF(N<=0)THEN BEGIN
      LQUOTE:=ORD(LESS);
      RQUOTE:=ORD(GREATER)
    END
    ELSE IF (N=1) THEN BEGIN
      LQUOTE:=TEMP[1];
      RQUOTE:=LQUOTE
    END
    ELSE BEGIN
      LQUOTE:=TEMP[1];
      RQUOTE:=TEMP[2]
    END
  END;
  
  
PROCEDURE EVAL(VAR ARGSTK:POSBUF;TD:STTYPE;
  I,J:INTEGER);
VAR
  ARGNO,K,T:INTEGER;
  TEMP:XSTRING;
BEGIN
  T:=ARGSTK[I];
  IF(TD=DEFTYPE)THEN
    DODEF(ARGSTK,I,J)
  ELSE IF (TD=EXPRTYPE)THEN
    DOEXPR(ARGSTK,I,J)
  ELSE IF (TD=SUBTYPE) THEN
    DOSUB(ARGSTK,I,J)
  ELSE IF (TD=IFTYPE) THEN
    DOIF(ARGSTK,I,J)
  ELSE IF (TD=LENTYPE) THEN
    DOLEN(ARGSTK,I,J)
  ELSE IF (TD=CHQTYPE) THEN
    DOCHQ(ARGSTK,I,J)
  ELSE BEGIN
    K:=T;
    WHILE(EVALSTK[K]<>ENDSTR) DO
      K:=K+1;
    K:=K-1;
    WHILE(K>T) DO BEGIN
      IF(EVALSTK[K-1] <> ARGFLAG) THEN
        PUTBACK(EVALSTK[K])
      ELSE BEGIN
        ARGNO:=ORD(EVALSTK[K])-ORD('0');
        IF(ARGNO>=0) AND (ARGNO <J-I)THEN BEGIN
          CSCOPY(EVALSTK,ARGSTK[I+ARGNO+1],TEMP);
          PBSTR(TEMP)
        END;
        K:=K-1
      END;
      K:=K-1
    END;
    IF(K=T)THEN
      PUTBACK(EVALSTK[K])
    END
  END;
PROCEDURE INITMACRO;
  BEGIN
    NULL[1]:=ENDSTR;
      DEFNAME[1]:=ORD('d');
      DEFNAME[2]:=ORD('e');
      DEFNAME[3]:=ORD('f');
      DEFNAME[4]:=ORD('i');
      DEFNAME[5]:=ORD('n');
      DEFNAME[6]:=ORD('e');
      DEFNAME[7]:=ENDSTR;
      SUBNAME[1]:=ORD('s');
      SUBNAME[2]:=ORD('u');
      SUBNAME[3]:=ORD('b');
      SUBNAME[4]:=ORD('s');
      SUBNAME[5]:=ORD('t');
      SUBNAME[6]:=ORD('r');
      SUBNAME[7]:=ENDSTR;
      EXPRNAME[1]:=ORD('e');
      EXPRNAME[2]:=ORD('x');
      EXPRNAME[3]:=ORD('p');
      EXPRNAME[4]:=ORD('r');
      EXPRNAME[5]:=ENDSTR;
      IFNAME[1]:=ORD('i');
      IFNAME[2]:=ORD('f');
      IFNAME[3]:=ORD('e');
      IFNAME[4]:=ORD('l');
      IFNAME[5]:=ORD('s');
      IFNAME[6]:=ORD('e');
      IFNAME[7]:=ENDSTR;
      LENNAME[1]:=ORD('l');
      LENNAME[2]:=ORD('e');
      LENNAME[3]:=ORD('n');
      LENNAME[4]:=ENDSTR;
      CHQNAME[1]:=ORD('c');
      CHQNAME[2]:=ORD('h');
      CHQNAME[3]:=ORD('a');
      CHQNAME[4]:=ORD('n');
      CHQNAME[5]:=ORD('g');
      CHQNAME[6]:=ORD('e');
      CHQNAME[7]:=ORD('q');
      CHQNAME[8]:=ENDSTR;
    BP:=0;
    INITHASH;
    LQUOTE:=ORD('`');
    RQUOTE:=ORD('''')
  END;
  
      

  
BEGIN
  INITMACRO;
  INSTALL(DEFNAME,NULL,DEFTYPE);
  INSTALL(EXPRNAME,NULL,EXPRTYPE);
  INSTALL(SUBNAME,NULL,SUBTYPE);
  INSTALL(IFNAME,NULL,IFTYPE);
  INSTALL(LENNAME,NULL,LENTYPE);
  INSTALL(CHQNAME,NULL,CHQTYPE);
  
  CP:=0;AP:=1;EP:=1;
  
  WHILE(GETTOK(TOKEN,MAXTOK)<>ENDFILE)DO
    IF(ISLETTER(TOKEN[1]))THEN BEGIN
      IF(NOT LOOKUP(TOKEN,DEFN,TOKTYPE))THEN
        PUTTOK(TOKEN)
      ELSE BEGIN
        CP:=CP+1;
        IF(CP>CALLSIZE)THEN
          ERROR('MACRO:CALL STACK OVERFLOW');
        CALLSTK[CP]:=AP;
        TYPESTK[CP]:=TOKTYPE;
        AP:=PUSH(EP,ARGSTK,AP);
        PUTTOK(DEFN);
        PUTCHR(ENDSTR);
        AP:=PUSH(EP,ARGSTK,AP);
        PUTTOK(TOKEN);
        PUTCHR(ENDSTR);
        AP:=PUSH(EP,ARGSTK,AP);
        T:=GETTOK(TOKEN,MAXTOK);
        PBSTR(TOKEN);
        IF(T<>LPAREN)THEN BEGIN
          PUTBACK(RPAREN);
          PUTBACK(LPAREN)
        END;
        PLEV[CP]:=0
      END
    END
    ELSE IF(TOKEN[1]=LQUOTE) THEN BEGIN
      NLPAR:=1;
      REPEAT
        T:=GETTOK(TOKEN,MAXTOK);
        IF(T=RQUOTE)THEN
          NLPAR:=NLPAR-1
        ELSE IF (T=LQUOTE)THEN
          NLPAR:=NLPAR+1
        ELSE IF (T=ENDFILE) THEN
          ERROR('MACRO:MISSING RIGHT QUOTE');
        IF(NLPAR>0) THEN
          PUTTOK(TOKEN)
      UNTIL(NLPAR=0)
    END
    ELSE IF (CP=0)THEN
      PUTTOK(TOKEN)
    ELSE IF (TOKEN[1]=LPAREN) THEN BEGIN
      IF(PLEV[CP]>0)THEN
        PUTTOK(TOKEN);
      PLEV[CP]:=PLEV[CP]+1
    END
    ELSE IF (TOKEN[1]=RPAREN)THEN BEGIN
      PLEV[CP]:=PLEV[CP]-1;
      IF(PLEV[CP]>0)THEN
        PUTTOK(TOKEN)
      ELSE BEGIN
        PUTCHR(ENDSTR);
        EVAL(ARGSTK,TYPESTK[CP],CALLSTK[CP],AP-1);
        AP:=CALLSTK[CP];
        EP:=ARGSTK[AP];
        CP:=CP-1
      END
    END
    ELSE IF (TOKEN[1]=COMMA) AND (PLEV[CP]=1)THEN BEGIN
      PUTCHR(ENDSTR);
      AP:=PUSH(EP,ARGSTK,AP)
    END
    ELSE
      PUTTOK(TOKEN);
  IF(CP<>0)THEN
    ERROR('MACRO:UNEXPECTED END OF INPUT')
END;





