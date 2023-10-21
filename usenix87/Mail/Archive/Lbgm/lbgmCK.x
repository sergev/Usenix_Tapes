#! SHELL
# 'Usage: lbgmCK Message-ID [ARCHIVE]'
# 'default archive, see ARCHIVE/newlog as well'
ARCHIVE=ARCHCUR
PROG=$0

case $# in
	1)	msgid=$1
		lbgmdir=$ARCHIVE
		;;
	2)	msgid=$1
		lbgmdir=$2
		;;
	*)	echo "usage: $PROG Message-ID [ARCHIVE]" 2>&1
		exit 1
		;;
esac

: 'lbgmdir is archive location'
if [ ! -d $lbgmdir ]
then
	echo "$PROG: $lbgmdir is not a directory" 2>&1
	exit 1
fi

cd $lbgmdir
LOGFILE=$lbgmdir/SAVELOG

if [ ! -r $LOGFILE ]
then
	echo "$PROG: You can not read the $LOGFILE"
	exit 1
fi

: '/***********************************************************************
	Note: The last line in the file will always set the fileid to ""

	GET FILENAME FROM MESSAGE-ID: "filenam" is the file to purge
	BEGIN: set the file sep to ":", and the fileid to "nothing" at the start
	1. if the file id is empty, then the first record contains a
		file name in field 1
	2. if the entire record is just blanks and tabs, empty the
		fileid, as this indicates the end of a header
	3. if we have found the MESSAGE-ID field in a record, and
		its second field matches the string we are looking for
		then print the filename
: ***********************************************************************/ '
grep -s $msgid $LOGFILE
case $? in
	0)	# found, exit normally
		exit 0
		;;
	1)	# not found, error
		echo "$msgid, not in $LOGFILE"
		exit 1
		;;
	2)	# grep error
		echo "grep had problems"
		exit 2
		;;
esac
