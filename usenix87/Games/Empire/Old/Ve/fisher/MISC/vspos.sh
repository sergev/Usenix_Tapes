# vspos - shell/awk program to update the ship file with info obtained
#	from buzzing harbors/sea from planes
#	Save the output from a flyover and strip everything but the
#	type shipno
#	list.  Note the coordinates, and guess at the country number

#	To update the ship file:
#	vspos coord cno < shiplist | eupd

if [ $# -eq 0 ];then
	echo "Usage: $0 coord cno < shiplist(type,shipno)"
	exit 1
fi
echo "[0:0] Command :"
awk '{
	type = $1;
	numb = $2;
	if( $1 == "pt" ) {
		type = $1 " " $2;
		numb = $3;
	}
	printf "Country (#%d) %s %s @%s\n", CNO, type, numb, COORD;
}'  COORD=$1  CNO=$2 -
