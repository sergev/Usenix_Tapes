#
# makefile for complete source archive manipulation program suite
#

#
# modify archive to point to the location where you will be
# keeping your archived sources
#
# set shell to bourne shell or variants (korn shell)
shell	=	/bin/sh
myname	=	barto
myhome	=	/mnt/${myname}
archdir	=	/mnt/Archive
toc_from_mod_sources= MODLIST
toc_from_source_archive= ALL-TOC
#
# below here generally does not change
#
currentmonth=	Current
processdir=	${myhome}/archive
archcur	=	${archdir}/${currentmonth}
savelog	=	Save.Log
newsdir	=	/usr/lib/news
progdir	=	${newsdir}/archive
chkprog	=	${progdir}/lbgmCK
delprog	=	${progdir}/lbgmX
savesource =	${progdir}/savesource

FILES	=\
	RNmacros skip-words quicklist

SRCS	=\
	Add-bugs.x Bld_quick.x Volume.x all-mod-req.x all-quick-req.x\
	ckarch.x clean.x do-mail.x eom.x\
	fix-path.x get.x get-mod-req.x get-std-req.x lbgmCK.x lbgmX.x\
	modlist.x mv-current-to.x process.x quick-mod-req.x restore-log.x\
	rm-cancel.x rmarch.x savesource.x shrink-log.x squish.x\
	toc.x toc-all.x valid-log.x

OBJS	=\
	Add-bugs Bld_quick Volume all-mod-req all-quick-req\
	ckarch clean do-mail eom\
	fix-path get get-mod-req get-std-req lbgmCK lbgmX\
	modlist mv-current-to process quick-mod-req restore-log\
	rm-cancel rmarch savesource shrink-log squish\
	toc toc-all valid-log

#
# DO NOT CHANGE THE ORDER OF THE REPLACEMENTS.
#
all: $(SRCS)
	@for i in $(SRCS); do\
	    echo "$$i -> `basename $$i .x`";\
	    sed -e 's%ARCHCUR%${archcur}%g' -e 's%SHELL%${shell}%g'\
		-e 's%ARCHDIR%${archdir}%g' -e 's%PROCESS%${processdir}%g'\
		-e 's%SAVELOG%${savelog}%g' -e 's%SAVESOURCE%${savesource}%g'\
		-e 's%TOCFROMMODSOURCES%${toc_from_mod_sources}%g'\
		-e 's%TOCFROMSOURCEARCHIVE%${toc_from_source_archive}%g'\
		-e 's%MYHOME%${myhome}%g' -e 's%MYNAME%${myname}%g'\
		-e 's%DELPROG%${delprog}%g' -e 's%CHKPROG%${chkprog}%g'\
		    < $$i > `basename $$i .x`;\
	done

SUID	=\
	Volume get toc

ARCHIVE	=\
	ckarch clean do-mail get lbgmCK lbgmX modlist\
	mv-current-to restore-log rm-cancel rmarch savesource\
	shrink-log squish toc-all valid-log


USER	=\
	Bld_quick all-mod-req quick-mod-req quick-mod-req get-mod-req\
	get-std-req quicklist skip-words

OWNER	=	-o news
GROUP	=	-g news
STD	=	${OWNER} ${GROUP} -m 755

install: all
	@echo "mkdir ${archdir}"
	@echo "mkdir ${progdir}"
	@echo "mkdir ${processdir}"
	@echo "install ${OWNER} ${GROUP} -m 6755 get ${archdir}/get"
	@echo "install ${OWNER} ${GROUP} -m 6755 Volume ${progdir}/Volume"
	@echo "install ${OWNER} ${GROUP} -m 6755 toc ${progdir}/toc"
	@for i in ${ARCHIVE}; do\
	    echo "install ${STD} $$i ${progdir}/$$i";\
	done
	@for i in ${USER}; do\
	    echo "install ${STD} $$i ${processdir}/$$i";\
	done

noobjs:
	/bin/rm -f $(OBJS)
