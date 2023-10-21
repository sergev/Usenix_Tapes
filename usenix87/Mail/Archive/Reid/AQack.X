#! /bin/sh
#
# This program is called from AQenqueue to let the customer know the status
# of his request, and possibly also to send the request if the size of the
# request is small enough.
#
# usage: AQack address name date type-of-msg bytes goodfiles badfiles

PATH=#PATH#
export PATH
TmpFile=#TMP#/AQack.$$
LOWLIMIT=#LOWLIMIT#
export LOWLIMIT

Path="$1"; shift;
From="$1"; shift;
Date="$1"; shift;
ReplyCode=$1; shift;
ByteCount=$1; shift;
Badfiles="$1"; shift;
Goodfiles="$1"; shift;

echo "$Date" $ReplyCode $Path $ByteCount >> #REQQ#/out.log

case $ReplyCode in
    null) cat << a1 > $TmpFile
The server could not find in your message the name of anything to send.
If you need instructions, send the server a message saying "help"
a1
 Retval=1 ;;
    allgood) cat << a2 > $TmpFile
All of the files that you asked for are in the database.
a2
 Retval=0 ;;
    allbad) cat << a3 > $TmpFile
The server was not able to find any in its database any of the files that you
requested. You asked for these files:

`echo $Badfiles | fmt`

Perhaps you are looking in the wrong directory? Perhaps you are using the
archive service incorrectly? If you would like instructions, send the server
a message saying "help". If you are completely baffled, send a message to
archive-manager (rather than to archive-server) and an actual human will
answer (though perhaps not instantly).
a3
 Retval=1 ;;
    somegood) cat << a4 > $TmpFile
The server was able to find some of the files that you requested and was
unable to find others. These were not found:

`echo $Badfiles | fmt`
a4
 Retval=0 ;;
    multishar) cat << a5 > $TmpFile
You have requested more than one "shar" file. These files are bundles
containing numerous programs within themselves, and it is risky to bundle
more than one of them together. Therefore the server will send you the first
shar file that you asked for, $Goodfiles, but will not
send you
$Badfiles
to get those files, you must send in a separate retrieval request.
a5
 Retval=0;;
    toobiguucp) cat << a6 > $TmpFile
The collection of files that you have requested is too big to mail to you.
Our uucp mailer is limited to sending no more than #UUCPLIMIT# Kbytes per message.
The server will send you as much as possible, but the files listed below
would not fit and so will not be sent. You must submit another retrieval
request if you want them:

`echo $Badfiles | fmt`

a6
 Retval=0;;
esac
case $ByteCount in
 0) : ;;
 *) if [ $ByteCount -lt $LOWLIMIT ]
    then
	cat << smallrequest >> $TmpFile
Since your request is small, it is being processed immediately instead of
being placed in the work queue. Here it is:
`cat #HOME#/copyright`

smallrequest
	set $Goodfiles
	case $# in
	    1) cat $Goodfiles >> $TmpFile;;
	    *) rshar $Goodfiles >> $TmpFile;;
	esac
	Retval=2
    else
	cat << bigrequest >> $TmpFile
Your request is $ByteCount bytes. It has been placed in the mailer's work
queue. The work queue is periodically processed by the archive mailer, which
always sends the shortest queued request first. The length of time that it
takes to receive your request will depend on how many smaller requests are in
front of it in line.
bigrequest
    fi;;
esac
cat - $TmpFile << funfilled | #MAILER# "$Path"
From: #SVC-TITLE# <#SVC-MAILBOX#>
To: $Path
Subject: ack receipt of your archive retrieval request
In-reply-to: Request from $From dated $Date

The #SVC-NAME# has received your request.
funfilled

rm -f $TmpFile
exit $Retval
