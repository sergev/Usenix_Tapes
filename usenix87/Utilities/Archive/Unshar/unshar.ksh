
function unshar
{
	if [ -z "$1" ]
	then
		sed -e '1,/^[:#]/d' | sh
	else
		for file in $*
		do
			echo "X - $file"
			sed -e '1,/^[:#]/d' $file | sh
		done
	fi
}

