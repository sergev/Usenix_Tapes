trap '' 1 2 3
dead_timeout=20
case $0
in
	0)	r1="$0>";r2="$0<";;
	1|2)	r1="$0<";r2="$0>";;
esac
fifo=/tmp/fifo$$
mknod $fifo p 
echo $fifo
exec >&-
(
	trap 'reader_end=1' 15
	reader_end=
	sleep $dead_timeout&
	wait
	trap '' 15
	kill $!
	if [ -z "$reader_end" ]
	then
		eval : $r1 $fifo
	fi
	rm -f $fifo
) 2> /dev/null &
cleanup=$!
(
	(eval "$@" $r2 $fifo; exit 0) || eval : $r2 $fifo
	kill $cleanup 2> /dev/null
) &
