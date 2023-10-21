#! /bin/csh -f
#
# psfilter - filter for the Apple LaserWriter (Postscript) printer
#
#  DON'T edit this file directly, edit the source in 
#  /usr/src/local/prop/transcript/src/psfilter.csh, run make psfilter
#  and install the result.  I had to retrofit changes once, please consider
#  the poor slob who has to change it next time.  (R.H. 5/85)
#
# Each of these files is merely a link to the current file, and we look at
# argument 0 to see which one we are being called by.  Alternatively, you
# may wish to modify printjob.c (/usr/src/usr.lib/lpr/printjob.c) to pass
# the name of the filter you wish to use as the argument following the -f
# switch.
#


set name host filter x y indent width length acct pid printer title
setenv TERMCAP /etc/printcap
umask 0

if ($?0) set filter = $0

top:
	if($#argv > 0) then
		switch ($argv[1])
		case -f:	# filter desired
			shift argv
			if ($#argv > 0) then
				set filter = $argv[1]
				shift argv
			endif
			goto top

		case -h:	# host spooling file
			shift argv
			if ($#argv > 0) then
				set host = $argv[1]
				shift argv
			endif
			goto top

		case -i*:	# indentation, ignored for now
			set i=$argv[1]
			shift argv
			goto top

		case -l*:	# page length, ignored for now
			set l=$argv[1]
			shift argv
			goto top

		case -n:	# uid of spooling user
			shift argv
			if ($#argv > 0) then
				set name = $argv[1]
				shift argv
			endif
			goto top

		case -p:	# printer to be used
			shift argv
			if ($#argv > 0) then
				set printer = (-i $argv[1])
				set pset
				shift argv
			endif
			goto top

		case -w*:	# page width, ignored for now
			set width=$argv[1]
			shift argv
			goto top

		case -x*:	# starting x position, ignored for now
			set x=$argv[1]
			shift argv
			goto top

		case -y*:	# starting y position, ignored for now
			set y=$argv[1]
			shift argv
			goto top

		default:	# currently assumed to be the accounting file
				# This name is defined as af in printcap
				# tail of name must match printer name
			set acct=$argv[1]
			set pid=(-P$argv[1]:t)
			set logfile=/usr/adm/log-$argv[1]:t
			shift argv
			goto top
		endsw
	endif


set lockfile=lock				# doubtful anything else
set cffile=`cat $lockfile | grep cf | head -1`	# grok the name of the cf
set title=`grep N $cffile | head -1 | sed s/N//`	# get job title
set mtmp="/tmp/spoolmail$$"			# name of temp mail file
set policy=.policy				# text file to be mailed
if ( ! -f $policy ) set policy=/dev/null	# subject will have to do
set taunt="Permission to use laser printer denied"

# Contrary to it's name, psif is the filter which talks to the printer. It's
# intended to be specified as the 'if' filter in the printcap.  This works
# fine if everything is processed to PostScript before being spooled, but in
# this setup it is used as the tail of the filter chain.  Psof should be
# specified as the 'of' entry in the printcap and the sh (short header) boolean
# should be set.  It formats a banner maker and psif sends it after EOF on
# the file it is processing.

# Modified June 86 to support LW's on Bridges also.  The printer name in
# the form -Pname (as determined above) is passed to psif.  Psif does a
# gethostbyname call, if the printer name is defined (in /etc/hosts or to
# the nameserver as applicable) then a network socket is opened to that
# address.  The address for the name is actually assigned to a Bridge serial
# port to which the LW is connected.  Psif then treats the socket as its
# stdout and does its normal processing, the output stream will flow across
# the net to the remote LW.  If the gethostbyname call fails then nothing
# special is done and psif behaves as before, assuming its stdout is
# connected to the physical printer device (as setup by lpd).
#  Ron Hitchens

onintr intrupt					# vector int's to error section

# Get the filetype portion of the filter name
switch ($filter:r)

	case /usr/local/lib/psasc:			# ascii document
	case psasc:
		set filtype="ascii"
		if ( "$title" == "" ) set title="ascii document"
# -------------------------------------
# Permission checking/accounting hacks should be hooked in here
# similar entries for each case below
#		/usr/local/lib/laser_ok $host $name # returns 0 if valid user
#		if ( $status ) then
#			Mail -s "$taunt" $name@$host < $policy
#			exit 2		# spool file is discarded
#		endif
# -------------------------------------

		/usr/local/lib/psif -h $host -n $name $pid $acct
		if ( $status ) breaksw			# go handle error
		exit 0					# all is well

	case /usr/local/lib/psdvi:			# TeX document
	case psdvi:
		set filtype="TeX"
		if ( "$title" == "" ) set title="TeX document"
		/usr/local/lib/dvi2ps -i -q  \
			-h /usr/local/lib/allfonts/postscript/tex.ps \
			| /usr/local/lib/psif -h $host -n $name $pid $acct
		if ( $status ) breaksw		# drop down and handle error
		exit 0				# success

	case /usr/local/lib/psditroff:			# ditroff document
	case psditroff:
		set filtype="Ditroff"
		if ( "$title" == "" ) set title="Ditroff document"
		/usr/local/lib/psdit | /usr/local/lib/psrev | \
			/usr/local/lib/psif -h $host -n $name $pid $acct
		if ( $status ) breaksw			# go handle error
		exit 0

	case /usr/local/lib/psmac:		# PS file made by a mac
	case psmac:				# uses rf (fortran) type
		set filtype="Macintosh"
		if ( "$title" == "" ) set title="Mac laser document"
		/usr/local/lib/macfilter | \
			/usr/local/lib/psif -h $host -n $name $pid $acct
		if ( $status ) breaksw		# go handle error
		exit 0

	case /usr/local/lib/pscat:			# old troff, no no
	case pscat:
		set filtype="old style troff"
		if ( "$title" == "" ) set title="old troff document"
		echo "Old style troff is not supported on the LaserWriter" \
			> $mtmp
		echo "Please use ditroff instead." >> $mtmp
		set nolog
		breaksw
		exit 0		# superstition

	default:
		set filtype="LaserWriter"
		if ( "$title" == "" ) set title="document"
		echo "No support for that file type on the LaserWriter" > $mtmp
		set nolog
		breaksw
		exit 0		# superstition

endsw

intrupt:			# a place to catch interrupts, hopefully these
				# shell vars have been set, will be if lpd 
				# kills a running filter, like psif

if ( ! $?nolog ) then
  echo "Error returned by $filtype filter.  Here are the last few lines from" \
	> $mtmp
  echo "the error log, yours should be last:" >> $mtmp
  echo "" >> $mtmp
  tail -8 $logfile >> $mtmp
endif

Mail -s "Error spooling $title to $pid" $name@$host < $mtmp
rm $mtmp

echo "There was a problem with your $filtype job spooled for $pid" > $mtmp
echo "A mail message was sent to you at $host with further details" >> $mtmp
( /usr/local/telegram $name@$host < $mtmp > /dev/null ; rm $mtmp ) &

exit 2				# throw the file away
