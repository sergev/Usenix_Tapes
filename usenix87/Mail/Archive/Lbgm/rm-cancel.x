#! SHELL
# rm-cancel: delete 'cancel' and 'newgrp' messages from the
#	directory. (Which can't be 'Current')
#
trap "cd ${base} ; /bin/rm -f $newlog ; exit 1" 0 1 2 3 4 5 6 7 8 9

archivedir=/usr/lib/news/archive
base=ARCHDIR
newlog=/tmp/newlog

if [ $# -lt 1 ]
then
	echo "$0 Archive_Month [ archive_month ... ]" 1>&2
	exit 1
fi

cat > $newlog <<'eof-NEWLOG'
#! SHELL
: 'newlog filename'
: 'default archive location'
PROG=`basename $0`
if [ ! -d $1 ]
then
    echo "$PROG: '$1' is not a directory" 1>&2
    exit 1
fi
dir=$1
cd $dir
shift
LOGFILE=SAVELOG
NEWLOG=New.Log

if [ ! -f $LOGFILE ]
then
    echo "$PROG: '$LOGFILE' nonexistant" 1>&2
    exit 1
fi

while [ $# -gt 0 ]
do
    filenam=$1
    shift
    if [ -z "$filenam" ]
    then
	echo "No filename specified" 2>&1
	exit 1
    fi

    if [ ! -r "$filenam" ]
    then
	if [ ! -r "$filenam".Z ]
	then
	    echo "$filenam".Z not found
	    echo pwd = `pwd`
	    echo "$PROG: Can't read $filenam" 2>&1
	    exit 1
	else
	    xfilenam=${filenam}.Z
	fi
    else
	xfilenam=${filenam}
    fi

    : 'delete the header from the old log file'
    sed -e /$filenam"/,/^	$/d" < $LOGFILE > $NEWLOG

    : '*********************************************************
      save the logfile with the name of the file deleted
      back up the file, and then save the New Log file
      **********************************************************'
    if [ -s $NEWLOG ]
    then
	mv $LOGFILE \#log.$filenam
	mv $xfilenam \#$xfilenam
	mv $NEWLOG $LOGFILE
    else
	echo "Sed Failed, No New Logfile"
    fi
done
eof-NEWLOG

chmod +x $newlog

while [ $# -ge 1 -a -d ${base}/$1 ]
do
    dir=$1
    shift
    if [ ${dir} = "Current" ]
    then
	echo "`basename $0`: 'Current' invalid directory"
	continue
    elif [ `/bin/ls -l ${base}/Current | sed -e 's/\(.*\) -> //'` = ${dir} ]
    then
	echo "`basename $0`: '${dir}' invalid directory (Really Current)"
	continue
    fi

    echo ${dir}
    ${archivedir}/toc ${dir} > /tmp/toc.$$

    for i in `grep -i newgroup < /tmp/toc.$$ | awk -F: '{print $1}'`
    do
	    echo -n "Killing $i ..."
	    $newlog ${base}/${dir} $i
	    echo " done"
    done

    for i in `grep -i cancel < /tmp/toc.$$ | awk -F: '{print $1}'`
    do
	    echo -n "Killing $i ..."
	    $newlog ${base}/${dir} $i
	    echo " done"
    done
    /bin/rm -f /tmp/toc.$$
done
