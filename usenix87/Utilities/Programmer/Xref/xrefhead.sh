#! /bin/sh
# quick and dirty cross reference generator

DESTDIR=???usrlocal			# revised by Makefile
PATH=/bin:/usr/bin:/usr/ucb:$DESTDIR	# revise these if necessary

width=80				# defaults
farg="-e '~'"				# a dummy character

files=""
while true
do
    case "$1" in
	"")	break;;
	-k)	keywords=$2; shift
		farg="-f $keywords";;
	-w)	width=$2; shift;;
	-*)	echo 1>&2 "Usage:  xref [-k keyfile] [-w n] [file ...]"
		exit 2;;
	 *)	files="$files $1";;
    esac
    shift
done

xreflex $files |
    fgrep -v $farg |
	sort -u +0f -1 +0 -1 +1 -2 +2n |
	    awk '\
