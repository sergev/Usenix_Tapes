#include <stdio.h>
#include "sqcom.h"
#include "sq.h"
#define ERROR -1
#define TRUE 1
#define FALSE 0

/* First translation - encoding of repeated characters
 * The code is byte for byte pass through except that
 * DLE is encoded as DLE, zero and repeated byte values
 * are encoded as value, DLE, count for count >= 3.
 */

/* ver3.3 modified to eliminate all assignments within return(..). */

init_ncr()	/*initialize getcnr() */
{
	state = NOHIST;
}

int
getcnr(iob)
FILE *iob;
{
	switch(state) {
	case NOHIST:
		/* No relevant history */
		state = SENTCHAR;
		lastchar = getc_crc(iob);
		return (lastchar);   
	case SENTCHAR:
		/* Lastchar is set, need lookahead */
		switch(lastchar) {
		case DLE:
			state = NOHIST;
			return (0);	/* indicates DLE was the data */
		case EOF:
			return (EOF);
		default:
			for(likect = 1; (newchar = getc_crc(iob)) == lastchar && likect < 255; ++likect)
				;
			switch(likect) {
			case 1:
				lastchar = newchar;
				return (lastchar);
			case 2:
				/* just pass through */
				state = SENDNEWC;
				return (lastchar);
			default:
				state = SENDCNT;
				return (DLE);
			}
		}
	case SENDNEWC:
		/* Previous sequence complete, newchar set */
		state = SENTCHAR;
		lastchar = newchar;
		return (lastchar);
	case SENDCNT:
		/* Sent DLE for repeat sequence, send count */
		state = SENDNEWC;
		return (likect);
	default:
		puts("Bug - bad state\n");
		exit(1);
	}
	return (0);	/*Won't get here but this'll make some compiler happy*/
}
