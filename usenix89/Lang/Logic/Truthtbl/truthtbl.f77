C
C	FUNCTION:	Generates truth tables for boolean expressions.
C
C
C	HISTORY:
C	V1.1		20 January 1986		STH
C	Changed to ** in loop expression.
C	V1		16 June 1984		Steve Hand - original
C
C
C	ENVIRONMENT:	This program completely conforms to standard
C			FORTRAN-77, ANSI X3.9-1978.
C
C			This program is compiled as a single module.
C
C			This program runs interactively, and uses
C			logical unit number 5 for input.
C
C			Imbedded form feed and tab characters may cause
C			errors with some compilers.  Tabs are assumed to
C			be every 8 characters (9, 17, 25, etc.).
C
C
C	STRUCTURE:
C
C				   +------ SCAN
C			MAIN ------+
C				   +------ MYLEN
C
C
C	NOTE:		Converted from the BASIC program in
C			Electronics Design News, 23 June 1982, P.200
C

 130  INTEGER   P(80), S(80), V(26), B(26), P1, S1, B1, I, J, X1
      INTEGER   C1, C2, MYLEN, EVAL
      CHARACTER E*80, X*1, Z*1
      CHARACTER OUT*80, DASH*50

      EVAL = 1
      DASH = '--------------------------------------------------'

 150  PRINT *, '                   Boolean Algebra'
      PRINT *
 160  PRINT *, 'Generates truth tables for boolean expressions!'
      PRINT *
      PRINT *, 'All input must be in UPPER CASE.'
      PRINT *
 170  PRINT *, 'Instructions? (Y/N)'

 180  READ ( 5, 8000 ) Z
 8000 FORMAT ( A1 )

 190  IF ( Z .EQ. 'N' ) GOTO 360
 200  IF ( Z .EQ. 'Y' ) GOTO 220
 210  GOTO 170

 220  PRINT *
      PRINT *, 'Use any single letter as a variable, ',
     +'up to 26 variables.'
      PRINT *
 250  PRINT *, '            - stands for NOT'
 260  PRINT *, '            . stands for AND'
 270  PRINT *, '            @ stands for XOR'
 280  PRINT *, '            + stands for OR'
      PRINT *
 290  PRINT *, 'The operators are listed in decreasing order of'
      PRINT *, 'evaluation.  Use parenthesis (nested to any depth)'
 300  PRINT *, 'to specify the desired order.'
      PRINT *
 310  PRINT *, '         Example :  -A.(B+(H@I))'
      PRINT *
      PRINT *, '- Enter "?" to see the instructions again.'
      PRINT *, '- Enter "*" to toggle between showing all evaluations'
      PRINT *, '  and showing only true evaluations.'
       
 360  PRINT *
      PRINT *, 'Enter boolean expression:'
      READ ( 5, 8010 ) E
 8010 FORMAT ( A80 )

 370  IF ( E(1:1) .EQ. '?' ) GOTO 220

      IF ( E(1:1) .EQ. '*' ) THEN
         EVAL = -1 * EVAL
         GOTO 360
      ENDIF

 380  P1 = 0
      S1 = P1
      B1 = P1

 390  DO 9000 I = 1, 26
         V(I) = P1
         B(I) = P1
 9000 CONTINUE

 400  IF ( MYLEN(E) .EQ. 0 ) THEN
         PRINT *
         PRINT *, 'Program finished.'
         STOP
       ENDIF

 410  DO 550 I = 1, MYLEN(E)
 420     X = E(I:I)
         CALL SCAN ( X, X1 )
 430     IF ( X1 .GT. 5 ) GOTO 460
 440     IF ( X1 .EQ. -1 ) GOTO 550
 450     GOTO (470, 500, 510, 510, 510, 510), X1+1
 460     P1 = P1+1
         P(P1) = X1
         GOTO 550

 470     IF ( S1 .EQ. 0 ) THEN
            PRINT *, 'PARENTHESIS ERROR.'
            GOTO 360
         ENDIF

 480     IF ( S(S1) .EQ. 1 ) THEN
            S1 = S1-1
            GOTO 550
         ENDIF

 490     P1 = P1+1
         P(P1) = S(S1)
         S1 = S1-1
         GOTO 470
 500     S1 = S1+1
         S(S1) = X1
         GOTO 550
 510     IF ( S1 .EQ. 0 ) GOTO 540
 520     IF ( X1 .GT. S(S1) ) GOTO 540
 530     P1 = P1+1
         P(P1) = S(S1)
         S1 = S1-1
         GOTO 510
 540     S1 = S1+1
         S(S1) = X1
 550  CONTINUE

 560  IF ( S1 .EQ. 0 ) GOTO 590

 570  IF ( S(S1) .EQ. 1 ) THEN
         PRINT *, 'PARENTHESIS ERROR.'
         GOTO 360
      ENDIF

 580  P1 = P1+1
      P(P1) = S(S1)
      S1 = S1-1
      GOTO 560

 590  DO 610 I = 1, P1
 600     IF ( P(I) .GT. 64 ) THEN
            IF ( P(I) .LT. 91 ) V(P(I)-64) = P(I)-64
         ENDIF
 610  CONTINUE

      OUT = ' '

 630  DO 670 I = 1, 26
 640     IF ( V(I) .EQ. 0 ) GOTO 670
 650     B1 = B1+1
         V(I) = B1
         OUT = OUT(1:MYLEN(OUT)) // '  ' // CHAR(I+64)
 670  CONTINUE

      OUT = OUT(1:MYLEN(OUT)) // '  ' // 'ANSWER'

      PRINT *
      PRINT *, OUT(1:MYLEN(OUT))
      PRINT *, '  ' // DASH(3:MYLEN(OUT))

 690  DO 900 J = 1, 2**B1
 700     S1 = 0

         DO 820 I = 1, P1
 710        IF ( P(I) .LE. 5 ) GOTO 730
 720        S1 = S1+1
            S(S1) = B(V(P(I)-64))
            GOTO 820
 730        IF ( P(I) .EQ. 5 ) GOTO 800

 740        IF ( S1 .LT. 2 ) THEN
               PRINT *, 'ERROR IN FORMULA.'
               GOTO 360
            ENDIF

 750        GOTO (760, 770, 780), P(I)-1
 760        S(S1-1) = INT((S(S1-1)+S(S1)+1)/2)
            GOTO 790
 770        S(S1-1) = ABS(S(S1-1)-S(S1))
            GOTO 790
 780        S(S1-1) = INT((S(S1-1)+S(S1))/2)
 790        S1 = S1-1
            GOTO 820

 800        IF ( S1 .EQ. 0 ) THEN
               PRINT *, 'ERROR IN FORMULA.'
               GOTO 360
            ENDIF

 810        S(S1) = 1-S(S1)
 820     CONTINUE

 830     IF ( EVAL .EQ. -1 ) THEN
            IF ( S(1) .EQ. 0 ) GOTO 850
         ENDIF

         OUT = ' '

 840     DO 9010 I = 1, B1
            OUT = OUT(1:MYLEN(OUT)) // '  ' // CHAR(B(I)+48)
 9010    CONTINUE

         OUT = OUT(1:MYLEN(OUT)) // '    ' // CHAR(S(1)+48)
         PRINT *, OUT(1:MYLEN(OUT))

 850     IF ( S1 .NE. 1 ) THEN
            PRINT *, 'ERROR IN FORMULA.'
            GOTO 360
         ENDIF

 860     C1 = 1

 870     DO 9020 I = B1, 1, -1
 880        C2 = INT((C1+B(I))/2)
 890        B(I) = ABS(B(I)-C1)
            C1 = C2
 9020    CONTINUE

 900  CONTINUE

 910  GOTO 360
      END

      SUBROUTINE SCAN ( X, X1 )

      INTEGER   X1
      CHARACTER X*1

 920  X1 = -1

 930  IF ( X .EQ. '-' ) THEN
         X1 = 5
         GOTO 1000
      ENDIF

 940  IF ( X .EQ. '.' ) THEN
         X1 = 4
         GOTO 1000
      ENDIF

 950  IF ( X .EQ. '@' ) THEN
         X1 = 3
         GOTO 1000
      ENDIF

 960  IF ( X .EQ. '+' ) THEN
         X1 = 2
         GOTO 1000
      ENDIF

 970  IF ( X .EQ. '(' ) THEN
         X1 = 1
         GOTO 1000
      ENDIF

 980  IF ( X .EQ. ')' ) THEN
         X1 = 0
         GOTO 1000
      ENDIF

 990  IF ( ICHAR(X) .GT. 64 ) THEN
         IF ( ICHAR(X) .LT. 91 ) X1 = ICHAR(X)
      ENDIF

 1000 RETURN
      END

