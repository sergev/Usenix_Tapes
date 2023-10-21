CFLAGS = -O
LINT = lint -ph
DESTDIR = /usr/local
MANLOC = /usr/man/manl/xref.l

all : xref xreflex xref.1 ; @echo 1>&2 "[Made xref xreflex xref.1]"

xref : xrefhead.sh mergelines.awk ; sh xref.sh $(DESTDIR) >/dev/null

xreflex : lex.yy.c ; cc -o xreflex $(CFLAGS) lex.yy.c -ll
lex.yy.c : xreflex.l ; lex -v xreflex.l

xref.1 : xref.man ; sh xref.man.sh $(DESTDIR) >/dev/null


piecetest :
	xreflex xreflex.l xref.man >xreflex_out
	sort -u +0f -1 +0 -1 +1 -2 +2n <xreflex_out >sort_out
	awk -f mergelines.awk <sort_out >awk_out

lint : lex.yy.c ; $(LINT) lex.yy.c > xreflex.lint

install : all
	install -c xref $(DESTDIR)
	install -c -s xreflex $(DESTDIR)
	install -c -m 644 xref.1 $(MANLOC)

clean : ; -rm *_out *.lint *.yy.c xref xreflex xref.1
