# syslog.awk -- read a uucp SYSLOG and find out how much
# stuff is transferred and how long it took
#
# This works With System V release I and 4.2BSD
#
# USAGE
#	awk -f syslog.awk /usr/spool/uucp/LOGFILE
#
# AUTHOR
#	Michael Wexler
#	trwrb!felix!peregrine!mike
#	Peregrine Systems, Inc
#
$1 ~ /.*!.*/	{
	n=split($1,a,"!");
	sys=a[1]
}
$1 !~ /.*!.*/	{
	sys=$2
}
{
	bytes[substr(sys,1,6)] += $7; 
	time[substr(sys,1,6)] += $9;
}
END	{
	for (sys in bytes)
	{
	print sys, "	Transferred ",bytes[sys]," bytes in ",time[sys]," seconds"
	print "	For an average speed of ",bytes[sys]/time[sys]," bytes/sec"
	}
}
