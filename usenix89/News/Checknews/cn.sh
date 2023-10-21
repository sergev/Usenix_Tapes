
#! /bin/sh

# Sun Nov 30 22:34:48 EST 1986	(...!sunybcs!gworek / ...!boulder!forys)
#
#	cn - Check for available news, multicolumn style.
#

	rn -s250 -c | \
		awk '{ \
			printf "[%2.2s] %-17.17s  ", $5, $4; \
			if ((NR % 3) == 0) \
				print ""; \
		} END { \
			if ((NR) && (NR % 3)) \
				print ""; \
		}'
