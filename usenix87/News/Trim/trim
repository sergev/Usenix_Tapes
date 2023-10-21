
trap 'echo Aborting ; rm -f /tmp/chl$$ ; exit ' 1 2 3 4 10 11 15

# SPOOL is the name of the news spool directory where news files are found
# ... it should contain the root name of just the spool dir and nothing
# ... else; otherwise, set spool as SPOOL="dir1 dir2 dir3" for multidir
# ... news spoolers.
#
# NORMALBYTES is the size in bytes that the spool directory should
# ... normally be -- this does not include the size of directories
SPOOL=/usr/spool/news
NORMALBYTES=2000000

# just a note:
#
# expect output from 'ls -r' to be ...
#
# drwxr-xr-x 18 joe      4096 Jan 12 15:26 ./
#
#	day is $6
#	month is $5
#	size is $4

tday=`date | cut -d' ' -f3`
tmon=`date | cut -d' ' -f2`

#
# sed pipeline is used below to add an extra space between permissions
# and number of links, BSD-ls can run the two together, eg we want
#	drwxr-xr-x107 joe      4096 Jan  6 18:18 src/
# converted to:
#	drwxr-xr-x 107 joe     4096 Jan  6 18:18 src/
# sed pipeline also converts tabs to spaces, be sure you have
# a REAL tab on second sed command.
#

#
# grep pipeline is used to remove directory entries from output.
#
ls -lR $SPOOL | sed 's/^\(..........\)/\1 /
s/	/ /g' | grep -v '^d' | awk '
BEGIN {
	Months="JanFebMarAprMayJunJulAugSepOctNovDec"
	NoDays="0  31 28 31 30 31 30 31 31 30 31 30 31"
	CurMonth = 0
	for( m = 0; m <= 11; ++m ) {
		if( substr(Months, (m*3) + 1, 3) == "'$tmon'") {
			CurMonth = m + 1
			break
		}
	}
	if( CurMonth > 0 ) {
		Julian = 0
		MonJulian = 0
		for( m = 0; m < CurMonth; ++m ) 
			MonJulian += substr(NoDays, (m*3)+1, 3)
		Julian = MonJulian + '$tday'
	}
	#
	# first output record is the Julian date of today
	#
	printf("9999999999999999999 TODAY %d\n", Julian)
}
NF >= 3 {
	
	# hash date into a julian like number

	CurMonth = 0
	for( m = 0; m <= 11; ++m ) {
		if( substr(Months, (m*3) + 1, 3) == $5) {
			CurMonth = m + 1
			break
		}
	}
	if( CurMonth > 0 ) {
		Julian = 0
		MonJulian = 0
		for( m = 0; m < CurMonth; ++m ) 
			MonJulian += substr(NoDays, (m*3)+1, 3)
		Julian = MonJulian + $6
	}
	date[Julian] += $4
}
END {
	for( day in date ) {
		printf("%d %ld\n", day, date[day])
	}
}' > /tmp/chl$$

#
# output to tmp file so we don't create a zillion pipes
#

sort -rn /tmp/chl$$ | awk '
BEGIN {
	sum = 0
	base = 0
}
NR == 1 {
	today = $3
	printf("\n# today is %d\n", $3)
}
NR > 1 {
	sum += $2
	if( sum >= '$NORMALBYTES' ) {
		if( base == 0 ) {
			printf("# passing '$NORMALBYTES' byte threshold, below should get expired\n")
			base = $1
		}
	}
	printf("# got %ld bytes on %d, sofarat %ld", $2, $1, sum)
	if( today == $1 )
		printf(", today")
	else if( (today - 1) == $1 )
		printf(", yesterday")
	printf("\n")
}
END {
	printf("# recommend that you expire over %d days\n", (today - base))
}'

rm -f /tmp/chl$$

exit
