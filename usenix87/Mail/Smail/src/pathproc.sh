#! /bin/sh
#
#	@(#)pathproc.sh	2.1 (smail) 12/14/86
#
# This script will do all that's necessary for
# transforming the output of pathalias -c into
# the format of a 'paths' file for smail.
#
# format of the pathalias -c output is
# cost	host	route
#
# format of a 'paths' file for smail is
# host	route	first_hop_cost
#
# first sort lines on increasing cost
#
sort -n |
#
# print cost of first hop in the chain in format
# host	route	cost
#
awk '
{
	nhops = split($3, hops, "!");
	if(nhops == 1) {
		tcost = 0;
	} else if(nhops == 2) {
		if(cost[hops[1]] == 0) {
			cost[hops[1]] = $1;
		}
		tcost = cost[hops[1]];
	} else {
		tcost = cost[hops[1]];
	}
	printf("%s\t%s\t%d\n", $2, $3, tcost);
}' |
#
# next convert host name to lower case and sort the output
#
lcasep | sort
