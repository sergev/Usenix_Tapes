# Select the fonts to be idle time scanned
if [ $# != 1 ]
then
	echo "Usage: setidle passwd-number" 1>&2
	exit 1
fi
cat <<!
%!
$1 serverdict begin exitserver
statusdict begin
	mark
	4 100 100 0 94	% Times-Roman 10 point
	5 100 100 0 94	% Times-Bold 10 point
	6 100 100 0 94	% Times-Italic 10 point
	setidlefonts
end
(idle time font scan changed\n) print flush
!
echo "\004"
