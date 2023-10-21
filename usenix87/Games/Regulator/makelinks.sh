#  Written by Warren Lavallee.  For use with games regulation
# program.  This makes the links that the program needs to operate
# with.

# The location of the Game Regulator to use
GR=/usr/local/games/.gr

CWD="`pwd`"
echo $CWD

if test ! -d .hiden; then
	mkdir .hiden
	echo "$0: made .hiden direcotry to hide games in"
	chmod 770 .hiden
fi

if test ! -f $GR; then
	echo "Game Regulator not there"
	exit
fi

echo doing links now:

for i do
	echo -n $i " "
	if test ! -f .hiden/$i; then
		mv $i .hiden
	fi
	ln $GR $CWD/$i
done
echo
exit 0
