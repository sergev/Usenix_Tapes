DEST	      = /usr/local

MDEST	      = /usr/man/manl/rs.l

LDFLAGS	      = -DBSD -O -w

LIBS          = -ltermcap

LINKER	      = cc

MANUAL	      = rs.1

PROGRAM	      = rs

SRCS	      = rs.c

all:		$(PROGRAM)

$(PROGRAM):     
		@echo -n "Loading rs ..."
		@$(LINKER) $(LDFLAGS) $(SRCS) $(LIBS) -o $(PROGRAM)
		@echo "done."

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@install -s $(PROGRAM) $(DEST)
		@echo Installing manual in $(MDEST)
		@cp $(MANUAL) $(MDEST)

program:        $(PROGRAM)
