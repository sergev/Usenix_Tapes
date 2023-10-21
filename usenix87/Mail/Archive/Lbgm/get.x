#! /bin/csh -f
# get: extract a requested file.  Given in 'directory archivename destname'
# where:
#	directory is the directory which the file lives in
#	archivename is the name which the archive program knows the file as
#	destname is the location where the user wants the program placed
#
setenv PATH "/usr/local:/usr/bin:/bin/:/usr/ucb:"

cd ARCHDIR

set RUNNER = `whoami`
set name = $0

if ( $#argv < 3 ) then
	echo "Usage: $0 dir archname destname [ archname destname ... ]"
	exit 1
endif

if ( $1 == "mod" ) then
	set modrequest = 1
	shift
else
	set modrequest = 0
endif

if ( ! -d $1 ) then
	echo "'$1' is not a directory"
	exit 1
endif

cd $1
shift

while ( $#argv > 0 )
	if ( `basename $2` == $2 ) then
		echo "$name : '$2', can't use names without directory"
		goto skip-out
	endif

	# if standard file, copy it

	if ( -f $1 ) then
		cp $1 $2
		goto send-mail
	endif

	# if compressed file, zcat to new file

	if ( -f $1.Z ) then
		zcat $1 > $2
	else
		echo "$1 does not exist, skipping"
		goto skip-out
	endif

# change permission, and get real subject name

send-mail:
	chmod 666 $2
	set subject = `grep "^Subject:" $2`

	switch ($RUNNER)
	case "MYNAME":
	case "news":
		breaksw
	default:
		echo "$subject" > /tmp/get$$
		echo "$1 is archive name" >> /tmp/get$$
		echo "$RUNNER aquired the file on `date`" >> /tmp/get$$
		Mail -s "$RUNNER" MYNAME < /tmp/get$$
	endsw
	/bin/rm -f /tmp/get$$
	if ( ${modrequest} == 0 ) then
	    echo "$1 is installed at $2"
	else
	    echo "$2"
	endif

skip-out:
	shift
	shift
end