C
C	FUNCTION:	Returns the true length of a string.
C
C	NOTE:		In FORTRAN, strings which contain fewer characters
C			than the maximum are padded on the right with spaces.
C			This routine returns the position of the rightmost
C			non-blank character in the string, which relates
C			to the string's "true" length.
C

      FUNCTION MYLEN ( STRING )
      INTEGER   POS
      CHARACTER STRING*(*)

      DO 10 POS = LEN(STRING), 1, -1
         IF ( STRING(POS:POS) .NE. ' ' ) GOTO 20
 10   CONTINUE

 20   MYLEN = POS

      RETURN
      END

C
C  Here is the original BASIC program, which runs on a Commodore-64.
C  From Electronics Design News, 23 June 1982, P.200
C
C  It is provided for owners of BASIC machines, and for comparison with
C  the FORTRAN version.
C
C
C100 REM ENTER "?" FOR RETURN TO INSTRUCTIONS
C110 REM ADD LINE 830  TO RESTRICT OUTPUT TO ONLY TRUE EVALUATIONS.
C120 REM FROM ELECTRONICS DESIGN NEWS, 23-JUN-82, P.200
C130 DIMP(80),S(80),V(26),B(26)
C150 PRINT"BOOLEAN ALGEBRA"
C160 PRINT"WILL GENERATE A TRUTH TABLE FOR ANY BOOLEAN EQUATION!"
C170 PRINT"INSTRUCTIONS? Y/N"
C180 GET Z$:IFZ$=""THEN180
C190 IFZ$="N"THEN360
C200 IFZ$="Y"THEN220
C210 GOTO180
C220 PRINT"USE ANY SINGLE LETTER AS A VARIABLE - UP TO 26 VARIABLES."
C230 PRINT"  1 STANDS FOR TRUE"
C240 PRINT"  0 STANDS FOR FALSE"
C250 PRINT"  - STANDS FOR NOT"
C260 PRINT"  . STANDS FOR AND"
C270 PRINT"  @ STANDS FOR XOR"
C280 PRINT"  + STANDS FOR OR"
C290 PRINT"THE OPERATORS ARE LISTED IN DECREASING ORDEROF EVALUATION. USE PARENTHESIS ";
C300 PRINT"(NESTED TO   ANY DEPTH) TO SPECIFY THE DESIRED ORDER."
C310 PRINT"EXAMPLE: -A.(B+(H@I))"
C320 REM PRINT"   -A.(B+(H@I))"
C330 PRINT" HIT ANY KEY TO START"
C340 GETZ$:IFZ$=""THEN340
C360 INPUT"BOOLEAN EQUATION IS ";E$
C370 IFRIGHT$(E$,1)="?"THEN220
C380 P=0:S=P:B=P
C390 FORI=1TO26:V(I)=P:B(I)=P:NEXTI
C400 IFLEN(E$)=0THENSTOP
C410 FORI=1TOLEN(E$)
C420 X$=MID$(E$,I,1):GOSUB920
C430 IFX>5THEN460
C440 IFX=-1THEN550
C450 ONX+1GOTO470,500,510,510,510,510
C460 P=P+1:P(P)=X:GOTO550
C470 IFS=0THENPRINT"PARENTHESIS ERROR.":GOTO360
C480 IFS(S)=1THENS=S-1:GOTO550
C490 P=P+1:P(P)=S(S):S=S-1:GOTO470
C500 S=S+1:S(S)=X:GOTO550
C510 IFS=0THEN540
C520 IFX>S(S)THEN540
C530 P=P+1:P(P)=S(S):S=S-1:GOTO510
C540 S=S+1:S(S)=X
C550 NEXTI
C560 IFS=0THEN590
C570 IFS(S)=1THENPRINT"PARENTHESIS ERROR.":GOTO360
C580 P=P+1:P(P)=S(S):S=S-1:GOTO560
C590 FORI=1TOP
C600 IFP(I)>64THENIFP(I)<91THENV(P(I)-64)=P(I)-64
C610 NEXTI
C620 PRINT" ";
C630 FORI=1TO26
C640 IFV(I)=0THEN670
C650 B=B+1:V(I)=B
C660 PRINT CHR$(I+64);"  ";
C670 NEXTI
C680 PRINT"ANS."
C690 FORJ=1TOEXP(LOG(2)*B)
C700 S=0:FORI=1TOP
C710 IFP(I)<=5THEN730
C720 S=S+1:S(S)=B(V(P(I)-64)):GOTO820
C730 IFP(I)=5THEN800
C740 IFS<2THENPRINT"ERROR IN FORMULA.":GOTO360
C750 ONP(I)-1GOTO760,770,780
C760 S(S-1)=INT((S(S-1)+S(S)+1)/2):GOTO790
C770 S(S-1)=ABS(S(S-1)-S(S)):GOTO790
C780 S(S-1)=INT((S(S-1)+S(S))/2)
C790 S=S-1:GOTO820
C800 IFS=0THENPRINT"ERROR IN FORMULA.":GOTO360
C810 S(S)=1-S(S)
C820 NEXTI
C830 REM IFS(1)=0THEN850
C840 FORI=1TOB:PRINTB(I);"";:NEXTI:PRINT S(1)
C850 IFS<>1THENPRINT"ERROR IN FORMULA.":GOTO360
C860 C1=1
C870 FORI=BTO1STEP-1
C880 C2=INT((C1+B(I))/2)
C890 B(I)=ABS(B(I)-C1):C1=C2:NEXTI
C900 NEXTJ
C910 GOTO360
C920 X=-1
C930 IFX$="-"THENX=5:GOTO1000
C940 IFX$="."THENX=4:GOTO1000
C950 IFX$="@"THENX=3:GOTO1000
C960 IFX$="+"THENX=2:GOTO1000
C970 IFX$="("THENX=1:GOTO1000
C980 IFX$=")"THENX=0:GOTO1000
C990 IFASC(X$)>64THENIFASC(X$)<91THENX=ASC(X$)
C1000 RETURN
