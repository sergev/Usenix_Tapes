LIB=/usr/games/lib/reversi
MAN=/usr/man/man6
DESTDIR=

reversi.doc: reversi.me
	tbl reversi.me | nroff -me | col > reversi.doc

install: reversi.doc reversi.remark
	cp reversi.doc reversi.remark $(DESTDIR)$(LIB)
	cp reversi.6 $(DESTDIR)$(MAN)
