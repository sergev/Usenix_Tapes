#! SHELL
# 'Usage: lbgmX Message-ID [ARCHIVE]'
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

if [ ! -w $LOGFILE ]
then
	echo "$PROG: You can not write the $LOGFILE"
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
filenam=`awk  'BEGIN { filid = "" ; FS = ":" }
	{ if ( filid == "" ) filid = $1 }
	{ if ( $0 ~ /^[	 ]*$/ ) filid = "" }
/^	[Mm][eE][sS][sS][aA][gG][eE]-[Ii][Dd]:/	{ if ( $2 == "'"$msgid"'" ) { print filid ; exit} } ' < $LOGFILE `

if [ x$filenam = x ]
then
	echo "$msgid not in $LOGFILE"
	exit 1
fi

: 'finish off'
if [ ! -r $filenam ]
then
	echo "$PROG: Can't read $filenam"
	exit 1
fi

: 'delete the header from the old log file'
sed -e /$filenam"/,/^	$/d" < $LOGFILE > New.Log

: '*********************************************************
  save the logfile with the name of the file deleted
  back up the file, and then save the New Log file
  **********************************************************'
if [ -s New.Log ]
then
	mv $LOGFILE \#log.$filenam
	mv $filenam \#$filenam
	mv New.Log $LOGFILE
else
	echo "Sed Failed, No New Logfile"
fi
