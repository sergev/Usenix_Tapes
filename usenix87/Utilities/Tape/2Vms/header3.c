#include "vmstape.h"

/* these functions really simulate r_record, but with these names
 * make reading the code easier
 */

/* is there a Header 3 area on the tape?
 */

header3()
{
	char	buf[RECSIZE];

	if( read(magtape, buf, RECSIZE) != 0 )
		return(1);
	return(0);
}

trailer3()
{
	char	buf[RECSIZE];

	if( read(magtape, buf, RECSIZE) != 0 )
		return(1);
	return(0);
}
