5 NULL$ = CHR$(0) :print "<clear><11 dn>","PLEASE WAIT" : TI$ = "000000" : PRINT ,
10 DIM CD$(7) : FOR Y = 0 TO 7 : FOR X = 1 TO 255 : CD$(Y) = CD$(Y) + NULL$ : NEXT
11 PRINT TI$;"<6 bs>"; : NEXT
12 DEF FN C(V) = VAL(MID$(EN$(V), X, 1))
13 DEF FN H(X) = X - 48 + 7 * ((X - 48)>9)
14 DEF FN V(V) = VAL(MID$(EN$(V), X - Y + 1, Y - 1))
15 DIM ED$(200) : FOR X = 1 TO 200 : ED$(X) = "@VPN\" : NEXT
16 DIM LA$(100), LA(100)
17 LN = 1 : GOTO 100
18 FOR X = 1 TO LEN(ED$(PN)) : Y = 0
19 IF MID$(ED$(PN), X, 1) <> "@" THEN PRINT MID$(ED$(PN), X, 1); : NEXT : RETURN
20 X = X + 1 : OP$ = MID$(ED$(PN), X, 1) : IF OP$ <> "R" THEN 26
21 X$ = "@" : GOSUB 98
22 C = FN C(PN) : Y = 0
23 X$ = "@" : GOSUB 98
24 C$ = MID$(ED$(PN), X - Y + 1, Y - 1)
25 FOR Y = 1 TO C : PRINT C$; : NEXT Y, X : RETURN
26 IF OP$ <> "V" THEN 30
27 X$ = "\" : GOSUB 98 : X$ = MID$(ED$(PN), X - Y + 1, Y - 1)
28 IF X$ = "PN" THEN PRINT PN; : NEXT : RETURN
29 IF X$ = "EN" THEN C = 49 - POS(0) : C$ = " " : GOTO 25
30 IF OP$ <> "C" THEN 32
31 X = X + 1 : COLOR FN C(PN) : NEXT : RETURN
32 IF OP$ <> "T" THEN 35
33 X$ = "@" : GOSUB 98
34 PRINT TAB(FN V(PN)); : NEXT : RETURN
35 IF OP$ <> "^" THEN 37
36 PRINT : NEXT : RETURN
37 IF OP$ <> "P" THEN NEXT : RETURN
38 X$ = ", " : GOSUB 98
39 C = FN V(PN) : Y = 0
40 X$ = "!" : GOSUB 98
41 C1 = FN V(PN)
42 PRINT "<home>; : FOR C2 = 1 TO C : PRINT "<left>" : NEXT
43 FOR C2 = 1 TO C1 : PRINT "<down>"; : NEXT
50 NEXT : RETURN
51 REM INLINE
52 print " : "; : pn ln : gosub 18 : print "$<bs>";
53 FOR XL = 1 TO 80
54 GET A$ : IF A$ = "" THEN 54
55 OP = ASC(A$) : IF OP<32 THEN 57
56 LOCATE ,,,0 : PRINT " ";CHR$(8); ED$(LN) = ED$(LN) + A$ : PRINT A$;"$<bs>"; : NEXT : GOTO 112
57 PRINT A$;"$<bs>"; : IF OP = 13 THEN PRINT "  " : ED$(PN) = ED$(PN) + " " : XL = 81 : GOTO 112
58 IF OP = 29 THEN A$ = MID$(ED$(PN), X, 1) : GOTO 56
59 IF OP = 20 THEN ED$(PN) = LEFT$(ED$(PN), LEN(ED$(PN)) - 1) : XL = XL - 2 : NEXT : GOTO 112
60 IF OP = 19 THEN XL = 81 : GOTO 115
61 XL = XL - 1 : NEXT : GOTO 112
95 X = X + 1 : Y = Y + 1 : CK$ = MID$(C$, X, 1) : IF CK$ <> X$ AND X<LEN(C$) AND CK$ <> ";" THEN 95
96 X% = (X = LEN(C$)) : RETURN
98 C$ = ED$(PN) : GOTO 95
100 REM DISPLAY LOOP
105 PRINT "<clear>LINE : ";LN
106 FOR PN = LN - 10 TO LN + 10
107 IF PN<1 THEN PRINT : NEXT : GOTO 52
108 IF PN = LN THEN PRINT " * ";
109 GOSUB 18 : PRINT : NEXT
110 GOTO 52
112 LN = LN + 1 : GOTO 105
115 REM COMMAND ENTRY
116 PRINT "<clear>COMMAND MODE" : PRINT "<down>"; : INPUT "COMMAND : ";C$ : X = 0
117 O2 = 0 : X$ = " " : GOSUB 95 : M$ = LEFT$(C$, 1)
118 IF X% THEN 500
119 O1 = X : X$ = " - " : GOSUB 95 : IF X% THEN 121
120 O2 = VAL(MID$(C$, X + 1))
121 O1 = VAL(MID$(C$, O1))
122 IF M$ = "M" THEN 130
123 IF M$ = "D" THEN 137
124 IF M$ = "I" THEN 143
125 IF M$ = "P" THEN 175
126 IF M$ = "S" THEN 190
127 IF M$ = "L" THEN 205
128 IF M$ = "W" THEN 220
129 PRINT "<reverse><white>", , "INVALID COMMAND" : FOR X = 1 TO 500 : NEXT : GOTO 116
130 IF O2 = 0 THEN LN = O1 : GOTO 100
131 IF (O2 - O1)<21 THEN LN = INT(O1 + (O2 - O1)/2 + .5) : GOTO 500
132 FOR PN = O1 TO O2
133 IF PN = LN THEN PRINT " * ";
134 GOSUB 18 : PRINT : FOR Z = 1 TO 150 : NEXT Z, PN
135 INPUT "RETURN TO ENTER MODE";C$
136 GOTO 100
137 IF O2 = 0 THEN O2 = O1
138 O2 = O2 + 1 : DI = O2 - O1 : FOR X = O2 TO 200
139 ED$(X - DI) = ED$(X) : NEXT
140 FOR X = 200 - DI TO 200
142 ED$(X) = "@VPN\" : NEXT : GOTO 100
143 IF O2 = 0 THEN O2 = O1
144 O2 = O2 + 1 : DI = O2 - O1 : FOR X = 200 - DI TO O1 STEP - 1
145 ED$(X + DI) = ED$(X) : NEXT
146 FOR X = O1 TO O2 - 1 : ED$(X) = "@VPN\" : NEXT
147 GOTO 100
400 X = 2 : X$ = "\" : GOSUB 98 : Y = 0 : X$ = " " : GOSUB 98 : IF Y>5 OR Y<2 THEN LABEL$ = "" : GOTO 406
402 LABEL$ = MID$(ED$(PN), X - Y + 1, Y - 1)
404 PRINT LABEL$, CP
406 IF LABEL$ <> "" THEN LA$(LP) = LABEL$ : LA(LP) = CP : LP = LP + 1
407 IF LEN(MN$) = 3 AND LEFT$(MN$, 1) = "B" THEN CP = CP + 1 - (SZ$ = "L")
408 CP = CP - (MN$ = "ABCD") - (MN$ = "ADDQ") - (MN$ = "ADDX") - (MN$ = "CMPM") - (MN$ = "EXG")
409 CP = CP - (MN$ = "EXT") - (MN$ = "ILLEGAL") - (MN$ = "MOVEQ") - (MN$ = "NOP") - (MN$ = "RESET")
410 CP = CP - (MN$ = "RTE") - (MN$ = "RTR") - (MN$ = "RTS") - (MN$ = "SBCD") - (MN$ = "SUBQ")
411 CP = CP - (MN$ = "SUBX") - (MN$ = "SWAP") - (MN$ = "TRAP") - (MN$ = "TRAPV") - (MN$ = "UNLK")
412 CP = CP - (RIGHT$(MN$, 1) = "I") * (2 - (SZ$ = "L"))
413 CP = CP - 2 * ((LEFT$(MN$, 2) = "DB") + (MN$ = "LINK") + (MN$ = "MOVEC") + (MN$ = "MOVEM"))
414 CP = CP - 2 * ((MN$ = "MOVEP") + (MN$ = "RTD") + (MN$ = "STOP"))
415 IF MN$ = "EQU" THEN GOSUB 700 : LA(LP - 1) = A : RETURN
416 IF MN$ = "END" THEN RETURN
417 IF MN$ = "DFS" THEN GOSUB 700 : CP = CP + A : RETURN
418 IF LEFT$(MN$, 2) = "DF"THEN GOSUB 875 : CP = CP + A : RETURN
419 X = 0 : X$ = "CCR" : GOSUB 98 : IF NOT X% THEN CP = CP + 2 : RETURN
420 X = 0 : X$ = "#" : GOSUB 98 : IF NOT X% THEN CP = CP + 1 - (SZ$ = "L")
421 X = 0 : X$ = "SR" : GOSUB 98 : IF NOT X% THEN CP = CP + 2 : RETURN
422 X = 0 : X$ = "USP" : GOSUB 98 : IF NOT X% THEN CP;CP + 1 : RETURN
423 X = 0 : X$ = " " : GOSUB 98 : GOSUB 98 : GOSUB 98 : AF = X + 1 : FP = 0
424 X$ = ") + " : GOSUB 98 : IF NOT (X% OR FP) THEN FP = - 1 : AF = X + 3 : GOTO 424
425 X = AF : X$ = " - (" : GOSUB 98 : IF NOT (X% OR FP) THEN FP = - 1 : AF = X + 6 : GOTO 424
426 X = AF : X$ = "(PC)" : GOSUB 98 : IF NOTX%THENCP = CP + 1 : IF NOTFPTHENFP = - 1 : AF = X + 5 : GOTO 424
427 X = AF : X$ = "(PC, " : GOSUB 98 : IF NOTX%THENCP = CP + 1 : IF NOTFPTHENFP = - 1 : AF = X + 10 : GOTO 424
428 X = AF : X$ = "(A" : GOSUB 98 : IF X% THEN 432
429 X = X + 2 : CP = CP - (MID$(EN(PN), X, 1) = ")") - (MID$(EN$(PN), X, 1) = ", ") : X = X + 2 : AF = AF + 4
430 IF MID$(EN$(PN), X - 1, 1) <> ", " THEN AF = AF + 6
431 IF NOT FP THEN FP = - 1 : GOTO 424
432 X = AF : X$ = "(A" : GOSUB 98 : IF NOT (X% OR FP) THEN FP = - 1 : AF = AF + 5 : GOTO 424
433 X = AF : A$ = "A" : GOSUB 98 : IF NOT (X% OR FP) THEN FP = - 1 : AF = AF + 3 : GOTO 424
434 X = AF : X$ = "D" : GOSUB 98 : IF NOT (X% OR FP) THEN FP = - 1 : AF = AF + 3 : GOTO 424
435 X = AF : X$ = ".W" : GOSUB 98 : IF NOTX%THENCP = CP + 1 : IF NOTFPTHENAF = AF + 3 : GOTO 424
436 X = AF : X$ = ".L" : GOSUB 98 : IF NOTX%THENCP = CP + 2 : IF NOTFPTHENAF = AF + 3 : GOTO 424
437 CP = CP + 1 : RETURN
450 V% = 0 : X = 0 : Y = 0 : X$ = ";" : GOSUB 98 : IF (X<8) AND (NOT X%) THEN V% = - 1 : F% = - 1 : RETURN
451 X = 0 : Y = 0 : X$ = " " : GOSUB 98 : Y = 0 : GOSUB 98
452 MN$ = MID$(ED$(PN), X - Y + 1, Y - 1) : PRINT ED$(PN), MN$
453 IF MN$ = "ORG" THEN O% = - 1 : GOSUB 700 : CP = A
454 IF NOT O% THEN E% = 0 : V% = - 1 : RETURN
455 IF LEN(MN$)>3 THEN 460
456 V% = (MN$ = "ORG") + (MN$ = "OBJ") + (MN$ = "EQU") + (MN$ = "END") + (MN$ = "DFB") + (MN$ = "EXG")
457 V% = V% + (MN$ = "DFW") + (MN$ = "DFL") + (MN$ = "DFS") + (MN$ = "LEA") + (MN$ = "PEA") : E% = 1
458 V% = V% + (MN$ = "JMP") + (MN$ = "JSR") + (MN$ = "NOP") + (LEFT$(MN$, 1) = "S")
459 V% = V% + (MN$ = "RTD") + (MN$ = "RTE") + (MN$ = "RTR") + (MN$ = "RTS") + (MN$ = "TAS") : RETURN
460 SZ$ = "W" : SZ$ = MID$(ED$(PN), X, 1)
461 MN$ = LEFT$(MN$, LEN(MN$) - 2) : V = 0
462 V = (LEFT$(MN$, 1) = "B") * - (LEN(MN$) = 3) * - ((SZ$ = "S") + (SZ$ = "L"))
463 V = V + (MN$ = "ABCD") * - (SZ$ = "B")
464 V = V + (MN$ = "ADD") + (MN$ = "ADDI") + (MN$ = "ADDQ") + (MN$ = "ADDX")
465 V = V + (MN$ = "ADDA") * - (SZ$ <> "B")
466 V = V + (MN$ = "AND") + (MN$ = "ANDI")
467 V = V + (MN$ = "ASL") + (MN$ = "ASR")
468 V = V + (MN$ = "BTST") * - (SZ$ = "W")
469 V = V + (MN$ = "BSR") * - ((SZ$ = "S") + (SZ$ = "L"))
470 V = V + (MN$ = "BSET") * - (SZ$ <> "W")
471 V = V + (MN$ = "BRA") * - ((SZ$ = "S") + (SZ$ = "L"))
472 V = V + (MN$ = "BCLR") * - (SZ$ <> "W")
473 V = V + (MN$ = "BCHG") * - (SZ$ <> "W")
474 V = V + (MN$ = "CHK") * - (SZ$ = "W") + (MN$ = "ROR") + (MN$ = "ROXL") + (MN$ = "ROXR")
475 V = V + (MN$ = "CLR") + (MN$ = "OR") + (MN$ = "ORI") + (MN$ = "RESET") + (MN$ = "ROL")
476 V = V + (MN$ = "CMP") + (MN$ = "CMPI") + (MN$ = "CMPM") + (MN$ = "SBCD")
477 V = V + (MN$ = "CMPA") * - (SZ$ <> "B")
478 V = V + (LEFT$(MN$, 1) = "D") * - (SZ$ = "W")
479 V = V + (MN$ = "EOR") + (MN$ = "EORI")
480 V = V + (MN$ = "EXT") * - (SZ$ <> "B")
481 V = V + (MN$ = "ILLEGAL") + (MN$ = "LINK")
482 V = V + (MN$ = "LSL") + (MN$ = "LSR")
483 V = V + (MN$ = "MOVE") + (MN$ = "MOVEC") + (MN$ = "MOVEQ") + (MN$ = ""MOVES")
484 V = V + ((MN$ = "MOVEA") + (MN$ = "MOVEM") + (MN$ = "MOVEP")) * - (SZ$ <> "B")
485 V = V + ((MN$ = "MULU") + (MN$ = "MULS")) * - (SZ$ = "W") + (MN$ = "SWAP")
486 V = V + (MN$ = "NBCD") + (MN$ = "NEG") + (MN$ = "NEGX") + (MN$ = "NOT")
487 V = V + (MN$ = "SUB") + (MN$ = "SUBI") + (MN$ = "SUBQ") + (MN$ = "SUBA") * - (SZ$ <> "B")
488 V = V + (MN$ = "TRAP") + (MN$ = "TRAPV") + (MN$ = "UNLK")
489 V% = V : E% = 2 : LN = PN
490 RETURN
500 IF C$ <> "A" THEN END
505 REM PASS ONE
506 E% = 0 : PN = 1 : LP = 1
507 IF ED$(PN) = "@VPN\" THEN 514
508 GOSUB 450 : IF NOT V% THEN 800 : 
510 IF NOT F% THEN GOSUB 400 : IF NOT V% THEN 800 : 
512 IF MN$ = "END") THEN 516
514 PN = PN + 1 : GOTO 508
516 PRINT "END OF PASS ONE"
518 PN = 1 : CP = 0
519 IF ED$(PN) = "@VPN\" THEN PN = PN + 1 : GOTO 519
520 GOSUB 550 : IF NOT V% THEN 800
521 IF F% THEN GOTO 528
522 GOSUB 600 : IF NOT V% THEN 800
524 GOSUB 650 : IF NOT V% THEN 800
526 GOSUB 700 : IF NOT V% THEN 800
528 IF MN$ <> "END" THEN PN = PN + 1 : GOTO 519
530 PRINT "ASSEMBLY COMPLETE"
532 GOTO 116
700 Y = 0 : X$ = ";" : GOSUB 98 : IF NOT X% THEN C$ = MID$(ED$(PN), X - Y + 1, Y - 1) : GOTO 704
702 C$ = MID$(ED$(PN), X - Y + 1)
704 Y = 0 : GOSUB 98 : C1$ = MID$(ED$(PN), X - Y + 1, Y - 1)
706 FOR X = 1 TO LEN(C1$) : C$ = MID$(C1$, X, 1) : GOSUB 900 : NEXT : RETURN
720 REM TRY HEX DATA
722 X = 0 : Y = 0 : X$ = "$" : GOSUB 98 : IF X% THEN 730 : REM NADA, DECIMAL, OR OCTAL
724 Y = 0 : C = X : X$ = ", " : GOSUB 98 : IF NOT X% THEN 728
726 Y = 0 : X = C : X$ = " " : GOSUB 98
728 Y = Y - 1 : A = 0 : FOR Z = X - Y + 1 TO X - 1 : A = A * 16 + FN H(ASC(MID$(ED$(PN), Z, 1))) : NEXT
729 RETURN
900 PP% = (CP - OR)/255 : OP% = (CP - OR) - PP% * 255
901 CD$(PP%) = MID$(CD$(PP%), 1, OP% - 1) + C$ + MID$(CD$(PP%), OP% + 1)
902 CP = CP + 1 : RETURN
905 PP% = (CP - OR)/255 : OP% = (CP - OR) - PP% * 255
906 CD$(PP%) = MID$(CD$(PP%), 1, OP% - 1) + CHR$(C) + MID$(CD$(PP%), OP% + 1)
907 CP = CP + 1 : RETURN
