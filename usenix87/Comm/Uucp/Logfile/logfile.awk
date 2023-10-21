# logfile.awk -- read a uucp LOGFILE and find out how long
# we spent talking to particular places.  (Also, remembers if
# the time spent was our call or their call).
#
# This is nice for: 1) Knowing when you made long distance
# calls and where to, 2) knowing how much of the load between
# you and some sites you're carrying.
#
#
# This works with the UUCP log file format produced by the
# uucp delivered with BRL Release 3.  (i.e. 4.2BSD, i.e. that
# *extremely* hacked up conglomeration of uucp's that prompted
# the writing of honey-danber). 
#
#
# USAGE
#	awk -f logfile.awk /usr/spool/uucp/LOGFILE
#
# Actually -- I would suggest saving LOGFILE somewhere and make
# sure uucico is no longer writing to it.  This way you're sure
# that the data generated is valid.  What I do here is:
#
#	set `date`
#	tag=$2.$7
#	cd /usr/spool/uucp
#	mv LOGFILE OLD/LOGFILE.${tag}
#	compress OLD/LOGFILE.${tag}
#	uncompress OLD/LOGFILE.${tag}
#	awk -f /usr/lib/uucp/logfile.awk OLD/LOGFILE.${tag}
#
# Somehow, compress waits until nobody is using the file before it
# compresses it.  This is nice and convenient.
#
#
# AUTHOR
#	David Herron (NPR lover)
#	cbosgd!ukma!david
#	University of Kentucky, Computer Science
#
# Changes:
#	1. Took out ignore capability(if you want it put it back in)
#	2. Made compatible with System V release I
#
# EDITOR
#	Michael Wexler
#	trwrb!felix!peregrine!mike
#	Peregrine Systems, Inc
#
BEGIN	{
	# states
	idle = 0; calling = 1; uscall = 2; themcall = 3;
	true = 1; false = 0
	}

# We're calling some place, and the call part has actually worked.
# 1) Record their name in the master list.
# 2) Remember that we're placing the call.

$1 ~ /.*!.*/	{
		n = split($1,a,"!");
		user=a[2];
		sys=substr(a[1],1,6);
		time=$2
		status=$4
		event=$5
	}
$1 !~ /.*!.*/	{
	user=$1
	sys=substr($2,1,6)
	time=$3
	status=$4
	event=$5
}
status == "SUCCEEDED" && event == "(call" {
	state[sys] = calling
}

# A call succeeded.  Either they called us or we called them.
# state[sys] tells us who is doing the calling.
# Have to remember the time.

status == "OK" && event == "(startup)" {
	startime[sys] = time
	if (state[sys] == calling) {
		printf("call\tout\t%s\t%s\n", sys, time)
		state[sys] = uscall
	}
	else {
		printf("call\tin\t%s\t%s\n", sys, time)
		state[sys] = themcall
	}
}


# Our outgoing call failed.  Throw away our information about the call.

status == "TIMEOUT" {
	state[sys] = idle
	}

# A call finished either successfully or unsuccessfully.
# Have to add in the time to the appropriate sum.
#
# It would be "hard" to calculate the time correctly.  So, I'm using
# a heuristic here to make it easy.  I assume that no phone call is
# going to last for longer than 1 day.  I calculate the time
# for the ending and beginning of the call, and if it's negative
# I add 24 hours to it.
#
# I know ... groady to the max, buuut...

(status == "OK" || status == "FAILED") && event == "(conversation" {
	printf("done\t(%s)\t%s\t%s\n", status, sys, time)
	interval = 0
	# get time spent into "interval"
	# Time format is: "(mon/day-hr:min-pid)"
	n = split(time, nn, "-")
	n = split(nn[2], hrmin, ":")
	tend = (hrmin[1]*60) + hrmin[2]
	n = split(startime[sys], nn, "-")
	n = split(nn[2], hrmin, ":")
	tbeg = (hrmin[1]*60) + hrmin[2]

	interval = tend - tbeg
	if (interval < 0)
		interval += (24*60)

	if (state[sys] == uscall)
		ourtime[sys] += interval
	else
		theirtime[sys] += interval
	}

# All that's left to do now is to feed the chickens and go home

END	{
	for (i in ourtime)
		printf("%s -- ourtime = %d\ttheirtime = %d\n", \
			i, ourtime[i], theirtime[i])
	}


