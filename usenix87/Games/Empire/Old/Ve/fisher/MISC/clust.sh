# clust - shell/awk program to show where ships are clustered
#	input:	output of rdship command
#	output:	sorted list of coordinates and number of ships at that location
#		1	4,3
#		4	-5,-16
#		23	10,-2
#		..	..
#	usage: rdship [-c cno] [| optional_filter] | clust

awk '
$2 ~ /pt/	{
		if( x[$4] == 0 ) x[n++] = $4;
		x[$4]++;
		next;
		}
	{
		if( x[$3] == 0 ) x[n++] = $3;
		x[$3]++;
	}
END	{
		for( i=0; i<n; i++ ) {
			printf "%3d\t%s\n", x[x[i]], x[i];
		}
	}
' | sort -n
