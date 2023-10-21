DEST	      = .

LINKER	      = cc

CFLGS		  = -O -s -n

PROGRAM	      = shred

SRCS	      = lex.yy.c

$(PROGRAM):     $(SRCS) 
		$(LINKER) $(CFLGS) -o $(PROGRAM) $(SRCS)
		@echo "done"

lex.yy.c:		 shred.l
		lex shred.l

install:	$(PROGRAM)
		cp $(PROGRAM) $(DEST)
		chmod 555 $(DEST)
