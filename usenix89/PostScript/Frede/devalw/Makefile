FONTS=S R I B BI H HO HB HX C CO CB CX S2
F=/usr/lib/font/devalw
MAKEDEV=../makedev
INSTALL=:

$F/DESC.out: DESC $(FONTS)
	$(MAKEDEV) DESC
	$(INSTALL) DESC.out bin 644 $F/DESC.out
	$(INSTALL) R.out bin 644 $F/R.out
	$(INSTALL) I.out bin 644 $F/I.out
	$(INSTALL) B.out bin 644 $F/B.out
	$(INSTALL) S.out bin 644 $F/S.out
	$(INSTALL) S2.out bin 644 $F/S2.out
	$(INSTALL) H.out bin 644 $F/H.out
	$(INSTALL) HO.out bin 644 $F/HO.out
	$(INSTALL) HB.out bin 644 $F/HB.out
	$(INSTALL) HX.out bin 644 $F/HX.out
	$(INSTALL) C.out bin 644 $F/C.out $F/CW.out
	$(INSTALL) CO.out bin 644 $F/CO.out
	$(INSTALL) CB.out bin 644 $F/CB.out
	$(INSTALL) CX.out bin 644 $F/CX.out
	rm -s *.out
