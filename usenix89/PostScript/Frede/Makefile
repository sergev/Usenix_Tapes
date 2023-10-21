
# SYS should be one of AUSAM, BSD, SYSV, V7
# This is only really important if VERBOSE (below) is 1.
SYS=SYSV
# VERBOSE should be 1 to generate postscript code to echo stuff
# back down the communication line. Otherwise 0.
VERBOSE=0
MAKEDEV=../devalw	# pathname relative to devalw
CD=builtin cd

all:
	$(CD) src; $(MAKE) $(MFLAGS) SYS=$(SYS) VERBOSE=$(VERBOSE)
	$(CD) devalw; $(MAKE) MAKEDEV=$(MAKEDEV)
