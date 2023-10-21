:
: "@(#)upost.sh	1.1 (TDI) 7/18/86 21:10:46"
: "@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery."
:
: "@(#)This file is part of UNaXcess version 0.4.5."
:
: "Permission is hereby granted to copy and distribute this program"
: "freely.  Permission is NOT given to modify this program or distribute"
: "it at cost, except for charging a reasonable media/copying fee."
:

UADIR="`sed -n 's/^@UAOWNER@:[^:]*:[^:]*:[^:]*:[^:]*:\([^:]*\):[^:]*$/\1/p' < /etc/passwd`"

conf=$1; shift
msgnum=`cat $UADIR/msgdir/$conf/himsg`
case "$msgnum" in
[0-9]*)	;;
*)	echo "$msgnum" >&2
	exit 1
esac
msgnum=`expr $msgnum + 1`
if echo "Date: `@udate@`
From: Message Posting Daemon <${LOGNAME-daemon>
To: All
Subject: $*
" > $UADIR/msgdir/$conf/$msgnum 2>/dev/null
then	cat >> $UADIR/msgdir/$conf/$msgnum
	echo $msgnum > $UADIR/msgdir/$conf/himsg
else	echo "$0: could not make new message $msgnum in $conf"
	exit 1
fi
