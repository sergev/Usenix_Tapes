#	sink - shell program to remove ships from ship file
#		usage: sink [shipno ...]
(
echo "[0:0] Command :"
for i
do
SHIP=`expr "    $i" : ".*\(....\)"`
echo "$SHIP battleship   0,0       0%   0   0   0   0   0   0   0"
done
) | eupd
