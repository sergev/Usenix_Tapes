#
# Makefile for atc parent
#

OWNER =		edjames
DESTINATION =	/usr/public

GAME =		atc
SRC =		src
DIST =		dist
GAMES =		games

CURSESLIBR =	libatc_curses.o
TUNABLE	=	tunable.c
NF      =	$(GAME) -p
MAKEFILE =	Makefile.dist

all:		makesrc install

install:	distrib $(DIST)
		(cd $(DIST); make)
		-mkdir `$(DIST)/$(NF)`
		-cp $(GAMES)/* `$(DIST)/$(NF)`
		-chmod 644 `$(DIST)/$(NF)`/*
		-chmod 755 `$(DIST)/$(NF)`
		-chown -f $(OWNER) `$(DIST)/$(NF)`/* `$(DIST)/$(NF)`
		(cd $(DIST); install -o $(OWNER) -m 4755 $(GAME) $(DESTINATION))

distrib:	$(DIST)/Makefile $(DIST)/$(TUNABLE) \
		$(DIST)/$(CURSESLIBR)

makesrc:	
		( cd $(SRC); make $(CURSESLIBR) )

$(DIST):
		mkdir $(DIST)

$(DIST)/$(TUNABLE):	$(SRC)/$(TUNABLE)
		rm -f $(DIST)/$(TUNABLE)
		cp $(SRC)/$(TUNABLE) $(DIST)/$(TUNABLE)
		chmod 644 $(DIST)/$(TUNABLE)
		
$(DIST)/Makefile:	$(SRC)/$(MAKEFILE)
		rm -f $(DIST)/Makefile
		cp $(SRC)/$(MAKEFILE) $(DIST)/Makefile
		
$(DIST)/$(CURSESLIBR):	$(SRC)/$(CURSESLIBR)
		rm -f $(DIST)/$(CURSESLIBR)
		cp $(SRC)/$(CURSESLIBR) $(DIST)/$(CURSESLIBR)
		
archive:
		shar Makefile README games/* src/*.[chly] doc/* > ARCH
