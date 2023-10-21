#! /bin/sh
#
# This program exists to be called from crontab so that it can call
# AQsend as a non-privileged user.
#
PATH=#PATH#
export PATH
Log=#REQQ#/out.log

trap "rm -f #TMP#/*.$$; exit" 0 1 2 3 15

# Don't do this if the load on #MAILGATE# is too high. /bin/test can only
# handle integers, so edit out the decimal point and multiply by 100.
# If you're trying to run this someplace besides a BSD system, what all this
# code is trying to do is determine whether or not the mail gateway is up, and
# if it up to determine what its load factor is.

ruptime | grep  '^#MAILGATE#' > #TMP#/AQcrontab.$$
if grep -s ' up ' #TMP#/AQcrontab.$$
then
# mail gateway is up
    LOAD15MIN=`sed -e 's/.* \([^ ]*\)$/\1/' -e 's/\.//g' #TMP#/AQcrontab.$$`
    if [ $LOAD15MIN -gt #MLOADLIM# ]
    then
	echo "`date`" AQcrontab: load factor $LOAD15MIN >> $Log
	exit
    fi
    
    # The load factor is low enough. 
    ID=`whoami`
    if [ "$ID" = "root" ]
    then
	su daemon << glogg
	AQsend #PERHOUR#
glogg
    else
	AQsend #PERHOUR#
    fi
else
# mail gateway is down
    echo "`date`" AQcrontab: #MAILGATE# down >> $Log
    exit
fi
