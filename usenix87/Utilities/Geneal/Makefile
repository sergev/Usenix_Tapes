CFLAGS	=	-v -DUSG=1 -Dindex=strchr -Drindex=strrchr

LINTFLAGS	=	

DEST		= .

EXTHDRS		= /usr/include/ctype.h /usr/include/stdio.h

HDRS		= famtree.h geneal.h pagemap.h

LDFLAGS		= 

LINKER		= cc

MAKEFILE	= Makefile

OBJS		= dataman.o errorman.o famgetdat.o family.o famtree.o	\
		  gbrowse.o geneal.o index.o indivs.o pagemap.o strsav.o \
		  tprintf.o xalloc.o

LIBS		=	

PRINT		= pr

PROGRAM		= geneal

SRCS		= dataman.c errorman.c famgetdat.c family.c famtree.c	\
		  gbrowse.c geneal.c index.c indivs.c pagemap.c strsav.c \
		  tprintf.c xalloc.c

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

depend:;	@echo Updating makefile
		@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(SRCS)

lint:;		lint $(LINTFLAGS) $(SRCS) $(LINTLIBS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@install -s $(PROGRAM) $(DEST)

print:;		@$(PRINT) $(SRCS) $(HDRS)

program:	$(PROGRAM)

tags:	   $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DEST=$(DEST) install

###
dataman.o: /usr/include/stdio.h geneal.h
errorman.o: /usr/include/stdio.h
famgetdat.o: /usr/include/stdio.h geneal.h famtree.h
family.o: /usr/include/stdio.h /usr/include/ctype.h geneal.h pagemap.h
famtree.o: /usr/include/stdio.h geneal.h famtree.h pagemap.h
gbrowse.o: geneal.h
geneal.o: /usr/include/stdio.h /usr/include/ctype.h geneal.h
index.o: geneal.h
indivs.o: geneal.h
pagemap.o: pagemap.h /usr/include/stdio.h
xalloc.o: /usr/include/stdio.h
