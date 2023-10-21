#
# Makefile for atc parent
#

# for Berkeley, use the following three lines
# OSVER = BSD
# INST =	bsd_inst
# USETERML =	-ltermcap

# for System V, use the following three lines
OSVER =		SYSV
INST =		sysv_inst
TERMLIB =

# for System V with kernel locking, use the following line
UNISTD = -DUNISTD

# Tailor the following two lines to fit your system
# DESTINATION must be terminated with a slash!
OWNER =		games
DESTINATION =	/usr/local/games/lib/atc/

GAME =		atc
SRC =		src
GAMES =		games

TUNABLE	=	tunable.c
NF      =	$(GAME) -p
MAKEFILE =	Makefile.dist

all:		makesrc

install:	makesrc comm_inst $(INST)

comm_inst:	
		-mkdir `$(SRC)/$(NF)`
		-cp $(GAMES)/* `$(SRC)/$(NF)`
		-chmod 644 `$(SRC)/$(NF)`/*
		-chmod 755 `$(SRC)/$(NF)`

bsd_inst:	
		-chown -f $(OWNER) `$(SRC)/$(NF)`/* `$(SRC)/$(NF)`
		(cd $(SRC); install -o $(OWNER) -m 4755 $(GAME) $(DESTINATION))

sysv_inst:	
		-chown $(OWNER) `$(SRC)/$(NF)`/* `$(SRC)/$(NF)`
		(cd $(SRC); cpset $(GAME) $(DESTINATION) 4755 djmolny ae)

makesrc:	
		(cd $(SRC); \
		make OSVER=$(OSVER) UNISTD=$(UNISTD) \
		DEST='"$(DESTINATION)"' $(CURSESLIBR) USETERML=$(USETERML) )

archive:
		shar Makefile README games/* src/*.[chly] doc/* > ARCH
