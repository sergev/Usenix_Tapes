

10 REM MULTIPLE OUTPUT LOGIC MINIMIZER, R. MCDERMOTT, 2/6/85
20 CLEAR 5000:DEFINT A-Z
30 READ MW,OW:XX=2^MW
32 DIM MN$(XX),M1(XX),L1(MW+1,XX),ST$(32,1)
34 DIM MA(OW*2),MO$(XX),NM(OW*2)
38 DIM MK$(1),QX$(1),QS$(1),MM$(1)
40 MK$="*":SP$=" ":QX$(0)="0":QX$(1)="1":QS$(0)=" + ":                             QS$(1)=" & ":MM$(0)="MINTERMS":MM$(1)="MAXTERMS"
45 INPUT "ENTER 0 FOR SUM OF PRODUCTS, 1 FOR PRODUCT OF SUMS"; MM
50 REM READ MINTERMS INTO MN$; NM=# OF MINTERMS, M1(.)=# OF 1s,                    MW=WIDTH OF TERMS (# OF VAR.)
55 REM CMD "Z","ON"
60 ER=0:GOSUB 10000: IF ER=1 THEN PRINT "PROGRAM ABORT":STOP
61 REM CMD"Z","OFF"
62 X1=NM*MW/2:X2=NM*MW
64 DIM M(X1),T(X1),PI$(X2),PI(X2,2),T$(X1),L$(X2)
65 FOR CO=(2^OW)-1 TO 1 STEP -1:PRINT "COMBINATION ";CO:                           GOSUB 6700:REM DETERMINE VARIABLES INCLUDED IN THIS COMB.
70 REM PUT MN$,M1 INTO T$,T (TEMP ARRAYS)
90 GOSUB 11000:SC=0
92 IF T=0 THEN GOTO 150
95 PRINT MM$(MM);":":FOR I=1 TO T:PRINT T$(I):NEXT I
100 REM FIRST LOOP, FIND PRIME IMPLICANTS (PI$= SUBCUBE,                            PI(I,0)=COMBINATION #(CO),PI(I,1)=SUBCUBE SIZE
110 SC=SC+1:GOSUB 1000:REM CREATE ORDERED LIST (L)
120 T=0:IF LL=-1 THEN GOTO 150: REM CLEAR TEMP LIST
130 GOSUB 2000:GOSUB 2500: REM COMBINE TERMS IN THIS LIST, PUT                      COMBINED TERMS INTO T$, UNCOVERED TERMS INTO PI$
140 GOTO 100: REM LOOP TILL EMPTY
150 NEXT CO
151 REM CMD"Z","ON"
152 FOR I=1 TO NP:CO=PI(I,0):GOSUB 6700:PRINT PI$(I),
154 FOR J=1 TO OW:IF MA(J)=1 THEN PRINT J;
155 NEXT J:PRINT:NEXT I
160 REM CMD "Z","ON"
161 PRINT "CREATING PRIME IMPLICANT TABLE"
162 FOR I=1 TO X1:T$(I)="":L$(I)="":NEXT I:REM CLEAR STRING SPACE
165 DIM SP(NP),SM(TT),LM(TT),LP(NP),LJ(OW+1)
166 REM DATA STRUCTURE: SP(.)=SELECT MARK FOR PI;                                   SM(.)=SELECT MARK FOR MINTERM; LM(.)=POINTER TO MINTERM                         LP(.)=POINTER TO PI
170 GOSUB 6000:GOSUB 6500: REM PRINT PRIME IMPLICANT TABLE
200 GOSUB 9000: REM ELIMINATE DOMINANCE
300 REM FIND ESSENTIAL PRIME IMPLICANTS
305 IF LJ=0 THEN GOTO 600
310 GOSUB 7000
320 REM STOP IF NO EPIs FOUND
330 IF NE=0 THEN GOTO 600
340 GOSUB 8000:REM CREATE REDUCED PI TABLE
345 IF LJ=0 THEN GOTO 600
350 GOSUB 6500:REM PRINT IT OUT
360 GOSUB 9000:REM REMOVE DOMINATED TERMS
370 GOTO 300
600 REM NO EPIs FOUND, SEE IF WE NEED TO BRANCH AND BOUND
620 IF LJ=0 THEN GOTO 900:REM STOP IF ALL MINTERMS COVERED
630 INPUT "ENTER SELECTION (NUMERIC)";BB:PI(LP(BB),2)=1:            SP(BB)=1:REM PICK EACH PI
640 PRINT "SELECTING ";PI$(LP(BB));" ARBITRARILY"
650 FOR J=1 TO LJ:IF MID$(L$(BB),J,1)=MK$ THEN SM(J)=1: REM         MARK ALL MINTERMS COVERED BY THIS SELECTED PI
660 NEXT J
670 GOSUB 8000:GOSUB 9000:GOSUB 7000:REM REDUCE TABLE
680 IF NE=0 THEN GOTO 620
690 GOTO 670
900 REM DONE, PRINT OUT RESULTS
910 GOSUB 15000
920 REM CMD"Z","OFF":STOP
930 STOP
1000 REM CREATE ORDERED LIST, L1(K,0)= # OF TERMS OF SIZE K;          L1(K,I)=POINTER TO TERMS
1010 IF T=0 THEN LL=-1:RETURN
1020 FOR K=0 TO MW+1:L1(K,0)=0:NEXT K:LL=-1:REM CLEAR L SIZE
1030 FOR I=1 TO T:CT=L1(T(I),0)+1
1040 L1(T(I),0)=CT:L1(T(I),CT)=I:L$(I)=T$(I):    M(I)=0
1050 IF LL<T(I) THEN LL=T(I):REM LL= MAX SIZE (ROWS) OF L
1060 NEXT I:L1(LL+1,0)=0:RETURN
2000 REM COMBINE TERMS
2005 IF LL=-1 THEN RETURN
2006 PRINT "PRIME IMPLICANTS: ";
2010 FOR N1=0 TO LL:N2=N1+1:L1=N1:REM FOR EACH GROUP OF TERMS
2020 REM CHECK FOR EMPTY GROUP
2030 IF L1(N1,0)=0 THEN GOTO 2300
2040 REM CHECK EACH TERM IN GROUP WITH EACH TERM IN NEXT GROUP
2050 FOR I=1 TO L1(N1,0):L1$=L$(L1(N1,I)):IF L1(N2,0)=0 THEN         GOTO 2200: REM UNLESS NEXT GROUP IS EMPTY
2060 FOR J=1 TO L1(N2,0):L2$=L$(L1(N2,J))
2070 GOSUB 3000:REM COMPARE TERMS, IDENTIFY ELIMINATED VARIABLE      AS EV, 0 IF NONE.
2080 IF EV<>0 THEN M(L1(N1,I))=1:M(L1(N2,J))=1:GOSUB 4000:REM          CREATE NEW TERM
2090 NEXT J
2200 IF M(L1(N1,I))=0 THEN GOSUB 5000:REM ADD UNCOVERED TERMS        AS PIs
2210 NEXT I
2300 NEXT N1:PRINT
2310 RETURN
2500 REM PRINT NEW LIST OF TERMS
2505 IF T=0 THEN RETURN
2508 PRINT SC;"-SUBCUBES"
2510 FOR I=1 TO T:PRINT T$(I):NEXT I
2520 RETURN
3000 REM COMPARE TERMS IN L1$ AND L2$
3010 M1=0
3020 FOR CH=1 TO MW:C1$=MID$(L1$,CH,1):C2$=MID$(L2$,CH,1)
3030 IF C1$=C2$ THEN GOTO 3070
3040 IF C1$="-" OR C2$="-" THEN M1=0:CH=MW:GOTO 3070: REM DONT       BOTHER IF -s DONT MATCH
3050 M1=M1+1:EV=CH
3070 NEXT CH
3080 IF M1<>1 THEN EV=0:REM ELIMINATE ONLY IF ONE MISMATCH
3090 RETURN
4000 REM CREATE NEW TERM, L1$ CONTAINS ORIGINAL TERM, EV IDENTS      THE ELIMINATED VARIABLE
4010 T$=L1$:MID$(T$,EV,1)="-"
4020 T=T+1:T$(T)=T$:T(T)=L1
4030 IF T=1 THEN RETURN
4040 REM CHECK FOR REDUNDANCY
4050 FOR T1=1 TO T-1
4060 IF T$=T$(T1) THEN T=T-1:T1=T:REM MATCH DELETES THIS ADD.
4070 NEXT T1:RETURN
5000 REM ADD UNCOVERED TERMS AS PI, TERM IS IN L1$
5005 PRINT L1$;" ";
5010 FO=0:FOR XX=1 TO NP:IF L1$=PI$(XX) THEN FO=1:XX=NP
5020 NEXT XX:IF FO=1 THEN RETURN
5030 REM NEW PI NEEDED
5040 NP=NP+1:PI$(NP)=L1$:PI(NP,0)=CO:PI(NP,1)=SC:PI(NP,2)=0
5050 RETURN
6000 REM CREATE PI TABLE: USE L$ ARRAY:                              L$(PI)=STRING OF BLANKS OR STARS MARKING COVERED MINTERMS
6010 REM PUT PI'S DOWN ROWS
6020 GOSUB 12000:REM FIRST PUT PI'S IN SUBCUBE ORDER
6030 LI=NP
6040 FOR I=1 TO LI:LP(I)=I:NEXT I:REM LP=POINTER TO PI
6050 REM PUT MN'S ACROSS ROWS
6060 LJ=0:FOR V=1 TO OW:LJ(V)=LJ+1
6070 FOR J=1 TO NM:CH$=MID$(MO$(J),V,1)
6080 IF CH$<>"X" AND CH$<>QX$(MM) THEN LJ=LJ+1:LM(LJ)=J
6090 NEXT J:NEXT V:LJ(OW+1)=LJ+1
6092 OW$="":FOR V=1 TO OW:OW$=OW$+STRING$(LJ(V+1)-LJ(V),             CHR$(90-(OW-V))):NEXT V:REM GET TITLE OF OUTPUTS
6095 FOR I=1 TO LI:L$(I)=STRING$(LJ,SP$):SP(I)=0:NEXT I
6100 REM DETERMINE COVERAGE
6110 FOR I=1 TO LI:PI=LP(I):PI$=PI$(PI):CO=PI(PI,0):GOSUB 6700
6115 CO=0: REM CREATE NEW COVERAGE, BASED ON COVERED OUTPUTS         (I.E. NOT INCLUDING DON'T CARE COVERAGE)
6120 FOR V=1 TO OW:C1=0:C2=2^(V-1):IF MA(V)=0 THEN GOTO 6150
6130 FOR J=LJ(V) TO LJ(V+1)-1:MN$=MN$(LM(J)):GOSUB 6300:             MID$(L$(I),J,1)=M$:IF M=1 THEN C1=C2
6140 NEXT J
6150 CO=CO + C1 : REM CREATE NEW COVERAGE
6155 NEXT V
6160 PI(PI,0)=CO:NEXT I
6170 REM CLEAR COVERAGE
6180 FOR I=1 TO LI:SP(I)=0:NEXT I
6190 FOR J=1 TO LJ:SM(J)=0:NEXT J
6200 RETURN
6300 REM SEE IF PI$ COVERS MN$
6310 M=1:M$=MK$
6320 FOR CH=1 TO MW:P1$=MID$(PI$,CH,1):M1$=MID$(MN$,CH,1)
6330 IF P1$<>"-" AND P1$<>M1$ THEN M=0:M$=SP$:CH=MW:REM CLEAR        MARK FLAG IF NO MATCH
6340 NEXT CH:RETURN
6500 REM PRINT PI TABLE
6520 REM PRINT HEADING
6530 PRINT " ","P R I M E   I M P L I C A N T   T A B L E"
6535 PRINT "OUTPUTS",OW$
6540 PRINT "PRIME IMP.",MM$(MM)
6550 FOR CH=1 TO MW:PRINT " ",
6560 FOR J=1 TO LJ:PRINT MID$(MN$(LM(J)),CH,1);:NEXT J
6570 PRINT:NEXT CH
6580 REM PRINT TABLE
6590 FOR I=1 TO LI:PRINT " ";PI$(LP(I)),L$(I)
6610 NEXT I:PRINT:RETURN
6700 REM DETERMINE MASK FOR INTERSECTION, BASED ON COMBINATION,      CO
6710 CX=CO:MA(0)=0:FOR CH=1 TO OW: CR=CX-INT(CX/2)*2:                CX=INT(CX/2):MA(CH)=CR:MA(0)=MA(0)+CR: NEXT CH:RETURN
7000 REM FIND ESSENTIAL PIs
7010 NE=0:PRINT "FINDING ESSENTIAL PIs:",:IF LJ=0 THEN RETURN
7020 FOR J=1 TO LJ:NX=0:IF SM(J)=1 THEN GOTO 7055
7030 REM SCAN DOWN EACH PI
7040 FOR I=1 TO LI:IF MID$(L$(I),J,1)=MK$ THEN NX=NX+1:NI=I
7050 NEXT I:IF NX=1 THEN GOSUB 7060
7055 NEXT J:PRINT:RETURN
7060 REM ESSENTIAL PI FOUND:MARK NIth PI AND COVERED MINTERMS
7070 PI(LP(NI),2)=1:NE=NE+1:SP(NI)=1:PRINT PI$(LP(NI)),
7080 FOR J1=1 TO LJ:IF MID$(L$(NI),J1,1)=MK$ THEN SM(J1)=1:          REM MARK EACH MINTERM COVERED BY THIS PI
7090 NEXT J1:RETURN
8000 REM CREATE REDUCED PI TABLE
8005 IF LJ=0 THEN RETURN
8010 NI=0:NJ=0:PRINT "CREATING REDUCED PI TABLE"
8020 REM REMOVE SELECTED PIs
8030 FOR I=1 TO LI:IF SP(I)=1 THEN GOTO 8050
8040 IF MID$(L$(I),1,LJ)<>STRING$(LJ,SP$) THEN NI=NI+1:              L$(NI)=L$(I):LP(NI)=LP(I)
8050 NEXT I
8060 REM REMOVE COVERED MINTERMS
8070 FOR J=1 TO LJ:IF SM(J)=1 THEN GOTO 8090
8080 NJ=NJ+1:IF NJ=J THEN GOTO 8090
8085 FOR I=1 TO NI:X$=MID$(L$(I),J,1):MID$(L$(I),NJ,1)=X$:           NEXT I
8087 LM(NJ)=LM(J):X$=MID$(OW$,J,1):MID$(OW$,NJ,1)=X$
8090 NEXT J:OW$=MID$(OW$,1,NJ)+""
8100 LI=NI:LJ=NJ:IF LJ=0 THEN RETURN
8110 REM CLEAR COVERED MARKS  (AND TRUNCATE STRING)
8120 FOR I=1 TO LI:SP(I)=0:X$="":X$=MID$(L$(I),1,LJ):L$(I)="":       L$(I)=X$:NEXT I
8130 FOR J=1 TO LJ:SM(J)=0:NEXT J
8140 RETURN
9000 REM REMOVE DOMINATED TERMS
9005 IF LJ=0 THEN RETURN
9010 DD=0:PRINT "ELIMINATING DOMINANCE"
9020 FOR I1=1 TO LI-1:P1=PI(LP(I1),1):IF P1>=MW THEN NI=I1:           GOSUB 7060:DD=1:PRINT " SELECTED AS ZERO COST"
9030 FOR I2=I1+1 TO LI:IF SP(I2)=1 THEN GOTO 9200:REM COMPARE        EACH PI WITH OTHER PIS, UNLESS ALREADY SELECTED
9040 D1=0:D2=0:D3=0
9045 P2=PI(LP(I2),1):IF P1>P2 THEN D1=1:REM DON'T LET LOWER         ORDER SUBCUBE DOMINATE
9050 FOR J=1 TO LJ:L1$=MID$(L$(I1),J,1):L2$=MID$(L$(I2),J,1)
9060 IF L1$=MK$ AND L2$=SP$ THEN D1=1
9070 IF L1$=SP$ AND L2$=MK$ THEN D2=1
9080 IF L1$=MK$ AND L2$=MK$ THEN D3=1
9090 NEXT J
9095 IF D3=1 AND D1=0 AND D2=0 THEN D1=1:REM BOTH COVER              EXACTLY SAME TERMS, CHOSE 1ST
9100 IF D1=1 AND D2=1 THEN GOTO 9200:REM NEITHER DOMINATES
9110 IF D2=0 THEN DD=DD+1:SP(I2)=1: REM I2 DOMINATES
9120 IF D1=0 THEN DD=DD+1:SP(I1)=1: REM I2 DOMINATES
9200 NEXT I2:NEXT I1
9210 IF DD=0 THEN PRINT "NO CHANGE":RETURN
9220 GOSUB 8000:GOSUB 6500:RETURN:REM CREATE REDUCED TABLE
10000 REM READ MINTERMS
10020 READ MN$:IF MN$="END" THEN RETURN
10025 PRINT MN$,:IF MN$="STATES" THEN GOSUB 16000:GOTO 10020
10040 READ MO$:MO$=MID$(MO$,1,OW):PRINT MO$
10050 IF ER<>1 THEN GOSUB 10400:REM ADD MN$, MO$ TO OUTPUT LIST
10060 GOTO 10020
10400 REM ADD MINTERM TO LIST, AND OUTPUT TO LIST
10405 IF ST=1 THEN GOSUB 16100: REM EXPAND STATE INPUT/OUTPUT
10410 NM=NM+1:MN=NM:MN$(MN)=MN$:MO$(MN)=MO$
10420 N1=0:N2=0:FOR CH=1 TO MW:CH$=MID$(MN$,CH,1)
10430 IF CH$="1" THEN N1=N1+1
10435 IF CH$="-" THEN N2=N2+1:NX(N2)=CH
10440 NEXT CH:M1(MN)=N1
10450 IF N2=0 THEN RETURN
10455 FOR J=1 TO N2:C=-1:CX=NX(J):FOR L=0 TO 1
10460 FOR K=1 TO 2^(J-1):C=C+1:MN=NM+C:MN$(MN)=MN$(NM+K-1):           M1(MN)=M1(NM+K-1)+L:MO$(MN)=MO$(NM+K-1)
10465 MID$(MN$(MN),CX,1)=QX$(L)
10470 NEXT K
10475 NEXT L:NEXT J
10480 NM=NM+C:RETURN
10500 'REM ******* DATA STRUCTURE ******
10510 'MN$=CURRENT MN$
10520 'MN$(I) = 01 PATTERN FOR ITH MINTERM
10530 'M1(I) = # OF 1S IN MN$(I)
10540 'MN= CURRENT MINTERM #
10550 'MN(0,C)= # OF MINTERMS FOR COMBINATION C
10560 'MN(I,C)= MINTERM I (PTR TO MN$) OF COMB. C
10570 'MX(0,C),MX(I,C) -- SAME AS MN, EXCEPT FOR DONT CARES
10580 'NM= TOTAL # OF MINTERMS
10600 'PI$= CURRENT PI$
10610 'PI$(I)= -1-0 COMBINATION FOR THE ITH PRIME IMPLICANT
10620 'PI = CURRENT PI #
10630 'PI(I,0)=COMBINATION # FOR THE ITH PI
10640 'PI(I,1)=SUBCUBE SIZE OF ITH PI
10650 'PI(1,2)=SELECT MARK FOR ITH PI
10660 'NP=TOTAL # OF PIS
11000 REM CREATE T$
11005 T=0:TX=0
11010 FOR I=1 TO NM:M=1
11020 FOR CH=1 TO OW:IF MA(CH)=0 THEN GOTO 11050
11025 CH$=MID$(MO$(I),CH,1):IF CH$="X" THEN TX=TX+1:GOTO 11050
11040 IF CH$=QX$(MM) THEN M=0
11050 NEXT CH
11060 IF M=1 THEN T=T+1:T$(T)=MN$(I):T(T)=M1(I)
11070 NEXT I
11075 IF MA(0)=1 THEN TT=TT+T-TX
11080 RETURN
12000 REM SORT PI$ BY SUBCUBE SIZE: REM INSERTION SORT
12010 FOR I=2 TO NP:J=I
12020 A$=PI$(I):A(0)=PI(I,0):A(1)=PI(I,1):A(2)=PI(I,2)
12040 J=J-1:REM STEP DOWN THROUGH LIST
12050 IF J=0 THEN 12100
12060 IF PI(J,1)=>A(1) THEN 12100
12070 PI$(J+1)=PI$(J):PI(J+1,0)=PI(J,0):PI(J+1,1)=PI(J,1):            PI(J+1,2)=PI(J,2)
12080 GOTO 12040
12100 J1=J+1:PI$(J1)=A$:PI(J1,0)=A(0):PI(J1,1)=A(1):                  PI(J1,2)=A(2)
12110 NEXT I
12120 RETURN
15000 REM PRINT OUT RESULTS
15010 PRINT:PRINT "MINIMIZED EQUATIONS"
15015 SP=0:FOR I=1 TO NP:SP=SP+PI(I,2):NEXT I: REM COUNT              SELECTED PIS
15016 DIM L(OW,SP)
15020 FOR V=1 TO OW:L(V,0)=0:NEXT V: REM CLEAR L
15030 FOR PI=1 TO NP:IF PI(PI,2)=1 THEN GOSUB 15200: REM GATHER       SELECTED PIS TO THEIR APPROPRIATE OUTPUT
15040 NEXT PI
15050 GOSUB 15300:REM REMOVE REDUNDANT PIS WITHIN EACH OUTPUT
15060 GOSUB 15500:REM PRINTOUT EACH EQUATION
15070 RETURN
15200 REM FIND APPLICABLE OUTPUTS FOR THIS PI
15210 CO=PI(PI,0):GOSUB 6700
15220 FOR V=1 TO OW:IF MA(V)=1 THEN L(V,0)=L(V,0)+1:                  L(V,L(V,0))=PI:REM L(V,0)=COUNT OF PIS FOR THIS VARIABLE
15230 NEXT V
15240 RETURN
15300 REM REMOVE REDUNDANT PIS WITHIN V;  MULTIPLE MINIMIZATION      MAY RESULT IN REDUNDANT PIS BEING INCLUDED.
15310 FOR V=1 TO OW:IF L(V,0)=0 THEN GOTO 15380
15320 REM COMPARE EACH PI WITH EACH OTHER, USING SAME ALGORITHM       USED FOR PIS COVERING MINTERMS.
15330 FOR J1=1 TO L(V,0)-1:PI$=PI$(L(V,J1))
15340 FOR J2=J1+1 TO L(V,0):MN$=PI$(L(V,J2)):REM ONLY NEED TO       COMPARE WITH SUBSEQUENTS, SINCE SORTED BY SIZE.
15350 GOSUB 6300:IF M=1 THEN L(V,J2)=0:REM USE SAME ROUTINE
15360 NEXT J2:NEXT J1
15380 NEXT V:RETURN
15500 REM PRINT OUT EACH EQUATION
15510 FOR V=1 TO OW:V$=CHR$(90-(OW-V))
15520 IF L(V,0)=0 THEN GOTO 15590
15530 PRINT V$;" = ";
15540 PL$=""
15550 FOR J=1 TO L(V,0):PI=L(V,J):PI$=PI$(PI)
15560 IF PI<>0 THEN GOSUB 15800:PRINT PL$;PA$;:PL$=QS$(MM)
15570 NEXT J:PRINT
15590 NEXT V:RETURN
15800 REM GET VARIABLE NAMES FOR PI$ INTO PA$
15805 IF MM=1 THEN GOTO 15900
15810 PA$=""
15820 FOR CH=1 TO MW:CH$=MID$(PI$,CH,1)
15830 IF CH$="0" THEN PA$=PA$+CHR$(64+CH)+"'"
15840 IF CH$="1" THEN PA$=PA$+CHR$(64+CH)
15850 NEXT CH
15860 RETURN
15900 PA$="(":PB$=""
15910 FOR CH=1 TO MW:CH$=MID$(PI$,CH,1)
15930 IF CH$="0" THEN PA$=PA$+PB$+CHR$(64+CH):PB$="+"
15940 IF CH$="1" THEN PA$=PA$+PB$+CHR$(64+CH)+"'":PB$="+"
15950 NEXT CH:PA$=PA$+")"
15960 RETURN
16000 REM READ IN STATE ASSIGNMENTS
16005 INPUT"ENTER D, T, RS, OR JK IMPLEMENTATION OPTION";ST$
16010 ST=1:NS=0
16020 READ S1$:PRINT S1$,:IF S1$="END" THEN GOTO 16050
16030 NS=NS+1:ST$(NS,0)=S1$:READ ST$(NS,1):PRINT ST$(NS,1)
16040 GOTO 16020
16050 PRINT:IF ST$="JK" OR ST$="RS" THEN OW=OW+LEN(ST$(1,1)):         REM ADJUST OUTPUTS SIZE FOR J AND K INPUT PER STATE
16060 RETURN
16100 REM EXPAND STATE I/O
16110 REM DO INPUT; FIND STATE VARIABLE
16120 T1$="":FOR J=1 TO LEN(MN$):C1$=MID$(MN$,J,1)
16130 IF C1$<>"0" AND C1$<>"1" AND C1$<>"-" THEN GOSUB 16200
16140 T1$=T1$+C1$:NEXT J
16150 MN$=T1$:PRINT MN$,MO$:RETURN
16200 REM DO OUTPUT; FIND STATE VARIABLE
16210 O1$="":FOR K=1 TO LEN(MO$):C2$=MID$(MO$,K,1)
16220 IF C2$<>"0" AND C2$<>"1" AND C2$<>"X" THEN GOSUB 16300
16230 O1$=O1$+C2$:NEXT K
16240 MO$=O1$:RETURN
16300 REM GOT INPUT STATE IN C1$, OUTPUT STATE IN C2$, FIND           EXCITATION INPUT
16310 C3$="":REM GET ASSIGNMENT OF STATES
16320 CH$=C1$:GOSUB 16900:C1$=CH$
16330 CH$=C2$:GOSUB 16900:C2$=CH$
16340 REM DO EXCITATION
16350 IF ST$="D" THEN C3$=C2$:GOTO 16380:REM DFF, D=NEXT STATE
16360 FOR CH=1 TO LEN(C1$):GOSUB 16500:REM GET EXCITATION FOR         EACH Q OF STATE
16370 C3$=C3$+CX$:NEXT CH
16380 GOSUB 16400:C2$=C3$:RETURN
16400 REM CHECK OUTPUT FOR -S INSTEAD OF XS
16410 FOR CH=1 TO LEN(C3$)
16420 IF MID$(C3$,CH,1)="-" THEN MID$(C3$,CH,1)="X"
16430 NEXT CH:RETURN
16500 REM GET T, RS OR JK EXCITATION
16510 IF ST$="JK" THEN GOTO 16550
16515 IF ST$="RS" THEN GOTO 16800
16520 REM TOGGLE INPUT
16530 CX$="1":IF MID$(C1$,CH,1)=MID$(C2$,CH,1) THEN CX$="0":          GOTO 16538
16535 IF MID$(C1$,CH,1)="X" OR MID$(C1$,CH,1)="-" THEN GOSUB          16700
16538 IF MID$(C2$,CH,1)="X" OR MID$(C2$,CH,1)="-" THEN CX$="X"
16540 RETURN
16550 REM JK EXCITATION
16555 IF MID$(C1$,CH,1)="-" OR MID$(C1$,CH,1)="X" THEN                 GOTO 16650
16560 IF MID$(C1$,CH,1)="0" THEN GOTO 16590
16570 CX$="X1":IF MID$(C2$,CH,1)="1" THEN CX$="X0"
16580 GOTO 16680 :' CHECK FOR DON'T CARE NEXT STATE
16590 CX$="0X":IF MID$(C2$,CH,1)="1" THEN CX$="1X"
16600 GOTO 16680 : ' CHECK FOR DON'T CARE NEXT STATE
16650 REM DON'T CARE, OR UNKNOWN INPUT AS PRIOR STATE
16660 IF MID$(C2$,CH,1)="1" THEN CX$="10"
16670 IF MID$(C2$,CH,1)="0" THEN CX$="01"
16680 IF MID$(C2$,CH,1)="-" OR MID$(C2$,CH,1)="X" THEN CX$="XX"
16690 RETURN
16700 REM DON'T CARE PRIOR STATE ON TOGGLES NOT ALLOWED
16710 PRINT "CAN'T HAVE DON'T CARE PRIOR STATE FOR TOGGLES"
16720 ER=1
16730 RETURN
16800 REM RS FLIP FLOP IMPLEMENTATION
16803 CX$="XX"
16805 IF MID$(C2$,CH,1)="0" THEN CX$="10"
16810 IF MID$(C2$,CH,1)="1" THEN CX$="01"
16815 IF MID$(C1$,CH,1)="0" AND MID$(C2$,CH,1)="0" THEN               CX$="X0"
16820 IF MID$(C1$,CH,1)="1" AND MID$(C2$,CH,1)="1" THEN               CX$="0X"
16825 RETURN
16900 REM DO STATE ASSIGNMENT
16910 FOR JC=1 TO NS
16920 IF CH$=ST$(JC,0) THEN CH$=ST$(JC,1):JC=NS
16930 NEXT JC:RETURN
18000 DATA 5,5: REM SAMPLE INPUT -- PHASE COMPARATOR, FIG.9.21
18005 REM STATE ASSIGNMENT
18010 DATA STATES
18020 DATA A,0-0
18030 DATA B,001
18040 DATA C,-11
18050 DATA D,101
18060 DATA E,110
18070 DATA F,100
18080 DATA END
18090 REM  RA. DG...
18100 DATA 0-A,00A
18110 DATA 1-A,00B
18120 DATA 0-B,00F
18130 DATA 10B,00B
18140 DATA 11B,00C
18150 DATA 00C,00A
18160 DATA 01C,00E
18170 DATA 10C,00D
18180 DATA 11C,00C
18190 DATA 0-D,10A
18200 DATA 1-D,10D
18210 DATA -0E,01A
18220 DATA -1E,01E
18230 DATA -0F,01F
18240 DATA -1F,01E
18250 DATA END
19000 DATA 6,8: REM SAMPLE INPUT -- 3 WAY TRAFFIC CONTROLLER
19010 DATA STATES
19020 DATA A,000
19030 DATA B,001
19040 DATA C,011
19050 DATA D,010
19060 DATA E,11-
19070 DATA F,10-
19080 DATA END
19090 DATA -00A,10000A
19100 DATA -01A,10000B
19110 DATA -10A,10000B
19120 DATA -11A,10000B
19130 DATA 0--B,01000B
19140 DATA 10-B,01000E
19150 DATA 11-B,01000C
19160 DATA -0-C,00100D
19170 DATA -1-C,00100C
19180 DATA 0--D,00010D
19190 DATA 1-0D,00010A
19200 DATA 1-1D,00010E
19210 DATA --0E,00001F
19220 DATA --1E,00001E
19230 DATA 0--F,00000F
19240 DATA 1--F,00000A
19250 DATA END
20000 DATA 4,7: REM SAMPLE INPUT -- BCD DISPLAY DRIVER
20010 DATA 0000,1111110
20020 DATA 0001,0110000
20030 DATA 0010,1101101
20040 DATA 0011,1111001
20050 DATA 0100,0110011
20060 DATA 0101,1011011
20070 DATA 0110,0011111
20080 DATA 0111,1110000
20090 DATA 1000,1111111
20100 DATA 1001,1110011
20110 DATA 101-,XXXXXXX
20130 DATA 11--,XXXXXXX
20170 DATA END
