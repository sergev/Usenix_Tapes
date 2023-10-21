#!/bin/sh
#
# install.sh
#
#	This shell script is meant to install new versions of
#  Game Regulator into the proper directory.  
# 
#			ll-xn!warren
#

if test $# != 3; then
	echo "Usage: $0 (Directory) (path of new GR) (path of old GR)"
	exit
fi

DIR=$1
GR=$2
GRN=$3

if test ! -d $DIR; then
	echo "$DIR: non-existant"
	exit
fi
if test ! -d $DIR/.hiden; then
	mkdir $DIR/.hiden
	echo "$0: making $DIR/.hiden to hide the games in"
fi
if test ! -f $GR; then
	echo "$0: $GR: not there"
	exit
fi

echo "Installing new binaries"
cp $GR $GRN
chgrp kmem $GRN
chmod 6775 $GRN
cd $DIR/.hiden
FILES="`/bin/ls`"
cd $DIR
echo $DIR
echo $FILES
rm -f $FILES


echo doing links now:

for i in $FILES; do
	echo -n $i " "
	if test ! -f .hiden/$i; then
		mv $i .hiden
	fi
	ln $GRN $DIR/$i
done
echo
exit 0
