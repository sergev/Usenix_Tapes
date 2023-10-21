#! SHELL
# fixpath: Shorten the path in the archive LOGFILE.
#
if [ $# -lt 1 ]
then
	echo "`basename $0`: dir [ dir ... ]" 1>&2
	exit
fi

cd ARCHDIR

dirs=$*
set ${dirs}
thisdir=$1

while [ $# -ge 0 ]
do
    cd ${thisdir}
    echo "Fixup ${thisdir}"
    shift
    dirs=$*

    while read line
    do
	if [ `expr length "${line}"` -gt 1 ]
	then
	    set ${line}
	    case "$1" in
	    "Path:")
		echo -n "	"
		echo ${line} | newpath
		;;
	    Jan*|Feb*|Mar*|Apr*|May*|Jun*|Jul*|Aug*|Sep*|Oct*|Nov*|Dec*)
		echo "${line}"
		;;
	    *)
		echo "	${line}"
		;;
	    esac
	else
	    echo "	${line}"
	    echo -n "." 1>&2
	fi
    done < SAVELOG > New.SAVELOG
    echo ""
    /bin/mv New.SAVELOG SAVELOG

    cd ..

    if [ `expr length "${dirs}"` = 0 ]
    then
	exit
    fi
    set ${dirs}
    thisdir=$1
done
