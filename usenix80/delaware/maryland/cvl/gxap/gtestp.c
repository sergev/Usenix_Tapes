#
#include        "./gparam"

/*
**      This routine will test for invalid parameters
**      sent to the row and column I/O functions.  If
**      the call is from the 'row' functions, then...
**
**              rcnum = relative row number
**              rcstart = relative column number
**              rc1 = area->rown
**              rc2 = area->coln
**
**      Similiarly for the column functions.
*/

int gtestp(rcnum, rcstart, npts, inc, rc1, rc2)
{
	register rstart, rnpts, temp;

	if(rcnum < 0 || rcnum >= rc1) return(1);

	if((rstart = rcstart) < 0 || rstart >= rc2) return(1);

	if((rnpts = npts) <= 0) return(-1);

	temp = (rnpts - 1) * inc + rstart; /* Scan size for this I/O operation */

	if(temp < 0 || temp >= rc2) return(1);

	return(0);      /* Else, parameters are valid */
}
