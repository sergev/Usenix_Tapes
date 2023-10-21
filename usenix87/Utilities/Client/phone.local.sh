#! /bin/sh
PHONELIST="/usr/local/lib/phonelist"
case $1 in
"")
	grep "." $PHONELIST
	;;
esac
grep -i $1 $PHONELIST
grep -i $1 /usr/lib/aliases | grep @ | grep -v ","
