#
# dennis@rlgvax 2/4/86
#
# fixtcp.mk	Makefile, this file
# fixtcp	shell script
# _get_tcp_.c	C program
# _get_tcp_	a.out program called by fixtcp
# .fixtcp.mail	header for mail
#
# directions, type
#	make -f fixtcp.mk	# to make necessary files
#	edit fixtcp.mk and change INSTALLDIR
#	make -f fixtcp.mk install
#	cd $INSTALLDIR		# directory where you really installed it
#	fixtcp			# to display tcp connections hung in finwait2
#
#				# don't do this if you have none to unstick
#	su root			# required for adb write mode
#	fixtcp fix		# to actually unstuck tcp connections

# change this at your site
INSTALLDIR = .

all: _get_tcp_

clean:
	rm -f _get_tcp_

install: _get_tcp_
	-cp _get_tcp_ $(INSTALLDIR)
	-cp fixtcp $(INSTALLDIR)

# distribute the latest version to the world, private for dennis@rlgvax
dist:
	rm -rf /tmp/dpb
	mkdir /tmp/dpb
	cp fixtcp.mk /tmp/dpb
	cp _get_tcp_.c /tmp/dpb
	cp ../cmd/fixtcp /tmp/dpb
	cp .fixtcp.mail	/tmp/dpb
	(cd /tmp/dpb; makeshar * >>.fixtcp.mail)

# please note that .fixtcp.mail was chosen so that makeshar *
# doesn't try to append to itself.
