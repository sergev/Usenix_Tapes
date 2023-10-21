#! /bin/sh
#
# This program exists to be called from crontab so that it can call
# AQsend as a non-privileged user.
#
PATH=#PATH#
export PATH
Log=#REQQ#/out.log

trap "rm -f /tmp/*.$$; exit" 0 1 2 3 15

scan-work-queue 250 500
