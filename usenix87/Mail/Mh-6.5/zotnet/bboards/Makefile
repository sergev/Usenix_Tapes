##############################################################################
#    Instructions to Make, for compilation of ZOT UIP/utility processes
#
#	This file is automatically generated.  Do not edit!
##############################################################################

# The following entries were inserted during MH configuration
OPTIONS	=	-DSYS5 -DMHRC -DMORE='"/local/bin/more"' -DSENDMTS -I..

CC      =	cc
CFLAGS  =	-O    $(OPTIONS)
LINT	=	lint
LFLAGS	=	-bhu $(OPTIONS)
LLIBS	=


######################################################################
#	Here it is...
######################################################################

all:		getbbent

lint:		l-getbbent

install:	inst-all clean

inst-all:;

tar:;

uninstall:;


######################################################################
# getbbent
######################################################################

getbbent:	getbbent.o

l-getbbent:;	$(LINT) $(LFLAGS) getbbent.c $(LLIBS)


##############################################################################
#	Miscellaneous tasks
##############################################################################

distribution:   clean

clean:		unclean
		-rm -f *.o

unclean:;	-rm -f _* :* core eddep makedep


##############################################################################
#	Dependencies
##############################################################################

MODULES	=	getbbent
depend:;	for m in $(MODULES); do ( \
		    i=`basename $$m .c`; \
		    echo $$i.o: $$i.c >> makedep; \
		    grep '^#[ 	]*include' $$i.c | \
			sed -e 's,[^"]*"/\([^"]*\)".*,'$$i'.o: /\1,' \
			    -e 's,[^"]*"\([^"]*\)".*,'$$i'.o: \1,' \
			    -e 's,[^<]*<\(.*\)>.*,#'$$i'.o: /usr/include/\1,' \
			>> makedep \
		); done
		echo '/^# DO NOT DELETE THIS LINE/+2,$$d' > eddep
		echo '$$r makedep' >> eddep
		echo 'w' >> eddep
		cp Makefile _Makefile
		ed - Makefile < eddep
		rm eddep makedep
		echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
		echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile

# DO NOT DELETE THIS LINE
# DEPENDENCIES START HERE
getbbent.o: getbbent.c
getbbent.o: bboards.h
getbbent.o: ../../h/strings.h
#getbbent.o: /usr/include/ctype.h
#getbbent.o: /usr/include/pwd.h
#getbbent.o: /usr/include/grp.h
#getbbent.o: /usr/include/stdio.h
#getbbent.o: /usr/include/sys/types.h
#getbbent.o: /usr/include/sys/stat.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
