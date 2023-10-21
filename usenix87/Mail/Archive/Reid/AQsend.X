#! /bin/sh
#
# This program takes "send" entries from the work queue and sends them.
#
# usage: AQsend 
#	 AQsend byte-limit, e.g. AQsend 100000

PATH=#PATH#
export PATH
HomeDir=#HOME#
WorkQueue=#REQQ#
Log=#REQQ#/out.log

SizeLimit=${1-999999999}
cd $WorkQueue
echo "`date`" AQsend $SizeLimit >> $Log
CumSize=0
Files=""
for j in 0*
do
    if [ "$j" = '0*' ] ; then exit; fi
    size=`expr "$j" : '\(.*\)\.'`
    CumSize=`expr $CumSize + $size`
    Files="$Files $j"
    if [ $CumSize -gt $SizeLimit ] ; then break; fi
done

echo Ready to send $Files
echo Cumulative size is $CumSize

cd $HomeDir
for j in $Files
do
    sh $WorkQueue/$j && rm -f $WorkQueue/$j
    sleep #SLEEPBETWEEN#
done
