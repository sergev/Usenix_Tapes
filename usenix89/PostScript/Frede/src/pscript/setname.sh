# Set a new printer name
if [ $# != 2 ]
then
	echo "Usage: setname passwd-number name" 1>&2
	exit 1
fi
cat <<!
%!
$1 serverdict begin exitserver
statusdict begin
	($2) setprintername
end
!
echo "\004"
