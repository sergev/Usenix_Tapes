# nomine - shell program to clear a mine
#	Use this when you're sure all the mines have been cleared

#	usage: nomine x,y

(
echo "[0:0] Command :"
echo "  sect  cmsgpob des eff min gold mob  civ mil   sh  gun  pl  ore bar prod"
SECT=`expr "   $1   " : ".*\(...,...\).*"`
echo "$SECT .......  .   0%   0    0   0    0   0    0    0   0    0   0   0"
echo "    1 sectors"
) | eupd
