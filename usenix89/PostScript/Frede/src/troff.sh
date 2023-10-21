# troff shell script - invoke ditroff with appropriate args
#	- Stephen Frede
# The default device
dev=-Talw
# default postprocessor
post=tpscript
# ditroff arguments
ditargs=
# postprocessor arguments
postargs=
for i
do
	case $i in
	-Talw)	dev=-Talw
		post=tpscript
		;;
	-Toldalw)
		dev=-Toldalw
		post=oldtpscript
		;;
	-Ttek)	dev=-Talw	# use alw font tables
		post=tc
		;;
	-Tuqalw)
		dev=-Talw
		post=tpscript
		echo "-Tuqalw has been re-named -Talw (new default) (warning)" 1>&2
		;;
	-Thpgl)	dev=-Talw	# use alw font tables for now
		post=hpgl
		echo "hpgl back end not finished yet" 1>&2
		exit 1
		;;
	-T*)	echo invalid device $i;  exit 1 ;;
	-c*)	# -cxxx doesn't work; translate to -mxxx
		ditargs="$ditargs "-m`expr "$i" : "-.\(.*\)"` ;;
	-a)	# ascii representation - no postprocessor
		ditargs="$ditargs -a"
		post=""
		;;
	-R*)	# rotate
		if [ "$dev" != "-Talw" ]
		then
			echo "-R option only applies to device 'alw'" 1>&2
			exit 1
		fi
		postargs="$postargs "-r`expr "$i" : "-.\(.*\)"`
		;;
	-[sS])	# single page at a time - manual feed on ALW
		if [ "$dev" = "-Talw" ]
		then
			postargs="$postargs -S"
		else
			ditargs="$ditargs $i"
		fi
		;;
	*)	ditargs="$ditargs "$i	;;
	esac
done

if [ "$post" = "" ]
then
	/bin/ditroff $dev $ditargs
else
	/bin/ditroff $dev $ditargs | /usr/lib/troff/$post $postargs
fi
