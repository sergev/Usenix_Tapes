RFS = remotefs fig1.mag fig2.mag fig3.mag fig4.mag fig5.mag fig6.mag \
	appendixB.out

rfs: $(RFS)
	troff $(ONLY) -Tmag -mstek remotefs > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
lgp: $(RFS)
	ltroff -mstek $(ONLY) remotefs

#
# run off figure 1 or 2 or 3
#
f1: fig1.mag
	troff -Tmag -mstek fig1.mag > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
f2: fig2.mag
	troff -Tmag -mstek fig2.mag > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
f3: fig3.mag
	troff -Tmag -mstek fig3.mag > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
f4: fig4.mag
	troff -Tmag -mstek fig4.mag > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
f5: fig5.mag
	troff -Tmag -mstek fig5.mag > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
f5: fig5.mag
	troff -Tmag -mstek fig5.mag > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
f6: fig6.mag
	troff -Tmag -mstek fig6.mag > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
A: appendixA.out
	troff -Tmag -mstek appendixA.out > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image
B: appendixB.out
	troff -Tmag -mstek appendixB.out > image
	@sh -c "echo -n Waiting... ; read x"
	dmag -w$(WIND) < image

#
# generate figure 1
#
fig1.mag: fig1
	/lib/cpp -Dfigure1 fig1 | sed -e '/^#/d' | pic -T100 > fig1.mag
fig1.lgp: fig1
	/lib/cpp -Dfigure1 fig1 | sed -e '/^#/d' | pic > fig1.lgp

#
# generate figure 2
#
fig2.mag: fig1
	/lib/cpp -Dfigure2 fig1 | sed -e '/^#/d' | pic -T100 > fig2.mag
fig2.lgp: fig1
	/lib/cpp -Dfigure2 fig1 | sed -e '/^#/d' | pic > fig2.lgp

#
# generate figure 3
#
fig3.mag: fig1
	/lib/cpp -Dfigure3 fig1 | sed -e '/^#/d' | pic -T100 > fig3.mag
fig3.lgp: fig1
	/lib/cpp -Dfigure3 fig1 | sed -e '/^#/d' | pic > fig3.lgp

#
# generate figure 4
#
fig4.mag: fig4
	pic -T100 fig4 > fig4.mag
fig4.lgp: fig4
	pic fig4 > fig4.lgp

#
# generate figure 5
#
fig5.mag: fig5
	pic -T100 fig5 > fig5.mag
fig5.lgp: fig5
	pic fig5 > fig5.lgp

#
# generate figure 6
#
fig6.mag: fig6
	pic -T100 fig6 > fig6.mag
fig6.lgp: fig6
	pic fig6 > fig6.lgp

#
# generate appendix A
#
appendixA.out: appendixA
	tbl appendixA > appendixA.out

#
# generate appendix B
#
appendixB.out: appendixB
	tbl appendixB > appendixB.out
