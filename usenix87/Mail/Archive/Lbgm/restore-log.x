#! SHELL
# restore-log
#	If you munge up your logfile, run this to restore it.
#
archive=ARCHDIR
logfile=SAVELOG

if [ $# != 1 ]
then
    echo "Please indicate month to restore" 1>&2
    exit
fi

cd ${archive}/$1
if [ -e $logfile ]
then
    echo "${logfile} exists!" 1>&2
    exit
fi

set `echo "$1" | sed -e 's/_/ /'`
prefix=$1

for i in `/bin/ls ${prefix}* | sed -e 's/_/ /' -e 's/\./ /' |\
	sort -n +1 | sed -e "s/${prefix} /${prefix}_/" -e 's/ /./'`
do
    file=$i
    echo "${file}"

: 'Dump the news header in a log file (up to a blank line)'
    header="$file:
    `sed -n '1,/^$/s/^/	/p' ${file}`"
    echo "${header}" >>$logfile
done
