# mine - shell program to drop a mine (shell) in sea sector
#	Use this whenever one (or more!) of your ships
#	"detects" one of the little cuties

#	usage:	mine x,y

(
echo "[0:0] Command :"
echo "  sect  cmsgpob des eff min gold mob  civ mil   sh  gun  pl  ore bar prod"
SECT=`expr "   $1   " : ".*\(...,...\).*"`
echo "$SECT .......  .   0%   0    0   0    0   0    1    0   0    0   0   0"
echo "    1 sectors"
) | eupd
