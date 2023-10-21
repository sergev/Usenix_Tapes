#! /bin/sh
for i in $*
do
	case $i in
	-h)	hold=;
		continue;;
	-h*)	hold="$hold `echo x$i|sed -e s/...//`"
		continue;;
	-)	size=;
		continue;;
	-*)	size=$i;
		continue;;
	//*)	continue;;
	esac
	/usr/lib/news/newsfeed $size $i $hold
done
