# Written  1:32 am  Feb 17, 1986 by rde@ukc.ac.uk in net.sources
# In Real Life: R.D.Eager at U of Kent at Canterbury, Canterbury, UK
# Subject: uudecode in MS-BASIC

[food for line eater]

There  has been some discussion about public domain versions of uudecode
for those souls who didn't get it in a distribution. I  thought  it  was
time  to post the updated version of uudecode in Microsoft BASIC; I have
made a minor change to cater for a variation in  coding  encountered  on
some uuencoded files.

This  program  is  short  enough  that  it  can  be  typed in by hand if
necessary. I hope it is of some  use  to  micro  owners  without  direct
access to other uudecodes.

---------CUT HERE--------CUT HERE----------CUT HERE---------CUT HERE------
1000 DEFINT A-Z
1010 REM Trap filing errors
1020 ON ERROR GOTO 1600
1030 CLS
1040 LOCATE 5,11
1050 PRINT STRING$(40," ")
1060 LOCATE 5,11
1070 INPUT "Enter name of input file: ", INFILE$
1080 OPEN INFILE$ FOR INPUT AS #1
1090 LOCATE 8,10
1100 PRINT STRING$(40," ")
1110 LOCATE 8,10
1120 INPUT "Enter name of output file: ", OUTFILE$
1130 OPEN "R", #2,OUTFILE$, 1
1140 FIELD #2, 1 AS N$
1150 REM Search for header line
1160 LINE INPUT #1,A$
1170 IF LEFT$(A$,5) <>"begin" THEN 1160
1180 LOCATE 11,10
1190 PRINT "Header = ";A$
1200 SP = ASC(" ")
1210 DIM BUF(5000)
1220 P = 0
1230 REM Main loop
1240 LINE INPUT #1, A$
1250 P = 0
1260 COUNT = ASC(LEFT$(A$,1)) - SP
1270 IF COUNT <> 64 THEN 1290
1280 COUNT = 0
1290 IF COUNT = 0 THEN 1560
1300 ADJ = COUNT MOD 4
1310 FOR I = 2 TO LEN(A$) STEP 4
1320    X1 = ASC(MID$(A$,I,I)) - SP
1330    IF X1 <> 64 THEN 1350
1340    X1 = 0
1350    X2 = ASC(MID$(A$,I+1,I+1)) - SP
1360    IF X2 <> 64 THEN 1380
1370    X2 = 0
1380    X3 = ASC(MID$(A$,I+2,I+2)) - SP
1390    IF X3 <> 64 THEN 1410
1400    X3 = 0
1410    X4 = ASC(MID$(A$,I+3,I+3)) - SP
1420    IF X4 <> 64 THEN 1440
1430    X4 = 0
1440    P = P + 1
1450    BUF(P) = (X2\16) + (X1*4)
1460    P = P + 1
1470    BUF(P) = (X3\4) + ((X2 MOD 16) * 16)
1480    P = P + 1
1490    BUF(P) = X4 + ((X3 MOD 4) * 64)
1500 NEXT I
1510 FOR I = 1 TO P
1520   LSET N$ = CHR$(BUF(I))
1530   PUT #2
1540 NEXT I
1550 GOTO 1240
1560 END
1570 REM
1580 REM Error trapping routine for file handling
1590 REM
1600 IF ERL <> 1080 GOTO 1650          ' not input file problem
1610 LOCATE 22,20
1620 PRINT "Can't open input file"
1630 GOSUB 1780
1640 RESUME 1040
1650 IF ERL <> 1130 GOTO 1700          ' not output file problem
1660 LOCATE 22,20
1670 PRINT "Can't open output file"
1680 GOSUB 1780
1690 RESUME 1090
1700 IF ERL <> 1160 THEN 1770
1710 LOCATE 22,20
1720 PRINT "Header line not found"
1730 GOSUB 1780
1740 GOSUB 1780
1750 LOCATE 24,1
1760 END
1770 ERROR ERR
1780 FOR I = 1 TO 5000: NEXT I
1790 LOCATE 22,20
1800 PRINT STRING$(30," ")
1810 RETURN
TE 24,1
1760 END
1770 ERROR ERR
1780 FOR I = 1 TO 5000: NEXT I
1
---------CUT HERE--------CUT HERE----------CUT HERE---------CUT HERE------
-- 
           Bob Eager

           rde@ukc.UUCP
           rde@ukc
           ...!mcvax!ukc!rde

           Phone: +44 227 66822 ext 7589
# End of text from net.sources on hplabsc.UUCP
