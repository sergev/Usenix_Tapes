SCOREFILENAME = /tmp/sdi_scores
SCOREFILE = "${SCOREFILENAME}"

# The breakdown into SRC 1, 2 and 3 is purely for shar purposes.
SRC1 = main.c helpers.c missile.c cursor.c scores.c save_game.c text.c piemenu_track.c

SRC2 = control.c blast.c circles.c laser.c rocks.c input.c notify.c control_procs.c  menu.c

SRC3 = intersect.c random.c incoming.c cities.c pr_helpers.c game_over.c argv.c global.c icons.c 

SRC = ${SRC1} ${SRC2} ${SRC3}

H1 = melt.h default_city.h cursor.h laser.h mushroom.h laserkill.h \
	missilekill.h city_icon1.h \
	city_icon2.h city_icon3.h city_icon4.h city_icon5.h city_icon6.h \
	city_icon7.h city_icon8.h rocks.h dynacursor.h foe_picture.h \
	laser_picture.h interceptor_picture.h rock_picture.h dyna_picture.h \
	silly_picture.h cross_picture.h foe_ground_picture.h \
	fancy_icon1.h fancy_icon2.h fancy_icon3.h 

H2 = fancy_icon4.h fancy_icon5.h \
	fancy_icon6.h fancy_icon7.h fancy_icon8.h fancy_icon9.h fancy_icon10.h \
	fancy_icon11.h fancy_icon12.h fancy_icon13.h fancy_icon14.h \
	fancy_icon15.h fancy_icon16.h fancy_icon17.h fancy_icon18.h \
	fancy_icon19.h fancy_icon20.h fancy_icon21.h fancy_icon22.h \
	fancy_icon23.h fancy_icon24.h walkmenu_impl.h image_impl.h center.h \
	incoming_picture.h pie_generic_cursor.h

H = ${H1} ${H2}

ADMIN = README sdi.man HISTORY.nr NOTES TODO makefile genmessage.c \
	makehistory.awk editall source_converter.c longfile.nr sdi.h \
	lookup.h gameover.h icon.h talkingmark.h makeman.sed novice_advice.h \
	occasional_advice.h expert_advice.h about_msg.h instructions.h \
	piemenu.h makeversion version.c

ALLFILES = ${ADMIN} ${SRC} ${H}

OBJ = main.o circles.o blast.o missile.o intersect.o random.o incoming.o \
	cities.o pr_helpers.o control.o game_over.o helpers.o argv.o global.o \
	laser.o icons.o scores.o save_game.o rocks.o notify.o input.o \
	control_procs.o cursor.o text.o menu.o piemenu_track.o \
	sourcecode.o history.o man.o version.o

#If you are running SUNOS 3.0, use this: CLAGS = -DSUNOS3_0

CFLAGS = -DSUNOS3_0

LIBS = -lsuntool -lsunwindow -lpixrect -lm

all: sdi

sdi: ${OBJ}
	cc ${CFLAGS} -o sdi ${OBJ} ${LIBS}

version.o: ${SRC} ${H} ${ADMIN}
	makeversion version.c
	cc ${CFLAGS} -c version.c

fastsdi:
	/bin/rm -f sdi
	touch sourcecode.o
	make sdi
	sdi

src:
	cat ${SRC} | expand -4 >src

sourcecode.o: ${SRC} source_converter
	cat ${SRC} | expand -4 | source_converter source_code > /tmp/sourcecode.s
	cc -c /tmp/sourcecode.s
	/bin/rm -rf /tmp/sourcecode.s

history.o: HISTORY.nr source_converter
	cat longfile.nr HISTORY.nr | tbl | nroff -me | col | awk -f makehistory.awk | source_converter history_text >/tmp/history.s
	cc -c /tmp/history.s
	/bin/rm -rf /tmp/history.s

man.o: sdi.man source_converter makeman.sed
	nroff -man longfile.nr sdi.man | sed -f makeman.sed | col | source_converter man_text >/tmp/man.s
	cc -c /tmp/man.s
	/bin/rm -rf /tmp/man.s

grind:
	cat ${SRC} | expand -4 | vgrind -t -h SDI >source.grind

textedit:
	editall ${SRC} ${ADMIN}
size:
	wc ${SRC}
	wc ${ADMIN}
	wc ${H}

clean:
	/bin/rm -f sdi bigmessage.h genmessage *.o *shar

shar:
	/bin/rm -f sdi.shar
	makescript -o sdi.shar ${ALLFILES}

Z: shar
	/bin/rm -f sdi.shar.Z
	compress sdi.shar

splitshar:
	makescript -o sdi1.shar ${ADMIN}
	makescript -o sdi2.shar ${SRC1}
	makescript -o sdi3.shar ${SRC2}
	makescript -o sdi4.shar ${SRC3}
	makescript -o sdi5.shar ${H1}
	makescript -o sdi6.shar ${H2}
	ls -l sdi1.shar sdi2.shar sdi3.shar sdi4.shar sdi5.shar sdi6.shar

#
# ORIGSIZE for game and the constant for bigmessage.h
# are related by a factor of 64.
#
game_over.o: bigmessage.h game_over.c
	cc ${CFLAGS} -c game_over.c -DORIGSIZE=256

scores.o: scores.c
	cc ${CFLAGS} -c scores.c '-DSCOREFILE=${SCOREFILE}' 

bigmessage.h: genmessage talkingmark.h
	genmessage talkingmark.h 4 >bigmessage.h

genmessage: genmessage.c
	cc ${CFLAGS} -o genmessage genmessage.c ${LIBS}

testrandom: random.o testrandom.o
	cc ${CFLAGS} -o testrandom testrandom.o random.o

vectortest: vectortest.o
	cc ${CFLAGS} -o vectortest vectortest.o ${LIBS}

wormtest: wormtest.o
	cc ${CFLAGS} -o wormtest wormtest.o ${LIBS}

icontest: icontest.o
	cc ${CFLAGS} -o icontest icontest.o ${LIBS}

keytest: keytest.o
	cc ${CFLAGS} -o keytest keytest.o ${LIBS}

invert: invert.o
	cc ${CFLAGS} -o invert invert.o ${LIBS}

icons.o: city_icon1.h city_icon2.h city_icon3.h city_icon4.h city_icon5.h city_icon6.h city_icon7.h city_icon8.h
