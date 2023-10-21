# Set a new passwd
if [ $# != 2 ]
then
	echo "Usage: setpasswd oldpasswd-number newpasswd-number" 1>&2
	exit 1
fi
cat <<!
%!
$1 serverdict begin exitserver
statusdict begin
	$1 $2 setpassword ==
end
!
echo "\004"
