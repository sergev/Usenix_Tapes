TROFF = psroff

noargs:
	@echo "specify what to make"

install:
	makelinks commands
	makelinks primitives
	makelinks library

print: front body

front:
	${TROFF} cover/title
	${TROFF} cover/preface
	${TROFF} cover/notes
	${TROFF} cover/diffs

body:
	cd commands; ${TROFF} -man *.1
	cd primitives; ${TROFF} -man *.2i
	cd library; ${TROFF} -man *.3i
