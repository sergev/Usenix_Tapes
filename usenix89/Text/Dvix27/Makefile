# The following file and directory specifications may need changing at
# your site:
#
# where are the bitmaps stored?
FONTAREA=/usr/lib/tex/fonts
#
# where is the software to be installed
BINAREA=/usr/local
#
# an Apollo-specific spool file
SPOOLFILE=/usr/spool/laserwriter/apollo
#

OPTIM= -O
CFLAGS = -DFONTAREA=\"${FONTAREA}\" \
         -DSPOOLFILE=\"${SPOOLFILE}\" '-DDVI2PS="/usr/local/dvix27 -q"' \
	 '-DPOSTF="/usr/local/lib/laserf -c"' \
	 '-DTEXTF="/usr/local/lib/laserf"' \
	$(OPTIM)

all: dvix27

dvix27: dvix27.o findfile.o
	cc ${CFLAGS} -o dvix27 dvix27.o findfile.o

install: all
	install -s -m 755 dvix27 ${BINAREA}/dvix27

#
# The following may be useful if you are trying to run this as a
# printcap filter
#

dvix27f:	dvix27f.c
	${CC} -o dvix27f ${CFLAGS} dvix27f.c
