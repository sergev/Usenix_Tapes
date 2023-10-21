#include "dca2troff.h"


do_flush(count)
int count;
{
    char c;
    if ( count == 0 )
	return;
    while ( count > 0 )
	{
	c = get1ch();
	--count;
	}
}

/* flush a structured field */
flush_sf()
{
	do_flush(sf_length - sf_incnt);
}

/* get a char from stdin, increment the sf count */
get1ch()
{
	++sf_incnt;
	return (getchar());
}

/* get a 2 byte number from the input */
get1num()
{
	int c, num;
	c = get1ch();		/* get 1st byte of length */
	if ( c == EOF )
		exit(0);	/* all done */
	c &= 0377;
	num = c << 8;
	c = get1ch();
	c &= 0377;
	num = num + c;
	return(num);
}
