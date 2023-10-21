# aliases.sh --- sample shell functions which do some of what the csh does

# pushd, popd, and dirs --- written by Chris Bertin
# Pixel Computer Inc. ...!wjh12!pixel!pixutl!chris
# as modified by Patrick Elam of GTRI

pushd () {
	SAVE=`pwd`
	DSTACK="$SAVE $DSTACK"
	if [ "$1" = "" ] 
	then
		if [ "$DSTACK" = "$SAVE " ]
		then
			echo "pushd: directory stack empty."
			DSTACK=""
			return 1
		fi
		set $DSTACK
		cd $2
		shift 2
		DSTACK="$SAVE $*"
	else
		if (cd $1)
		then
			cd $1 >&-
		else
			popd > /dev/null
			return 1
		fi
	fi
	dirs
	return 0
}

popd () {
	if [ "$DSTACK" = "" ] 
	then
		echo "popd: Directory statck empty"
		return 1
	fi
	set $DSTACK
	cd $1
	shift
	DSTACK=$*
	dirs
	return 0
}

dirs () {
	echo "`pwd` $DSTACK"
	return 0
}

xchng () {	# exchanged top two entries on the stack
	if [ "$DSTACK" = "" ]
	then
		echo exchange directory stack empty
		return 1
	else
		pushd
		return 0
	fi
}

source () {	# have the shell read a file in the current shell
	. $*
}

bye () { logout ; }

logout () { exit 0 ; }
