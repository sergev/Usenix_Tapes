/*
 * Copyright 1985, Todd Brunhoff.
 *
 * This software was written at Tektronix Computer Research Laboratories
 * as partial fulfillment of a Master's degree at the University of Denver.
 * This is not Tektronix proprietary software and should not be
 * confused with any software product sold by Tektronix.  No warranty is
 * expressed or implied on the reliability of this software; the author,
 * the University of Denver, and Tektronix, inc. accept no liability for
 * any damage done directly or indirectly by this software.  This software
 * may be copied, modified or used in any way, without fee, provided this
 * notice remains an unaltered part of the software.
 *
 * $Log:	debug.c,v $
 * Revision 2.0  85/12/07  18:21:07  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: debug.c,v 2.0 85/12/07 18:21:07 toddb Rel $";
#include	"server.h"

main(argc, argv)
	char	**argv;
	int	argc;
{
	int	dbg;

	dbg = atox(argv[1]);
	printf("set debug to %x\n", dbg);
	printf("remotename()=%d\n", remotename(NM_DEBUG, dbg));
	perror("debug");
}

/*
 * ascii to hex
 */
atox(buf)
	char    *buf;
{
	register char   *p;
	register unsigned       num, nibble;

	/*
	 * now, take it out checking to make sure that the number is
	 * valid.
	 */
	if (! buf)
		return(0);
	for(num=0, p = buf; *p; p++)
	{
		nibble = *p;
		if (nibble >= 'A' && nibble <= 'F')
			nibble -= 'A' - 10;
		else if (nibble >= 'a' && nibble <= 'f')
			nibble -= 'a' - 10;
		else if (nibble >= '0' && nibble <= '9')
			nibble -= '0';
		else
			return(0);
		num = (num << 4) | nibble;
	}
	return(num);
}
