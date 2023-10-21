:
: "@(#)umotd.sh	1.1 (TDI) 7/18/86 21:10:44"
: "@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery."
:
: "@(#)This file is part of UNaXcess version 0.4.5."
:
: "Permission is hereby granted to copy and distribute this program"
: "freely.  Permission is NOT given to modify this program or distribute"
: "it at cost, except for charging a reasonable media/copying fee."
:

UADIR="`sed -n 's/^@UAOWNER@:[^:]*:[^:]*:[^:]*:[^:]*:\([^:]*\):[^:]*$/\1/p' < /etc/passwd`"

msgnum=`cat $UADIR/motd/himotd`
case "$msgnum" in
[0-9]*)	;;
*)	echo "$msgnum" >&2
	exit 1
esac
msgnum=`expr $msgnum + 1`
if echo "UNaXcess V0.04.04 -- `@udate@`
" > $UADIR/motd/$msgnum 2>/dev/null
then	${EDITOR-vi} $UADIR/motd/$msgnum
	echo $msgnum > $UADIR/motd/himotd
else	echo "$0: could not make new bulletin $msgnum"
	exit 1
fi
